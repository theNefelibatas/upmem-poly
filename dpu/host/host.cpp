#include <dpu>
#include <iostream>

int main() {
    auto system = dpu::DpuSet::allocate(1);
    auto dpu = system.dpus()[0];

    dpu->load("build/dpu");
    dpu->exec();
    dpu->log(std::cout);
}