// src/poly/sink_attend_test.cu - attend_batch() against attend() for Flint's attention: heads of 128 values in
// groups of 8, a learned sink a head, and a window layer's ring as well as a full layer's rows.  attend()'s plain
// kernels are what was checked against llama.cpp; attend_batch() rounds the queries and the weights to halves on the
// tensor cores, so the two differ in the third or fourth digit.  A full layer's rows are also read by attend()'s
// blocks of rows (score_floats), which differ from its plain kernels in the last bits only.  A last arm stores the
// same rows as q4_0 (store_q4) and checks the blocks of rows' q4 read against the plain kernel's.
//
// usage: poly_sink_attend_test   (exit 0: every case within 4e-3, the blocks of rows within 1e-4, and no NaN)
#include "strata/poly/kernels.hpp"

#include <cublas_v2.h>
#include <cuda_fp16.h>
#include <cuda_runtime.h>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <random>
#include <vector>

namespace {

using namespace strata::poly;

constexpr int kDim = 128, kHeads = 32, kKvHeads = 4, kChunk = 8, kWindow = 513, kBatch = 1024;
constexpr int kMaxRows = 8192, kRing = kWindow + kBatch;

template <typename T> T* upload(const std::vector<T>& v) {
    T* d = nullptr;
    cudaMalloc(&d, v.size() * sizeof(T));
    cudaMemcpy(d, v.data(), v.size() * sizeof(T), cudaMemcpyHostToDevice);
    return d;
}

std::vector<float> download(const float* d, size_t n) {
    std::vector<float> v(n);
    cudaMemcpy(v.data(), d, n * sizeof(float), cudaMemcpyDeviceToHost);
    return v;
}

struct Buffers {
    float *q, *score, *out, *work, *sinks;
    uint16_t *k, *v;
    size_t score_n;
    cublasHandle_t blas;
};

// Flint's caches stored as q4_0: the same rows, the split kernel's q4 read against attend()'s plain one.
struct Q4 {
    uint8_t *kq, *vq;
    uint16_t *ks, *vs;
};

int check_q4(const Buffers& b, const Q4& c, long long pos, int rows, const char* what) {
    const size_t n = size_t(rows) * kHeads * kDim;
    const float scale = 1.f / std::sqrt((float) kDim);
    std::vector<float> ref(n);
    for (int first = 0; first < rows; first += kChunk) {
        const int cc = std::min(kChunk, rows - first);
        const size_t o = size_t(first) * kHeads * kDim;
        attend(b.q + o, nullptr, nullptr, pos + first, cc, b.score, kChunk, b.out, kDim, kHeads, kKvHeads, scale, nullptr,
               0, 0, false, nullptr, 0, 0, {}, {}, b.sinks, 0, Kvq4{c.kq, c.ks}, Kvq4{c.vq, c.vs});
        cudaMemcpy(ref.data() + o, b.out, size_t(cc) * kHeads * kDim * sizeof(float), cudaMemcpyDeviceToHost);
    }
    float split = 0.f, big = 0.f;
    long long nan_at = -1;
    for (int first = 0; first < rows; first += kChunk) {
        const int cc = std::min(kChunk, rows - first);
        const size_t o = size_t(first) * kHeads * kDim;
        attend(b.q + o, nullptr, nullptr, pos + first, cc, b.score, kChunk, b.out, kDim, kHeads, kKvHeads, scale, nullptr,
               0, 0, false, nullptr, 0, b.score_n, {}, {}, b.sinks, 0, Kvq4{c.kq, c.ks}, Kvq4{c.vq, c.vs});
        const std::vector<float> got = download(b.out, size_t(cc) * kHeads * kDim);
        for (size_t i = 0; i < got.size(); ++i) {
            if (std::isnan(got[i]) && nan_at < 0) nan_at = (long long) (o + i);
            split = std::max(split, std::fabs(got[i] - ref[o + i]));
        }
    }
    for (float x : ref) big = std::max(big, std::fabs(x));
    const bool ok = nan_at < 0 && split <= 1e-4f;
    std::printf("%s q4 %s pos %5lld rows %4d: %.2e from attend() (results up to %.2f)", ok ? "ok  " : "FAIL", what, pos,
                rows, split, big);
    if (nan_at >= 0)
        std::printf(", first NaN at token %lld head %lld", nan_at / (kHeads * kDim), nan_at / kDim % kHeads);
    std::printf("\n");
    return ok ? 0 : 1;
}

// rows tokens from pos, in a window layer's ring (window > 0) or a full layer's rows
int check(const Buffers& b, long long pos, int rows, int window, const char* what) {
    const size_t n = size_t(rows) * kHeads * kDim;
    const float scale = 1.f / std::sqrt((float) kDim);
    const int ring = window ? kRing : 0;
    std::vector<float> ref(n);
    for (int first = 0; first < rows; first += kChunk) {
        const int c = std::min(kChunk, rows - first);
        const size_t o = size_t(first) * kHeads * kDim;
        attend(b.q + o, b.k, b.v, pos + first, c, b.score, kChunk, b.out, kDim, kHeads, kKvHeads, scale, nullptr,
               window, ring, false, nullptr, 0, 0, {}, {}, b.sinks);
        cudaMemcpy(ref.data() + o, b.out, size_t(c) * kHeads * kDim * sizeof(float), cudaMemcpyDeviceToHost);
    }
    attend_batch(b.blas, b.q, b.k, b.v, pos, rows, b.score, b.score_n, b.work, b.out, kDim, kHeads, kKvHeads, scale,
                 nullptr, window, ring, false, {}, {}, nullptr, nullptr, 0, b.sinks);
    const std::vector<float> batch = download(b.out, n);
    float most = 0.f, big = 0.f;
    long long nan_at = -1;
    for (size_t i = 0; i < n; ++i) {
        if (std::isnan(batch[i]) && nan_at < 0) nan_at = (long long) i;
        most = std::max(most, std::fabs(batch[i] - ref[i]));
        big = std::max(big, std::fabs(ref[i]));
    }
    // a full layer: attend() with room for its blocks of rows takes attn_split_kernel and attn_join_kernel
    float split = 0.f;
    if (window == 0)
        for (int first = 0; first < rows; first += kChunk) {
            const int c = std::min(kChunk, rows - first);
            const size_t o = size_t(first) * kHeads * kDim;
            attend(b.q + o, b.k, b.v, pos + first, c, b.score, kChunk, b.out, kDim, kHeads, kKvHeads, scale, nullptr,
                   0, 0, false, nullptr, 0, b.score_n, {}, {}, b.sinks);
            const std::vector<float> got = download(b.out, size_t(c) * kHeads * kDim);
            for (size_t i = 0; i < got.size(); ++i) {
                if (std::isnan(got[i]) && nan_at < 0) nan_at = (long long) (o + i);
                split = std::max(split, std::fabs(got[i] - ref[o + i]));
            }
        }
    const bool ok = nan_at < 0 && most <= 4e-3f && split <= 1e-4f;
    std::printf("%s %s pos %5lld rows %4d: %.2e from attend() (results up to %.2f)", ok ? "ok  " : "FAIL", what, pos,
                rows, most, big);
    if (window == 0) std::printf(", blocks of rows %.2e", split);
    if (nan_at >= 0)
        std::printf(", first NaN at token %lld head %lld", nan_at / (kHeads * kDim), nan_at / kDim % kHeads);
    std::printf("\n");
    return ok ? 0 : 1;
}

}  // namespace

// --bench: one token's decode at a context of `rows` rows, q4_0, the plain kernel against the blocks of rows (what
// the split path saves a token), in milliseconds a call.
int bench_decode(int rows, int reps) {
    const size_t width = size_t(kKvHeads) * kDim, values = size_t(rows) * width;
    std::mt19937 rng(11);
    std::normal_distribution<float> nd(0.f, 1.f);
    std::vector<float> kf(values), vf(values), q(size_t(kHeads) * kDim), sinks(kHeads, 0.5f);
    for (float& x : kf) x = nd(rng);
    for (float& x : vf) x = nd(rng);
    for (float& x : q) x = 2.f * nd(rng);
    float* kfd = upload(kf);
    float* vfd = upload(vf);
    float* qd = upload(q);
    float* sinksd = upload(sinks);
    float* out = nullptr;
    uint8_t *kq, *vq;
    uint16_t *ks, *vs;
    cudaMalloc(&kq, values / 2);
    cudaMalloc(&vq, values / 2);
    cudaMalloc(&ks, values / 32 * sizeof(uint16_t));
    cudaMalloc(&vs, values / 32 * sizeof(uint16_t));
    cudaMalloc(&out, q.size() * sizeof(float));
    store_q4(kfd, kq, ks, (int) width, 0, rows, nullptr);
    store_q4(vfd, vq, vs, (int) width, 0, rows, nullptr);
    const int blocks = (rows + 1024 - 1) / 1024;
    const size_t split_need = size_t(kHeads) * (blocks + 1) * (kDim + 2);
    const size_t plain_need = size_t(kChunk) * kHeads * size_t(rows);   // the plain kernel's scores: chunk * heads * keys
    const size_t need = std::max(split_need, plain_need);
    float* score = nullptr;
    cudaMalloc(&score, need * sizeof(float));
    const float scale = 1.f / std::sqrt((float) kDim);
    const Kvq4 k4{kq, ks}, v4{vq, vs};
    cudaEvent_t e0, e1;
    cudaEventCreate(&e0);
    cudaEventCreate(&e1);
    float plain = 0.f, split = 0.f;
    const auto time_it = [&](size_t sf) {
        cudaEventRecord(e0);
        for (int rep = 0; rep < reps; ++rep)
            attend(qd, nullptr, nullptr, rows - 1, 1, score, kChunk, out, kDim, kHeads, kKvHeads, scale, nullptr, 0, 0,
                   false, nullptr, 0, sf, {}, {}, sinksd, 0, k4, v4);
        cudaEventRecord(e1);
        cudaEventSynchronize(e1);
        float ms = 0.f;
        cudaEventElapsedTime(&ms, e0, e1);
        if (const cudaError_t e = cudaGetLastError(); e != cudaSuccess) {
            std::printf("CUDA: %s\n", cudaGetErrorString(e));
            return -1.f;
        }
        return ms / reps;
    };
    for (int warm = 0; warm < 2; ++warm) {
        plain = time_it(0);
        split = time_it(need);
    }
    std::printf("q4 decode, %d rows, one token: plain kernel %.3f ms, blocks of rows %.3f ms (%.1fx)\n", rows, plain,
                split, plain / split);
    cudaEventDestroy(e0);
    cudaEventDestroy(e1);
    cudaFree(kfd), cudaFree(vfd), cudaFree(qd), cudaFree(sinksd), cudaFree(out), cudaFree(kq), cudaFree(vq),
        cudaFree(ks), cudaFree(vs), cudaFree(score);
    return 0;
}

int main(int argc, char** argv) {
    if (argc > 2 && std::string(argv[1]) == "--bench") return bench_decode(std::atoi(argv[2]), 5);
    std::mt19937 rng(7);
    std::normal_distribution<float> nd(0.f, 1.f);
    std::vector<float> q(size_t(kBatch) * kHeads * kDim), sinks(kHeads);
    for (float& x : q) x = 2.f * nd(rng);   // spread scores, so the softmax has a few large weights
    for (float& x : sinks) x = 2.f * nd(rng);
    std::vector<uint16_t> k(size_t(kMaxRows) * kKvHeads * kDim), v(k.size());
    for (uint16_t& x : k) x = __half_as_ushort(__float2half(nd(rng)));
    for (uint16_t& x : v) x = __half_as_ushort(__float2half(nd(rng)));
    Buffers b{};
    b.q = upload(q), b.k = upload(k), b.v = upload(v), b.sinks = upload(sinks);
    b.score_n = size_t(64) * kHeads * kMaxRows;   // as the engine sizes it: blocks of rows as long as a prompt's
    cudaMalloc(&b.score, b.score_n * sizeof(float));
    cudaMalloc(&b.out, q.size() * sizeof(float));
    const size_t work = size_t(kBatch) * kHeads * (kDim + kDim / 2 + 3) + 1;
    cudaMalloc(&b.work, work * sizeof(float));
    cublasCreate(&b.blas);
    // the same caches as q4_0: store_q4 on the card from the halves' values
    std::vector<float> kf(k.size()), vf(v.size());
    for (size_t i = 0; i < k.size(); ++i) kf[i] = __half2float(__ushort_as_half(k[i]));
    for (size_t i = 0; i < v.size(); ++i) vf[i] = __half2float(__ushort_as_half(v[i]));
    float* kfd = upload(kf);
    float* vfd = upload(vf);
    const size_t width = size_t(kKvHeads) * kDim, qrows = kMaxRows * width;
    Q4 c{};
    cudaMalloc(&c.kq, qrows / 2);
    cudaMalloc(&c.vq, qrows / 2);
    cudaMalloc(&c.ks, qrows / 32 * sizeof(uint16_t));
    cudaMalloc(&c.vs, qrows / 32 * sizeof(uint16_t));
    store_q4(kfd, c.kq, c.ks, (int) width, 0, kMaxRows, nullptr);
    store_q4(vfd, c.vq, c.vs, (int) width, 0, kMaxRows, nullptr);
    int bad = 0;
    bad += check(b, 0, 100, 0, "full  ");
    bad += check(b, 0, 600, 0, "full  ");
    bad += check(b, 3000, 740, 0, "full  ");
    bad += check(b, 0, 100, kWindow, "window");
    bad += check(b, 0, 600, kWindow, "window");
    bad += check(b, 1024, 1024, kWindow, "window");
    bad += check(b, 3000, 740, kWindow, "window");
    bad += check_q4(b, c, 0, 100, "full  ");
    bad += check_q4(b, c, 0, 600, "full  ");
    bad += check_q4(b, c, 3000, 740, "full  ");
    cublasDestroy(b.blas);
    cudaFree(b.q), cudaFree(b.k), cudaFree(b.v), cudaFree(b.sinks), cudaFree(b.score), cudaFree(b.out),
        cudaFree(b.work), cudaFree(kfd), cudaFree(vfd), cudaFree(c.kq), cudaFree(c.vq), cudaFree(c.ks),
        cudaFree(c.vs);
    if (const cudaError_t e = cudaGetLastError(); e != cudaSuccess) {
        std::printf("CUDA: %s\n", cudaGetErrorString(e));
        return 1;
    }
    std::printf(bad ? "%d failed\n" : "all passed\n", bad);
    return bad ? 1 : 0;
}
