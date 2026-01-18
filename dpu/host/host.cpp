#include <cstdint>
#include <dpu>
#include <iostream>
#include <vector>

#include "header.h"
#include "poly.hpp"
#include "timer.hpp"
#include "log.hpp"

void poly_op(const Poly& p1, const Poly& p2, op_t op, TimingReport& tr, Poly& p_theoretical);

int main() {
    TimingReport tr;

    auto system = dpu::DpuSet::allocate(NR_DPUS);

    const uint32_t nr_dpus = (uint32_t)system.dpus().size();
    constexpr uint32_t poly_size = POLY_SIZE;
    const uint32_t chunk_size = (poly_size + nr_dpus - 1) / nr_dpus;

    constexpr op_t op = MUL;

    PolyParams params(POLY_SIZE, Q, PolyFormat::COEFFICIENT);
    UniformGenerator dug(0xdeadbeefULL);

    Poly p1(params, dug);
    Poly p2(params, dug);
    Poly p_theoretical(params);
    poly_op(p1, p2, op, tr, p_theoretical);

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
            in1[d][j] = p1.data()[offset + j];
            in2[d][j] = p2.data()[offset + j];
        }
    }

    {
        ScopedTimer t(tr, "host_to_dpu_copy");
        system.copy(_STR(POLYNOMIAL(1)), in1);
        system.copy(_STR(POLYNOMIAL(2)), in2);
        uint32_t d = 0;
        for (auto &dpu : system.dpus()) {
            dpu->copy("DPU_INPUT_ARGUMENTS", std::vector<dpu_args_t>(1, args[d]));
            d++;
        }
        system.copy("OP_TYPE", std::vector<op_t>(1, op));
    }

    {
        ScopedTimer t(tr, "dpu_exec");
        system.exec();
    }

    std::vector<std::vector<uint32_t>> out(nr_dpus, std::vector<uint32_t>(chunk_size, 0));
    {
        ScopedTimer t(tr, "dpu_to_host_copy");
        system.copy(out, _STR(POLYNOMIAL(0)));
    }

    std::vector<uint32_t> merged(poly_size);
    for (uint32_t d = 0; d < nr_dpus; d++) {
        uint32_t offset = d * chunk_size;
        uint32_t len = args[d].length;
        for (uint32_t j = 0; j < len; j++) {
            merged[offset + j] = out[d][j];
        }
    }

    Poly p_dpu(params, merged);

    auto& log_file = Log::logfile();
    system.log(log_file);

    if (p_theoretical == p_dpu) {
        std::cout << "[OK] Equal" << std::endl;
    } else {
        std::cout << "[ERROR] Not same" << std::endl;
    }

    tr.print(log_file);
}

void poly_op(const Poly& p1, const Poly& p2, op_t op, TimingReport& tr, Poly& p_theoretical) {
    if (op == ADD) {
        ScopedTimer t(tr, "cpu_compute_addition");
        p_theoretical = p1 + p2;
    } if (op == MUL) {
        ScopedTimer t(tr, "cpu_compute_multiplication");
        p_theoretical = p1 * p2;
    }
    
}