#pragma once

#include <cstdint>
#include <stdexcept>
#include <vector>

enum class PolyFormat {
    COEFFICIENT = 0,
    EVALUATION = 1,
};

struct PolyParams {
    uint32_t n;
    uint32_t q;
    PolyFormat fmt;

    constexpr PolyParams(uint32_t n_, uint32_t q_, PolyFormat f = PolyFormat::COEFFICIENT)
        : n(n_), q(q_), fmt(f) {}
};

class IRandomGenerator {
public:
    virtual ~IRandomGenerator() = default;

    virtual uint32_t SampleMod(uint32_t q, uint64_t idx) = 0;
};

class UniformGenerator : public IRandomGenerator {
public:
    explicit UniformGenerator(uint64_t seed = 0x123456789abcdefULL) : seed_(seed) {}

    uint32_t SampleMod(uint32_t q, uint64_t idx) override {
        uint64_t x = SplitMix64(seed_ ^ idx);
        return static_cast<uint32_t>(x % q);
    }

private:
    uint64_t seed_;

    static uint64_t SplitMix64(uint64_t x) {
        x += 0x9e3779b97f4a7c15ULL;
        x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ULL;
        x = (x ^ (x >> 27)) * 0x94d049bb133111ebULL;
        return x ^ (x >> 31);
    }
};

class Poly {
public:
    explicit Poly(const PolyParams& params)
        : params_(params), coeffs_(params.n, 0) {}

    Poly(const PolyParams& params, IRandomGenerator& gen)
        : params_(params), coeffs_(params.n, 0) {
        for (uint32_t i = 0; i < params_.n; i++) {
            coeffs_[i] = gen.SampleMod(params_.q, i);
        }
    }

    Poly(const PolyParams& params, const std::vector<uint32_t>& vec)
        : params_(params), coeffs_(vec) {
        if (coeffs_.size() != params_.n) {
            throw std::runtime_error("Poly: vector size mismatch with params.n");
        }
        for (auto& x : coeffs_) x %= params_.q;
    }

    uint32_t size() const { return static_cast<uint32_t>(coeffs_.size()); }
    uint32_t modulus() const { return params_.q; }
    const PolyParams& params() const { return params_; }

    const std::vector<uint32_t>& data() const { return coeffs_; }
    std::vector<uint32_t>& data() { return coeffs_; }

    uint32_t operator[](uint32_t i) const { return coeffs_.at(i); }

    bool operator==(const Poly& other) const {
        if (params_.n != other.params_.n || params_.q != other.params_.q) return false;
        return coeffs_ == other.coeffs_;
    }

    Poly operator+(const Poly& other) const {
        CheckCompatible(other, "add");
        Poly out(params_);
        for (uint32_t i = 0; i < params_.n; i++) {
            uint64_t s = static_cast<uint64_t>(coeffs_[i]) + other.coeffs_[i];
            out.coeffs_[i] = static_cast<uint32_t>(s % params_.q);
        }
        return out;
    }

    Poly operator*(const Poly& other) const {
        CheckCompatible(other, "mul");
        Poly out(params_);
        for (uint32_t i = 0; i < params_.n; i++) {
            uint64_t p = static_cast<uint64_t>(coeffs_[i]) * other.coeffs_[i];
            out.coeffs_[i] = static_cast<uint32_t>(p % params_.q);
        }
        return out;
    }

private:
    PolyParams params_;
    std::vector<uint32_t> coeffs_;

    void CheckCompatible(const Poly& other, const char* op) const {
        if (params_.n != other.params_.n || params_.q != other.params_.q) {
            throw std::runtime_error(std::string("Poly: incompatible params for ") + op);
        }
    }
};
