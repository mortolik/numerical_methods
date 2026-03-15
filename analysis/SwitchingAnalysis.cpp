#include "SwitchingAnalysis.hpp"
#include <cmath>

SwitchingAnalysis::SwitchingAnalysis(double dt, int maxSteps)
    : dt_(dt), maxSteps_(maxSteps), gen_(std::random_device{}()) {}

// Симуляция одной траектории, возвращает время переключения
// withSignal: true - с переключающим сигналом, false - без
// noiseIntensity: интенсивность шума (D)
double SwitchingAnalysis::simulate(double noiseIntensity, bool withSignal) {
    std::normal_distribution<double> norm(0.0, 1.0);
    double x = -1.0; // стартовое состояние (например, левый минимум)
    double t = 0.0;
    double threshold = 0.0; // граница переключения
    double signalAmp = withSignal ? 0.5 : 0.0; // амплитуда сигнала
    double signalFreq = 1.0; // частота сигнала
    for (int i = 0; i < maxSteps_; ++i) {
        double signal = withSignal ? signalAmp * std::sin(signalFreq * t) : 0.0;
        // Пример: двойная потенциальная яма с сигналом
        double force = -4 * x * (x * x - 1) + signal;
        double dx = force * dt_ + std::sqrt(2 * noiseIntensity * dt_) * norm(gen_);
        x += dx;
        t += dt_;
        if (x > threshold) {
            return t;
        }
    }
    return t; // если не переключилось, вернуть макс. время
}

double SwitchingAnalysis::meanSwitchingTime(double noiseIntensity, bool withSignal, int nRuns) {
    double sum = 0.0;
    for (int i = 0; i < nRuns; ++i) {
        sum += simulate(noiseIntensity, withSignal);
    }
    return sum / nRuns;
}

std::vector<double> SwitchingAnalysis::mstVsNoise(const std::vector<double>& noiseList, bool withSignal, int nRuns) {
    std::vector<double> result;
    for (double D : noiseList) {
        result.push_back(meanSwitchingTime(D, withSignal, nRuns));
    }
    return result;
}
