#pragma once

#include <chrono>
#include <cstdint>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>


class TimingReport {
public:
    using clock = std::chrono::steady_clock;

    void add_ns(const std::string& key, std::uint64_t ns) {
        auto& s = stats_[key];
        s.total_ns += ns;
        s.count += 1;
        if (ns < s.min_ns) s.min_ns = ns;
        if (ns > s.max_ns) s.max_ns = ns;
    }

    void reset() { stats_.clear(); }

    void print(std::ostream& os = std::cout) const {
        os << "\n==== Timing Report (chrono::steady_clock) ====\n";
        for (const auto& [key, s] : stats_) {
            double total_ms = s.total_ns / 1e6;
            double avg_ms   = (s.count ? (s.total_ns / 1e6 / s.count) : 0.0);
            double min_ms   = (s.min_ns == UINT64_MAX ? 0.0 : s.min_ns / 1e6);
            double max_ms   = s.max_ns / 1e6;

            os << key << ":\n"
               << "  count: " << s.count << "\n"
               << "  total: " << total_ms << " ms\n"
               << "  avg  : " << avg_ms   << " ms\n"
               << "  min  : " << min_ms   << " ms\n"
               << "  max  : " << max_ms   << " ms\n";
        }
        os << "============================================\n";
    }

private:
    struct Stat {
        std::uint64_t total_ns = 0;
        std::uint64_t count = 0;
        std::uint64_t min_ns = UINT64_MAX;
        std::uint64_t max_ns = 0;
    };
    std::unordered_map<std::string, Stat> stats_;
};

class ScopedTimer {
public:
    ScopedTimer(TimingReport& report, std::string key)
        : report_(report), key_(std::move(key)), start_(TimingReport::clock::now()) {}

    ~ScopedTimer() {
        auto end = TimingReport::clock::now();
        auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start_).count();
        report_.add_ns(key_, static_cast<std::uint64_t>(ns));
    }

    ScopedTimer(const ScopedTimer&) = delete;
    ScopedTimer& operator=(const ScopedTimer&) = delete;

private:
    TimingReport& report_;
    std::string key_;
    TimingReport::clock::time_point start_;
};
