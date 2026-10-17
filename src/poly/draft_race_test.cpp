// src/poly/draft_race_test.cpp - Does a read with the draft block repeat bit for bit?  Each run opens the engine,
// reads the same prompt in several calls on the layer-major path (calls of 64 tokens, stage_ahead 64), then asks the
// draft block for three guesses, one after the other.  The guesses, their probabilities and the state after the read
// must be the same in every run.  Expert swaps and the CPU's share of a small call are off, so neither changes
// between runs; the expert cache is still filled at each load.
// A read that did not give the draft block two or more calls on that path (every expert cached, no MMQ) cannot show
// the race: the test then says SKIP instead of PASS.
//
// The mixed model-knob x draft-knob arm (plan Phase 4) then walks the same prompt under six knob pairs - the
// fp16/fp16 control first, then each mixed direction - with speculative rounds shaped like main.cpp's serve loop
// (draft three, window four, greedily verify, keep the run that was right).  Mixed-format corruption presents as
// acceptance loss, not a crash (plan §6), so the arm FAILs loudly on silent corruption (tool-call equivalence,
// deterministic-prefix exact-match, acceptance deltas vs the control with the plan's gates) rather than merely
// slowing down; it also walks save_rows/load_rows under each pair (write under one knob pair, read under the same)
// and checks a knob change is a clean size mismatch, never a silent misread.
//
//   poly_draft_race_test MODEL.gguf [RUNS] [LAYER_SPLIT]     (RUNS >= 2, default 8; LAYER_SPLIT: as --layer-split,
//
//                                                            empty: one card)
//
// Needs the model file and a card, so it is run by hand (no add_test, by design).  Exit 0: the race repeated and
// every mixed pair met the gates; 1: a run differed or a gate failed; 2: an error or a bad argument; 3: SKIP.
#include "engine.hpp"

#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

using strata::poly::Engine;
using strata::poly::KvFmt;
using strata::poly::Options;
using strata::poly::most_likely;

namespace {

struct Seen {
    int64_t draft_calls = 0;   // the read's calls of the draft block's rows on the layer-major path
    std::vector<int32_t> guesses;
    std::vector<float> probs;
    std::vector<uint8_t> state;
};

Seen run_once(const Options& o, const std::vector<int32_t>& text, int n) {
    auto e = Engine::open(o);
    if (!e->drafts()) throw std::runtime_error("the model has no draft block, or it does not run");
    const int64_t before = e->read_draft_calls_for_test();
    if (!e->read(text.data(), n)) throw std::runtime_error("the read was given up");
    Seen s;
    s.draft_calls = e->read_draft_calls_for_test() - before;
    s.state.resize(e->state_bytes());
    e->save_state(s.state.data());
    int32_t g = text[(size_t) n];
    for (int i = 0; i < 3; ++i) {
        float p = 0.f;
        g = e->draft(g, i, &p);
        s.guesses.push_back(g);
        s.probs.push_back(p);
    }
    return s;
}

// The mixed model-knob x draft-knob arm (plan §4 Phase 4, gates §5, risk §6).  The plan's risk is that
// mixed-format corruption presents as acceptance loss, not a crash: a wrongly-read draft block still answers,
// just worse, and the serve loop would merely slow down.  So this arm FAILs loudly on silent corruption:
//
//   tool-equiv: the walk's verified (kept) tokens rendered one tool call each, name + JSON args,
//               order-sensitive per call, must equal the control's 100% (plan §5 Phase-2 "tool-equiv fail
//               = FAIL").  The engine has no tool layer; the kept stream is what the harness feeds tool
//               parsing, so exact equality here is the engine-level equivalent.
//   prefix:     the kept token IDs must exactly equal the control's over the whole walk (plan §5
//               deterministic-prefix exact-match; Phase-1/2 "prefix 5/5").
//   acceptance: each pair's own draft-acceptance (guesses the greedy window-verify keeps, counted the way
//               main.cpp counts guessed_right/guessed) vs the control's; a silent drop over 5pp FAILs
//               (plan §5 Phase-2 ">5pp silent drop ... = FAIL, no proceed").  ms/round is reported, never
//               a gate: slowness alone is not the signal (§6).
//   rows:       each pair's save_rows/load_rows round trip must be bit-exact (write under one knob pair,
//               read under the same), and any two pairs with different knobs must have different
//               row_bytes(), so a knob change is a clean size mismatch (reject-or-empty), never a silent
//               misread.  Blobs never cross processes: both knobs are fixed at load (Engine::save_rows'
//               Phase-1a note), so no version tag is needed while they stay process-static.
//
// A pair whose Engine::open() hits a --kv/--draft-kv arch gate (not a qwen35moe model) SKIPs like the race
// does above, it does not FAIL: the gate is the specified behavior (plan §3).

struct Knob {
    const char* name;
    KvFmt kv;
    KvFmt draft;
};

// The control first, then every mixed direction: each knob quantized alone, both together, and the two
// cross pairs (target layers vs draft block in different formats).
const Knob kKnobs[] = {
    {"fp16/fp16", KvFmt::kFp16, KvFmt::kFp16},
    {"q8_0/fp16", KvFmt::kQ8_0, KvFmt::kFp16},
    {"fp16/q8_0", KvFmt::kFp16, KvFmt::kQ8_0},
    {"q4_0/q4_0", KvFmt::kQ4_0, KvFmt::kQ4_0},
    {"q8_0/q4_0", KvFmt::kQ8_0, KvFmt::kQ4_0},
    {"q4_0/q8_0", KvFmt::kQ4_0, KvFmt::kQ8_0},
};

const int kWalkRounds = 16;   // speculative rounds a pair; 48 offered guesses, so 5pp is about two tokens

struct Mixed {
    bool skipped = false;
    std::string skip_why;
    int64_t draft_calls = 0;
    std::vector<int32_t> guesses;   // every round's three draft tokens, for the agreement report
    std::vector<int32_t> kept;      // the walk's verified tokens: the deterministic prefix (§5)
    std::vector<std::string> tools;   // one tool call a kept token (§5 tool-equiv proxy, see above)
    long offered = 0, accepted = 0;
    double ms_round = 0;
    size_t row_bytes = 0;
    bool rows_ok = false;   // the same-pair save_rows/load_rows round trip was bit-exact
};

bool gate_reject(const std::exception& ex) {
    // Engine::open()'s per-knob arch gates (plan §3): a non-fp16 knob on a model that is not qwen35moe.
    return std::strstr(ex.what(), "only supported for") != nullptr;
}

int32_t greedy(const Engine& e, const float* logits) { return (int32_t) most_likely(logits, e.n_vocab()); }

// One knob pair's walk: the same read as run_once (the layer-major path with stage_ahead 64), a same-pair
// save_rows/load_rows round trip, then speculative rounds shaped like main.cpp's serve loop: draft three from
// the token at pos(), window those four, greedily verify the guesses against the target layers' logits_of,
// and keep the run that was right.  The draft probabilities are deliberately not gated: they are floats the
// quantization legitimately moves; what must not move silently are the verified tokens (tool-equiv, prefix)
// and, beyond 5pp, the acceptance rate.
Mixed run_mixed(const Options& base, const Knob& k, const std::vector<int32_t>& text, int n, int rounds) {
    Options o = base;
    o.kv_format = k.kv;
    o.draft_kv_format = k.draft;
    Mixed m;
    auto e = Engine::open(o);
    if (!e->drafts()) throw std::runtime_error("the model has no draft block, or it does not run");
    const int64_t before = e->read_draft_calls_for_test();
    if (!e->read(text.data(), n)) throw std::runtime_error("the read was given up");
    m.draft_calls = e->read_draft_calls_for_test() - before;
    if (m.draft_calls < 2) {
        m.skipped = true;
        m.skip_why = "the read gave the draft block fewer than two calls on the layer-major path";
        return m;
    }
    // The rows walk: write under this pair, read under the same, and check the bytes came back as written.
    // save/load_rows are byte copies between the caches and host RAM (qwen35.cpp), so a same-pair round
    // trip must be bit-exact; the cross-pair check in main() then shows a knob change cannot silently reuse
    // these bytes (different row_bytes(), and blobs never cross processes anyway).
    const int64_t pos = e->pos();
    m.row_bytes = e->row_bytes();
    std::vector<uint8_t> a((size_t) pos * m.row_bytes), b(a.size());
    e->save_rows(0, pos, a.data());
    e->load_rows(a.data(), 0, pos, 0, pos);
    e->save_rows(0, pos, b.data());
    m.rows_ok = a == b;
    // The decode walk from the token at pos(), the same one run_once drafts from.
    int32_t w[4];
    int32_t t = text[(size_t) n];
    int seq = 0;
    char call[64];
    const auto t0 = std::chrono::steady_clock::now();
    for (int r = 0; r < rounds; ++r) {
        int32_t g = t;
        for (int i = 0; i < 3; ++i) {
            float p = 0.f;
            g = e->draft(g, i, &p);
            m.guesses.push_back(g);
        }
        w[0] = t;
        w[1] = m.guesses[(size_t) r * 3];
        w[2] = m.guesses[(size_t) r * 3 + 1];
        w[3] = m.guesses[(size_t) r * 3 + 2];
        e->window(w, 4);
        int right = 0;
        while (right < 3 && greedy(*e, e->logits_of(right)) == w[right + 1]) ++right;
        m.offered += 3;
        m.accepted += right;
        for (int j = 0; j <= right; ++j) {
            m.kept.push_back(w[j]);
            std::snprintf(call, sizeof call, "emit({\"seq\":%d,\"tok\":%d})", seq++, w[j]);
            m.tools.push_back(call);
        }
        t = greedy(*e, e->logits_of(right));
        e->keep(right + 1);
    }
    m.kept.push_back(t);
    std::snprintf(call, sizeof call, "emit({\"seq\":%d,\"tok\":%d})", seq++, t);
    m.tools.push_back(call);
    const double ms =
        std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - t0).count();
    m.ms_round = ms / (double) rounds;
    return m;
}

}  // namespace

int main(int argc, char** argv) {
    if (argc < 2) {
        std::fprintf(stderr, "usage: %s MODEL.gguf [RUNS] [LAYER_SPLIT]\n", argv[0]);
        return 2;
    }
    Options o;
    o.model = argv[1];
    int runs = 8;
    if (argc > 2) {
        char* end = nullptr;
        const long v = std::strtol(argv[2], &end, 10);
        if (end == argv[2] || *end != '\0' || v < 2 || v > 1000) {
            std::fprintf(stderr, "RUNS is a whole number from 2 to 1000 (one run compares nothing), not '%s'\n", argv[2]);
            return 2;
        }
        runs = (int) v;
    }
    if (argc > 3) o.layer_split = argv[3];
    o.max_context = 4096;
    o.prompt_batch = 64;   // a read of 200 tokens is four calls
    o.stage_ahead = 64;    // and takes the layer-major path, where the draft block's rows come last
    o.window = 4;
    o.expert_swaps = 0;
    o.cpu_share = 0;
    const int n = 200;
    std::vector<int32_t> text(n + 1);
    for (int i = 0; i <= n; ++i) text[(size_t) i] = 1000 + (53 * i + i / 9) % 100000;
    try {
        const Seen first = run_once(o, text, n);
        if (first.draft_calls < 2) {
            std::fprintf(stderr, "SKIP: the read gave the draft block %lld call(s) on the layer-major path; the race "
                                 "needs two or more (every expert cached, or no MMQ?)\n", (long long) first.draft_calls);
            return 3;
        }
        std::fprintf(stderr, "the read gave the draft block %lld calls on the layer-major path\n",
                     (long long) first.draft_calls);
        int differ = 0;
        for (int k = 1; k < runs; ++k) {
            const Seen s = run_once(o, text, n);
            const bool g = s.guesses == first.guesses;
            const bool p = std::memcmp(s.probs.data(), first.probs.data(), s.probs.size() * sizeof(float)) == 0;
            const bool st = s.state == first.state;
            if (s.draft_calls != first.draft_calls) throw std::runtime_error("a run took another read path");
            std::fprintf(stderr, "run %d: guesses %s, probabilities %s, state %s\n", k + 1, g ? "same" : "DIFFER",
                         p ? "same" : "DIFFER", st ? "same" : "DIFFER");
            differ += !(g && p && st);
        }
        std::fprintf(stderr, "%s: %d of %d runs differ from the first (split '%s')\n", differ ? "FAIL" : "PASS", differ,
                     runs - 1, o.layer_split.c_str());
        // The mixed-format arm: the control first, then each mixed pair against it.  A pair the arch gates
        // reject SKIPs (plan §3); anything else thrown is a hard error like the race's.
        std::vector<Mixed> arm;
        for (size_t a = 0; a < sizeof kKnobs / sizeof kKnobs[0]; ++a) {
            Mixed m;
            try {
                m = run_mixed(o, kKnobs[a], text, n, kWalkRounds);
            } catch (const std::exception& ex) {
                if (!gate_reject(ex)) throw;
                m.skipped = true;
                m.skip_why = ex.what();
            }
            if (m.skipped) {
                std::fprintf(stderr, "mixed %s: SKIP: %s\n", kKnobs[a].name, m.skip_why.c_str());
            }
            arm.push_back(m);
        }
        if (arm[0].skipped) {
            std::fprintf(stderr, "mixed: SKIP: the control (fp16/fp16) cannot show the walk: %s\n",
                         arm[0].skip_why.c_str());
            return 3;
        }
        const Mixed& ctrl = arm[0];
        const double ctrl_acc = 100.0 * (double) ctrl.accepted / (double) ctrl.offered;
        std::fprintf(stderr, "mixed control fp16/fp16: acceptance %ld/%ld = %.2f%%, %d kept, row %zu bytes, "
                             "%.2f ms/round\n",
                     ctrl.accepted, ctrl.offered, ctrl_acc, (int) ctrl.kept.size(), ctrl.row_bytes,
                     ctrl.ms_round);
        int mixed_fails = 0, ran_all = 0, rows_ok = 0;
        for (size_t a = 0; a < arm.size(); ++a) {
            if (arm[a].skipped) continue;
            ++ran_all;
            rows_ok += arm[a].rows_ok ? 1 : 0;
        }
        if (!ctrl.rows_ok) {
            std::fprintf(stderr, "mixed fp16/fp16: FAIL: the same-pair rows round trip differs\n");
            ++mixed_fails;
        }
        for (size_t a = 1; a < arm.size(); ++a) {
            const Mixed& m = arm[a];
            if (m.skipped) continue;
            const double acc = 100.0 * (double) m.accepted / (double) m.offered;
            const double drop = ctrl_acc - acc;
            long agree = 0;
            for (size_t i = 0; i < m.guesses.size() && i < ctrl.guesses.size(); ++i)
                agree += m.guesses[i] == ctrl.guesses[i] ? 1 : 0;
            std::fprintf(stderr, "mixed %s: acceptance %ld/%ld = %.2f%% (control %.2f%%, delta %+.2fpp), draft "
                                 "agree %ld/%ld, %d kept, row %zu bytes, %.2f ms/round\n",
                         kKnobs[a].name, m.accepted, m.offered, acc, ctrl_acc, acc - ctrl_acc, agree,
                         (long) ctrl.guesses.size(), (int) m.kept.size(), m.row_bytes, m.ms_round);
            if (!m.rows_ok) {
                std::fprintf(stderr, "mixed %s: FAIL: the same-pair rows round trip differs (write under %s, "
                                     "read under the same)\n",
                             kKnobs[a].name, kKnobs[a].name);
                ++mixed_fails;
            }
            if (m.kept != ctrl.kept) {
                size_t i = 0;
                while (i < m.kept.size() && i < ctrl.kept.size() && m.kept[i] == ctrl.kept[i]) ++i;
                std::fprintf(stderr, "mixed %s: FAIL: deterministic prefix differs at kept %zu (got %d, "
                                     "control %d; lengths %d vs %d)\n",
                             kKnobs[a].name, i, i < m.kept.size() ? m.kept[i] : -1,
                             i < ctrl.kept.size() ? ctrl.kept[i] : -1, (int) m.kept.size(),
                             (int) ctrl.kept.size());
                ++mixed_fails;
            }
            if (m.tools != ctrl.tools) {
                size_t i = 0;
                while (i < m.tools.size() && i < ctrl.tools.size() && m.tools[i] == ctrl.tools[i]) ++i;
                std::fprintf(stderr, "mixed %s: FAIL: tool-call equivalence differs at call %zu (got %s, control "
                                     "%s; %d vs %d calls)\n",
                             kKnobs[a].name, i, i < m.tools.size() ? m.tools[i].c_str() : "(none)",
                             i < ctrl.tools.size() ? ctrl.tools[i].c_str() : "(none)", (int) m.tools.size(),
                             (int) ctrl.tools.size());
                ++mixed_fails;
            }
            if (drop > 5.0) {
                std::fprintf(stderr, "mixed %s: FAIL: acceptance drops %.2fpp vs control (%.2f%% vs %.2f%%; the "
                                     "plan allows 5pp)\n",
                             kKnobs[a].name, drop, acc, ctrl_acc);
                ++mixed_fails;
            }
        }
        // A knob change must be a clean size mismatch, never a silent misread: any two pairs with different
        // knobs share no blob layout, and blobs never cross processes (Engine::save_rows' Phase-1a note).
        for (size_t a = 0; a < arm.size(); ++a) {
            for (size_t b = a + 1; b < arm.size(); ++b) {
                if (arm[a].skipped || arm[b].skipped) continue;
                if (kKnobs[a].kv == kKnobs[b].kv && kKnobs[a].draft == kKnobs[b].draft) continue;
                if (arm[a].row_bytes == arm[b].row_bytes) {
                    std::fprintf(stderr, "mixed: FAIL: %s and %s share row_bytes %zu: a knob change would not be "
                                         "a clean size mismatch\n",
                                 kKnobs[a].name, kKnobs[b].name, arm[a].row_bytes);
                    ++mixed_fails;
                }
            }
        }
        if (differ || mixed_fails) {
            std::fprintf(stderr, "draft_race_test: FAIL: race %d differ, mixed %d failed assertion(s) (%d/%d "
                                 "pairs ran, rows round-trip %d/%d)\n",
                         differ, mixed_fails, ran_all - 1, (int) arm.size() - 1, rows_ok, ran_all);
            return 1;
        }
        std::printf("draft_race_test: PASS (race %d runs match, mixed %d/%d pairs, rows round-trip %d/%d)\n",
                    runs - 1, ran_all - 1, (int) arm.size() - 1, rows_ok, ran_all);
        return 0;
    } catch (const std::exception& ex) {
        std::fprintf(stderr, "ERROR: %s\n", ex.what());
        return 2;
    }
}
