#ifndef SWITCHINGANALYSIS_HPP
#define SWITCHINGANALYSIS_HPP

#include <vector>
#include <random>

class SwitchingAnalysis {
public:
    SwitchingAnalysis(double dt, int maxSteps);
    // Вычислить среднее время переключения для заданной интенсивности шума
    double meanSwitchingTime(double noiseIntensity, bool withSignal, int nRuns = 1000);
    // Получить массив MST для набора интенсивностей шума
    std::vector<double> mstVsNoise(const std::vector<double>& noiseList, bool withSignal, int nRuns = 1000);
private:
    double simulate(double noiseIntensity, bool withSignal);
    double dt_;
    int maxSteps_;
    std::mt19937 gen_;
};

#endif // SWITCHINGANALYSIS_HPP
