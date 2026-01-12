#include <cstdint>
#include <dpu>
#include <iostream>
#include <vector>

#include "header.h"
#include "timer.h"
#include "log.h"

class Polynomial {
public:
    Polynomial(int size = POLY_SIZE) {
        poly_.resize(size);
        for (int i = 0; i < size; i++) {
            poly_[i] = rand() % Q;
        }
    }

    Polynomial(const std::vector<uint32_t> &vec) {
        poly_ = vec;
    }

    int size() const {
        return poly_.size();
    }

    uint32_t operator[](int i) const {
        return poly_.at(i);
    }

    bool operator==(const Polynomial &other) const {
        int size = poly_.size();
        if (other.size() != size) {
            std::cerr << "[WARN] Compare with different size" << std::endl;
            return false;
        }
        for (int i = 0; i < size; i++) {
            if (poly_[i] != other[i]) {
                return false;
            }
        }
        return true;
    }

    Polynomial operator+(const Polynomial &other) const {
        int size = poly_.size();
        if (other.size() != size) {
            std::cerr << "[WARN] Add with different size" << std::endl;
            return *this;
        }
        std::vector<uint32_t> vec(size);
        for (int i = 0; i < size; i++) {
            vec[i] = (poly_[i] + other[i]) % Q;
        }
        return Polynomial(vec);
    }

    Polynomial operator*(const Polynomial &other) const {
        int size = poly_.size();
        if (other.size() != size) {
            std::cerr << "[WARN] Multiplicate with different size" << std::endl;
            return *this;
        }
        std::vector<uint32_t> vec(size);
        for (int i = 0; i < size; i++) {
            vec[i] = (poly_[i] * other[i]) % Q;
        }
        return Polynomial(vec);
    }

    std::vector<uint32_t> poly_;
};

void poly_addtion();
void poly_multiplication();

int main() {
    poly_multiplication();
}

void poly_addtion() {
    TimingReport tr;

    auto system = dpu::DpuSet::allocate(NR_DPUS);

    Polynomial p1, p2;
    Polynomial p_theoretical;
    {
        ScopedTimer t(tr, "cpu_compute_addition");
        p_theoretical = p1 + p2;
    }

    system.load(_STR(DPU_TARGET));
    
    {
        ScopedTimer t(tr, "host_to_dpu_copy");
        system.copy(_STR(POLYNOMIAL(1)), p1.poly_);
        system.copy(_STR(POLYNOMIAL(2)), p2.poly_);
    }

    {
        ScopedTimer t(tr, "dpu_exec");
        system.exec();
    }

    std::vector<std::vector<uint32_t>> res(system.dpus().size(), std::vector<uint32_t>(POLY_SIZE));
    {
        ScopedTimer t(tr, "dpu_to_host_copy");
        system.copy(res, _STR(POLYNOMIAL(0)));
    }
    Polynomial p_dpu(res[0]);

    auto& log_file = Log::logfile();
    system.log(log_file);

    if (p_theoretical == p_dpu) {
        std::cout << "[OK] Equal" << std::endl;
    } else {
        std::cout << "[ERROR] Not same" << std::endl;
    }

    tr.print(log_file);
}

void poly_multiplication() {
    TimingReport tr;

    auto system = dpu::DpuSet::allocate(NR_DPUS);

    const uint32_t nr_dpus = (uint32_t)system.dpus().size();
    constexpr uint32_t poly_size = POLY_SIZE;
    const uint32_t chunk_size = (poly_size + nr_dpus - 1) / nr_dpus;

    Polynomial p1, p2;
    Polynomial p_theoretical;
    {
        ScopedTimer t(tr, "cpu_compute_multiplication");
        p_theoretical = p1 * p2;
    }

    system.load(_STR(DPU_TARGET));

    std::vector<std::vector<uint32_t>> in1(nr_dpus, std::vector<uint32_t>(chunk_size, 0));
    std::vector<std::vector<uint32_t>> in2(nr_dpus, std::vector<uint32_t>(chunk_size, 0));
    std::vector<dpu_args_t> args(nr_dpus);

    for (uint32_t d = 0; d < nr_dpus; d++) {
        uint32_t offset = d * chunk_size;
        uint32_t len = 0;
        if (offset < poly_size) len = std::min(chunk_size, poly_size - offset);
        args[d].length = len;

        for (uint32_t j = 0; j < len; j++) {
            in1[d][j] = p1.poly_[offset + j];
            in2[d][j] = p2.poly_[offset + j];
        }
    }

    {
        ScopedTimer t(tr, "host_to_dpu_copy");
        system.copy(_STR(POLYNOMIAL(1)), in1);
        system.copy(_STR(POLYNOMIAL(2)), in2);
        system.copy("DPU_INPUT_ARGUMENTS", args);
    }

    // {
    //     ScopedTimer t(tr, "host_to_dpu_copy");
    //     system.copy(_STR(POLYNOMIAL(1)), p1.poly_);
    //     system.copy(_STR(POLYNOMIAL(2)), p2.poly_);
    // }

    {
        ScopedTimer t(tr, "dpu_exec");
        system.exec();
    }

    std::vector<std::vector<uint32_t>> out(nr_dpus, std::vector<uint32_t>(chunk_size, 0));
    {
        ScopedTimer t(tr, "dpu_to_host_copy");
        system.copy(out, _STR(POLYNOMIAL(0)));
    }

    // 汇总拼成完整 polynomial
    std::vector<uint32_t> merged(poly_size);
    for (uint32_t d = 0; d < nr_dpus; d++) {
        uint32_t offset = d * chunk_size;
        uint32_t len = args[d].length;
        for (uint32_t j = 0; j < len; j++) {
            merged[offset + j] = out[d][j];
        }
    }

    Polynomial p_dpu(merged);

    // std::vector<std::vector<uint32_t>> res(system.dpus().size(), std::vector<uint32_t>(POLY_SIZE));
    // {
    //     ScopedTimer t(tr, "dpu_to_host_copy");
    //     system.copy(res, _STR(POLYNOMIAL(0)));
    // }
    // Polynomial p_dpu(res[0]);

    auto& log_file = Log::logfile();
    system.log(log_file);

    if (p_theoretical == p_dpu) {
        std::cout << "[OK] Equal" << std::endl;
    } else {
        std::cout << "[ERROR] Not same" << std::endl;
    }

    tr.print(log_file);
}