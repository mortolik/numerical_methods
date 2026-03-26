#include "SecondOrderModel.hpp"
#include <iostream>
#include <chrono>

int main() {
    SecondOrderModel model;
    model.setA(0.5);
    model.setGamma(1.0);
    model.setDt(0.01);
    model.setSteps(100000); // 100k steps!
    model.setUseHeun(true);
    model.setSeed(42);

    std::vector<double> noise;
    for(int i=0; i<50; ++i) noise.push_back(0.001 + i*0.02);

    auto start = std::chrono::high_resolution_clock::now();
    try {
        auto res = model.computeMSTvsNoise(noise, 3.14, 1000, true, 1.0, 0.4);
        auto end = std::chrono::high_resolution_clock::now();
        std::cout << "Computed " << res.size() << " elements.\n";
        std::cout << "Time: " << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << " ms\n";
    } catch(const std::exception& e) {
        std::cout << "Exception: " << e.what() << "\n";
    }

    return 0;
}
