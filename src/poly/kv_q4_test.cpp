// src/poly/kv_q4_test.cpp - the Q4_0 KV cache kernels of the poly engine, store_q4 and store_q4_at, against
// ggml's own q4_0 formulas.  No model file and no card beyond CUDA device 0:
//   1. round trip: for several row counts and n in {32,128,256}, random floats of mixed magnitude are stored and
//      read back; every dequantized value must be within half a block scale (plus a hair) of the input;
//   2. byte layout: for n=256 the device quants and fp16 scales must match ggml's quantize_row_q4_0_ref
//      (transcribed below, each line marked with its ggml source line) byte for byte, which pins
//      quants[row*(n/2) + blk*16 + j] (values j and j+16 in the low/high nibbles) and scales[row*(n/32) + blk];
//   3. store_q4_at with a ring: two rows written from a device `at` land at (*at + r) % ring and equal what plain
//      store_q4 writes from the same position;
//   4. attend/attend_batch: caches stored with store_q4 read back through the Q4 value path must match a host
//      attention computed over the fp16 keys with the values from ggml's dequantize_row_q4_0 (transcribed below).
//      The value readers pair values j and j+16 (not j and j+1), so this pins the nibble pairing end to end
//      (a consecutive pairing answers with other rows' values).
// ggml's CPU quantize_row_q4_0/dequantize_row_q4_0 are not called directly: kv_q8_test.cpp carries no ggml
// include and poly_kv_q8_test links no ggml library, and the include path and link a direct call would need
// live in CMakeLists.txt, which is outside this change — so the ground truth is byte-exact transcriptions of
// the ref, each line marked with its ggml source line.
// Exit 0 prints "poly_kv_q4_test: PASS"; any failed assertion prints the offending index and values and exits 1.
#include "strata/poly/kernels.hpp"

#include <cuda_fp16.h>
#include <cuda_runtime.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <random>
#include <vector>

namespace {

void ck(cudaError_t e, const char* what) {
    if (e != cudaSuccess) {
        std::fprintf(stderr, "poly_kv_q4_test: %s: %s\n", what, cudaGetErrorString(e));
        std::exit(1);
    }
}

struct Dev {
    void* p = nullptr;
    explicit Dev(size_t n) { ck(cudaMalloc(&p, n ? n : 1), "cudaMalloc"); }
    ~Dev() { if (p) cudaFree(p); }
    Dev(const Dev&) = delete;
    Dev& operator=(const Dev&) = delete;
    template <typename T> T* as() { return static_cast<T*>(p); }
};

uint16_t half_bits(__half h) {
    uint16_t b;
    std::memcpy(&b, &h, sizeof(b));
    return b;
}

float half_value(uint16_t b) {
    __half h;
    std::memcpy(&h, &b, sizeof(h));
    return __half2float(h);
}

// A magnitude drawn over five orders (1e-4 .. 1e3) and either sign, so small and large values mix within a block.
float rand_val(std::mt19937& rng) {
    std::uniform_real_distribution<float> u(0.f, 1.f);
    const float mag = std::pow(10.f, -4.f + 7.f * u(rng));
    return (u(rng) < 0.5f ? -1.f : 1.f) * mag;
}

// ggml's q4_0 writer, transcribed line for line from quantize_row_q4_0_ref
// (third_party/llama.cpp/ggml/src/ggml-quants.c:113-148) into this engine's split quants/scales planes (one
// fp16 scale a 32-value block beside its 16 packed bytes, rather than ggml's block_q4_0 {d, qs[16]}): the same
// arithmetic must give the same bytes.
void host_block(const float* xr, uint8_t* qr, uint16_t* scale) {
    float amax = 0.0f;   // ggml-quants.c:121-122: the largest |v|, and the value that has it
    float max = 0.0f;
    for (int j = 0; j < 32; ++j) {   // ggml-quants.c:124-130: the signed extreme, not the absolute max
        const float v = xr[j];
        if (amax < std::fabs(v)) {
            amax = std::fabs(v);
            max = v;
        }
    }
    const float d = max / -8;   // ggml-quants.c:132: max keeps its sign, so d may be negative
    const float id = d ? 1.0f / d : 0.0f;   // ggml-quants.c:133
    *scale = half_bits(__float2half(d));   // ggml-quants.c:135: stored as is (negative d allowed)
    for (int j = 0; j < 16; ++j) {   // ggml-quants.c:137-146: truncation, upper clamp only
        const float x0 = xr[j] * id;
        const float x1 = xr[j + 16] * id;
        const uint8_t xi0 = (uint8_t) std::min(15, (int) (int8_t) (x0 + 8.5f));
        const uint8_t xi1 = (uint8_t) std::min(15, (int) (int8_t) (x1 + 8.5f));
        qr[j] = xi0;
        qr[j] |= xi1 << 4;
    }
}

// ggml's q4_0 reader, transcribed from dequantize_row_q4_0
// (third_party/llama.cpp/ggml/src/ggml-quants.c:459-477) for one 32-value block held in this engine's split
// planes: the low/high nibbles of byte j hold values j and j + 16, and each dequantizes as (nibble - 8) * d
// with the fp16 scale used as is, so a negative d reads back correctly.
void ggml_dequant_block(const uint8_t* qr, uint16_t dbits, float* y) {
    const float d = half_value(dbits);   // ggml-quants.c:467
    for (int j = 0; j < 16; ++j) {   // ggml-quants.c:469-475
        const int x0 = (qr[j] & 0x0F) - 8;   // ggml-quants.c:470
        const int x1 = (qr[j] >> 4) - 8;   // ggml-quants.c:471
        y[j] = x0 * d;   // ggml-quants.c:473
        y[j + 16] = x1 * d;   // ggml-quants.c:474
    }
}

// 1. Round trip: dequantize with the fp16-rounded scale read back from `scales` and bound the error.
void round_trip(int rows, int n, std::mt19937& rng, int& fails, long& values) {
    const int nb = n / 32;
    std::vector<float> x((size_t) rows * n);
    for (float& v : x) v = rand_val(rng);

    Dev dx(x.size() * sizeof(float)), dquants((size_t) rows * n / 2),
        dscales((size_t) rows * nb * sizeof(uint16_t));
    ck(cudaMemcpy(dx.as<float>(), x.data(), x.size() * sizeof(float), cudaMemcpyHostToDevice), "upload x");
    strata::poly::store_q4(dx.as<float>(), dquants.as<uint8_t>(), dscales.as<uint16_t>(), n, 0, rows, nullptr);
    ck(cudaGetLastError(), "store_q4 launch");
    ck(cudaStreamSynchronize(nullptr), "store_q4 sync");

    std::vector<uint8_t> quants((size_t) rows * n / 2);
    std::vector<uint16_t> scales((size_t) rows * nb);
    ck(cudaMemcpy(quants.data(), dquants.p, quants.size(), cudaMemcpyDeviceToHost), "download quants");
    ck(cudaMemcpy(scales.data(), dscales.p, scales.size() * sizeof(uint16_t), cudaMemcpyDeviceToHost), "download scales");

    for (int r = 0; r < rows; ++r) {
        for (int b = 0; b < nb; ++b) {
            const float d = half_value(scales[(size_t) r * nb + b]);
            const float half_scale = std::fabs(d) * 0.5f;
            const uint8_t* qr = quants.data() + (size_t) r * (n / 2) + (size_t) b * 16;
            for (int j = 0; j < 32; ++j) {
                const size_t i = (size_t) r * n + (size_t) b * 32 + j;
                const int p = j & 31, byte = qr[p & 15];
                const float got = (float) ((p < 16 ? (byte & 0xF) : (byte >> 4)) - 8) * d;
                // A q4_0 value cannot land closer than half a block scale.  d may be negative (its sign follows
                // the block's signed extreme), so the bound uses |d|.  The scale is written fp16 from the
                // full-precision d the codes were rounded with (the kernel's id = 1/d), so decoding with the fp16
                // scale can miss by up to 15 * 2^-11 block scales at the rounding boundary (the largest nibble):
                // allow |x| * 1.5e-3 too, which covers that at the top codes.
                // The 16 codes only span (nibble - 8) * d, nibble 0..15: ggml clamps the top code at 15 with no
                // lower clamp, so a value past the far endpoint clips to it by design (a large value whose sign
                // opposes the block's signed extreme, e.g. -622 in a block peaking at +638).  Allow that
                // out-of-range distance on top of the half-scale bound.
                const float lo = std::fmin(-8.f * d, 7.f * d), hi = std::fmax(-8.f * d, 7.f * d);
                float tol = half_scale + std::fabs(x[i]) * 1.5e-3f + 1e-3f;
                if (x[i] < lo) tol += lo - x[i];
                else if (x[i] > hi) tol += x[i] - hi;
                ++values;
                if (!(std::fabs(got - x[i]) <= tol)) {
                    std::fprintf(stderr,
                                 "poly_kv_q4_test: round-trip: n=%d rows=%d value [%d,%d,%d] x=%.9g got=%.9g "
                                 "d=%.9g err=%.9g tol=%.9g\n",
                                 n, rows, r, b, j, (double) x[i], (double) got, (double) d,
                                 (double) std::fabs(got - x[i]), (double) tol);
                    ++fails;
                }
            }
        }
    }
}

// 2. Byte layout: ggml's d, nibbles and fp16 scale (host_block above) must equal the device's bytes.
void byte_layout(int rows, int n, std::mt19937& rng, int& fails, long& values) {
    const int nb = n / 32;
    std::vector<float> x((size_t) rows * n);
    for (float& v : x) v = rand_val(rng);

    std::vector<uint8_t> equants((size_t) rows * n / 2);
    std::vector<uint16_t> escales((size_t) rows * nb);
    for (int r = 0; r < rows; ++r) {
        for (int b = 0; b < nb; ++b)
            host_block(x.data() + (size_t) r * n + (size_t) b * 32,
                       equants.data() + (size_t) r * (n / 2) + (size_t) b * 16, &escales[(size_t) r * nb + b]);
    }

    Dev dx(x.size() * sizeof(float)), dquants((size_t) rows * n / 2),
        dscales((size_t) rows * nb * sizeof(uint16_t));
    ck(cudaMemcpy(dx.as<float>(), x.data(), x.size() * sizeof(float), cudaMemcpyHostToDevice), "upload x");
    strata::poly::store_q4(dx.as<float>(), dquants.as<uint8_t>(), dscales.as<uint16_t>(), n, 0, rows, nullptr);
    ck(cudaGetLastError(), "store_q4 launch");
    ck(cudaStreamSynchronize(nullptr), "store_q4 sync");

    std::vector<uint8_t> quants((size_t) rows * n / 2);
    std::vector<uint16_t> scales((size_t) rows * nb);
    ck(cudaMemcpy(quants.data(), dquants.p, quants.size(), cudaMemcpyDeviceToHost), "download quants");
    ck(cudaMemcpy(scales.data(), dscales.p, scales.size() * sizeof(uint16_t), cudaMemcpyDeviceToHost), "download scales");

    ++values;
    if (std::memcmp(equants.data(), quants.data(), quants.size()) != 0) {
        size_t i = 0;
        while (i < quants.size() && equants[i] == quants[i]) ++i;
        const int r = (int) (i / (n / 2)), rem = (int) (i % (n / 2));
        std::fprintf(stderr,
                     "poly_kv_q4_test: byte layout: quants differ at row=%d blk=%d byte=%d: host=0x%02x "
                     "device=0x%02x (n=%d rows=%d)\n",
                     r, rem / 16, rem % 16, (unsigned) equants[i], (unsigned) quants[i], n, rows);
        ++fails;
    }
    ++values;
    if (std::memcmp(escales.data(), scales.data(), scales.size() * sizeof(uint16_t)) != 0) {
        size_t i = 0;
        while (i < scales.size() && escales[i] == scales[i]) ++i;
        const int r = (int) (i / nb), b = (int) (i % nb);
        std::fprintf(stderr,
                     "poly_kv_q4_test: byte layout: scales differ at row=%d blk=%d: host=0x%04x device=0x%04x "
                     "(n=%d rows=%d)\n",
                     r, b, (unsigned) escales[i], (unsigned) scales[i], n, rows);
        ++fails;
    }
}

// 3. store_q4_at with a ring: rows land at (*at + r) % ring and match plain store_q4 from the same position.
void ring_test(std::mt19937& rng, int& fails, long& values) {
    const int n = 128, nb = n / 32, rows = 2, ring = 5;
    const long long at_val = 4;   // rows go to 4 and 0: the second wraps

    std::vector<float> x((size_t) rows * n);
    for (float& v : x) v = rand_val(rng);

    Dev dx(x.size() * sizeof(float)), d_at(sizeof(long long));
    ck(cudaMemcpy(dx.as<float>(), x.data(), x.size() * sizeof(float), cudaMemcpyHostToDevice), "upload x");
    ck(cudaMemcpy(d_at.p, &at_val, sizeof(at_val), cudaMemcpyHostToDevice), "upload at");

    Dev rquants((size_t) ring * n / 2), rscales((size_t) ring * nb * sizeof(uint16_t));
    strata::poly::store_q4_at(dx.as<float>(), rquants.as<uint8_t>(), rscales.as<uint16_t>(), n,
                              d_at.as<long long>(), rows, ring, nullptr);
    ck(cudaGetLastError(), "store_q4_at launch");
    ck(cudaStreamSynchronize(nullptr), "store_q4_at sync");

    Dev pquants((size_t) (at_val + rows) * n / 2), pscales((size_t) (at_val + rows) * nb * sizeof(uint16_t));
    strata::poly::store_q4(dx.as<float>(), pquants.as<uint8_t>(), pscales.as<uint16_t>(), n, at_val, rows, nullptr);
    ck(cudaGetLastError(), "store_q4 launch");
    ck(cudaStreamSynchronize(nullptr), "store_q4 sync");

    std::vector<uint8_t> rquant((size_t) ring * n / 2), pquant((size_t) (at_val + rows) * n / 2);
    std::vector<uint16_t> rscale((size_t) ring * nb), pscale((size_t) (at_val + rows) * nb);
    ck(cudaMemcpy(rquant.data(), rquants.p, rquant.size(), cudaMemcpyDeviceToHost), "download ring quants");
    ck(cudaMemcpy(rscale.data(), rscales.p, rscale.size() * sizeof(uint16_t), cudaMemcpyDeviceToHost),
       "download ring scales");
    ck(cudaMemcpy(pquant.data(), pquants.p, pquant.size(), cudaMemcpyDeviceToHost), "download plain quants");
    ck(cudaMemcpy(pscale.data(), pscales.p, pscale.size() * sizeof(uint16_t), cudaMemcpyDeviceToHost),
       "download plain scales");

    for (int r = 0; r < rows; ++r) {
        const size_t rr = (size_t) ((at_val + r) % ring);
        const size_t pr = (size_t) (at_val + r);
        ++values;
        if (std::memcmp(rquant.data() + rr * n / 2, pquant.data() + pr * n / 2, (size_t) n / 2) != 0) {
            size_t i = 0;
            while (i < (size_t) n / 2 && rquant[rr * n / 2 + i] == pquant[pr * n / 2 + i]) ++i;
            std::fprintf(stderr,
                         "poly_kv_q4_test: ring: row %d at %zu != plain row %zu: quant [%zu] ring=0x%02x "
                         "plain=0x%02x\n",
                         r, rr, pr, i, (unsigned) rquant[rr * n / 2 + i], (unsigned) pquant[pr * n / 2 + i]);
            ++fails;
        }
        ++values;
        if (std::memcmp(rscale.data() + rr * nb, pscale.data() + pr * nb, (size_t) nb * sizeof(uint16_t)) != 0) {
            size_t i = 0;
            while (i < (size_t) nb && rscale[rr * nb + i] == pscale[pr * nb + i]) ++i;
            std::fprintf(stderr,
                         "poly_kv_q4_test: ring: row %d at %zu != plain row %zu: scale [%zu] ring=0x%04x "
                         "plain=0x%04x\n",
                         r, rr, pr, i, (unsigned) rscale[rr * nb + i], (unsigned) pscale[pr * nb + i]);
            ++fails;
        }
    }
}

// 4. store -> attend/attend_batch vs a ggml-grounded host attention: the Q4 value path must match an attention
// computed on the host over the stored fp16 keys and the ggml-dequantized values.  Keys stay fp16 so only the
// value reader is measured (the key readers pair j/j+16 already); a consecutive j/j+1 value pairing answers
// with other values and misses by O(1).
void attend_value_check(int dim, int n_head, int kv_heads, int ctx_rows, int q_rows, std::mt19937& rng,
                        int& fails, long& values) {
    using namespace strata::poly;
    const int width = kv_heads * dim, nb = width / 32, per = n_head / kv_heads;
    const float scale = 1.0f / std::sqrt((float) dim);
    std::normal_distribution<float> nd(0.f, 1.f);

    std::vector<float> kf((size_t) ctx_rows * width), vf((size_t) ctx_rows * width),
        q((size_t) q_rows * n_head * dim);
    for (float& v : kf) v = nd(rng);
    for (float& v : vf) v = nd(rng);
    for (float& v : q) v = 2.f * nd(rng);   // spread scores, so the softmax has a few large weights

    Dev dq(q.size() * sizeof(float)), dkf(kf.size() * sizeof(float)), dvf(vf.size() * sizeof(float));
    ck(cudaMemcpy(dq.as<float>(), q.data(), q.size() * sizeof(float), cudaMemcpyHostToDevice), "upload q");
    ck(cudaMemcpy(dkf.as<float>(), kf.data(), kf.size() * sizeof(float), cudaMemcpyHostToDevice), "upload kf");
    ck(cudaMemcpy(dvf.as<float>(), vf.data(), vf.size() * sizeof(float), cudaMemcpyHostToDevice), "upload vf");

    Dev kfp16((size_t) ctx_rows * width * sizeof(uint16_t));
    store_f16(dkf.as<float>(), kfp16.as<uint16_t>(), width, 0, ctx_rows, nullptr);
    ck(cudaGetLastError(), "store_f16 k launch");
    ck(cudaStreamSynchronize(nullptr), "store_f16 sync");

    Dev vq((size_t) ctx_rows * width / 2), vqs((size_t) ctx_rows * width / 32 * sizeof(uint16_t));
    store_q4(dvf.as<float>(), vq.as<uint8_t>(), vqs.as<uint16_t>(), width, 0, ctx_rows, nullptr);
    ck(cudaGetLastError(), "store_q4 v launch");
    ck(cudaStreamSynchronize(nullptr), "store_q4 sync");

    const int chunk = 8;   // wider than q_rows, so per >= 4 takes attn_value_t_kernel behind the scores
    Dev dscore((size_t) chunk * n_head * ctx_rows * 2 * sizeof(float)),
        dout((size_t) q_rows * n_head * dim * sizeof(float));
    // the same keys in fp16, values through the Q4 path: Kvq4 with the fp16 caches null
    attend(dq.as<float>(), kfp16.as<uint16_t>(), nullptr, 0, q_rows, dscore.as<float>(), chunk, dout.as<float>(),
           dim, n_head, kv_heads, scale, nullptr, 0, 0, false, nullptr, 0, 0, Kvq8{}, Kvq8{},
           Kvq4{nullptr, nullptr}, Kvq4{vq.as<uint8_t>(), vqs.as<uint16_t>()});
    ck(cudaGetLastError(), "attend q4v launch");
    ck(cudaStreamSynchronize(nullptr), "attend q4v sync");
    std::vector<float> got((size_t) q_rows * n_head * dim);
    ck(cudaMemcpy(got.data(), dout.p, got.size() * sizeof(float), cudaMemcpyDeviceToHost), "download q4v");

    // attend_batch() reads Q4 through its fp16 workspace; same expectation below with room for the halves rounding.
    void* blas = blas_create(nullptr);
    const size_t score_floats = 4096;
    Dev bscore(score_floats * sizeof(float));
    const size_t work_floats = (size_t) q_rows * n_head * (dim + dim / 2 + 3) + 1;
    Dev work(work_floats * sizeof(float));
    const size_t work_halves = (size_t) width * 8;
    Dev kwork(work_halves * sizeof(uint16_t)), vwork(work_halves * sizeof(uint16_t));
    attend_batch(blas, dq.as<float>(), kfp16.as<uint16_t>(), nullptr, 0, q_rows, bscore.as<float>(), score_floats,
                 work.as<float>(), dout.as<float>(), dim, n_head, kv_heads, scale, nullptr, 0, 0, false, Kvq8{},
                 Kvq8{}, kwork.as<uint16_t>(), vwork.as<uint16_t>(), work_halves, Kvq4{nullptr, nullptr},
                 Kvq4{vq.as<uint8_t>(), vqs.as<uint16_t>()});
    ck(cudaGetLastError(), "attend_batch q4v launch");
    ck(cudaStreamSynchronize(nullptr), "attend_batch q4v sync");
    std::vector<float> batched(got.size());
    ck(cudaMemcpy(batched.data(), dout.p, batched.size() * sizeof(float), cudaMemcpyDeviceToHost),
       "download batched");
    blas_destroy(blas);

    // What the device wrote, read back: the fp16 keys exactly as the kernels read them, and the q4 values
    // through ggml's dequantizer — not through our writer's intent.
    std::vector<uint16_t> khalf((size_t) ctx_rows * width);
    ck(cudaMemcpy(khalf.data(), kfp16.p, khalf.size() * sizeof(uint16_t), cudaMemcpyDeviceToHost),
       "download kfp16");
    std::vector<uint8_t> vquant((size_t) ctx_rows * width / 2);
    ck(cudaMemcpy(vquant.data(), vq.p, vquant.size(), cudaMemcpyDeviceToHost), "download vquants");
    std::vector<uint16_t> vscale((size_t) ctx_rows * nb);
    ck(cudaMemcpy(vscale.data(), vqs.p, vscale.size() * sizeof(uint16_t), cudaMemcpyDeviceToHost),
       "download vscales");
    std::vector<float> kk(khalf.size()), vv((size_t) ctx_rows * width);
    for (size_t i = 0; i < khalf.size(); ++i) kk[i] = half_value(khalf[i]);
    for (int r = 0; r < ctx_rows; ++r)
        for (int b = 0; b < nb; ++b)
            ggml_dequant_block(vquant.data() + (size_t) r * (width / 2) + (size_t) b * 16,
                               vscale[(size_t) r * nb + b], vv.data() + (size_t) r * width + (size_t) b * 32);

    // The host's own attention, token t over rows 0..t like the kernels': softmax of scale * q.k, then the
    // ggml-dequantized values.  Head h reads key group h / per at g * dim.
    std::vector<float> want(got.size());
    std::vector<float> s(ctx_rows);
    for (int t = 0; t < q_rows; ++t) {
        for (int h = 0; h < n_head; ++h) {
            const float* qt = q.data() + ((size_t) t * n_head + h) * dim;
            const size_t g = (size_t) (h / per) * dim;
            float mx = -INFINITY;
            for (int i = 0; i <= t; ++i) {
                const float* ki = kk.data() + (size_t) i * width + g;
                float acc = 0.0f;
                for (int d = 0; d < dim; ++d) acc += qt[d] * ki[d];
                s[i] = acc * scale;
                mx = std::fmax(mx, s[i]);
            }
            float sum = 0.0f;
            for (int i = 0; i <= t; ++i) sum += s[i] = std::exp(s[i] - mx);
            const float inv = 1.0f / sum;
            float* out = want.data() + ((size_t) t * n_head + h) * dim;
            for (int d = 0; d < dim; ++d) out[d] = 0.0f;
            for (int i = 0; i <= t; ++i) {
                const float* vi = vv.data() + (size_t) i * width + g;
                const float w = s[i] * inv;
                for (int d = 0; d < dim; ++d) out[d] += w * vi[d];
            }
        }
    }

    // The kernels sum in float in another order than the host, so last-bit scores and weights differ: 1e-3
    // with room still pins the nibble pairing, whose errors are O(1).
    for (size_t i = 0; i < want.size(); ++i) {
        const float tol = 1e-3f + 1e-4f * std::fabs(want[i]);
        ++values;
        if (!(std::fabs(got[i] - want[i]) <= tol)) {
            std::fprintf(stderr,
                         "poly_kv_q4_test: attend: dim=%d heads=%d kv=%d value [%zu] want=%.6g q4=%.6g "
                         "err=%.6g tol=%.6g\n",
                         dim, n_head, kv_heads, i, (double) want[i], (double) got[i],
                         (double) std::fabs(got[i] - want[i]), (double) tol);
            ++fails;
            if (fails > 10) return;
        }
    }

    for (size_t i = 0; i < want.size(); ++i) {
        const float tol = 5e-3f + 5e-4f * std::fabs(want[i]);
        ++values;
        if (!(std::fabs(batched[i] - want[i]) <= tol)) {
            std::fprintf(stderr,
                         "poly_kv_q4_test: attend_batch: dim=%d heads=%d kv=%d value [%zu] want=%.6g q4=%.6g "
                         "err=%.6g tol=%.6g\n",
                         dim, n_head, kv_heads, i, (double) want[i], (double) batched[i],
                         (double) std::fabs(batched[i] - want[i]), (double) tol);
            ++fails;
            if (fails > 20) return;
        }
    }
}

}  // namespace

int main() {
    ck(cudaSetDevice(0), "cudaSetDevice");

    std::mt19937 rng(0x51u);
    int fails = 0;
    long values = 0;

    const int ns[3] = {32, 128, 256};
    const int rss[3] = {1, 3, 7};
    // many trials: 20 sweeps of (1+3+7) rows over three n = 660 random rows
    for (int t = 0; t < 20; ++t)
        for (int ri = 0; ri < 3; ++ri)
            for (int ci = 0; ci < 3; ++ci) round_trip(rss[ri], ns[ci], rng, fails, values);

    byte_layout(2, 256, rng, fails, values);
    byte_layout(3, 256, rng, fails, values);
    byte_layout(5, 256, rng, fails, values);

    ring_test(rng, fails, values);

    // per < 4 takes attn_value_kernel (no wt), per >= 4 takes attn_value_t_kernel behind the scores
    attend_value_check(128, 2, 2, 8, 4, rng, fails, values);
    attend_value_check(128, 8, 2, 8, 4, rng, fails, values);

    if (fails) {
        std::fprintf(stderr, "poly_kv_q4_test: FAIL: %d failed assertion(s)\n", fails);
        return 1;
    }
    std::printf("poly_kv_q4_test: PASS (%ld values checked)\n", values);
    return 0;
}
