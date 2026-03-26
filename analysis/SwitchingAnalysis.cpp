#include "SwitchingAnalysis.hpp"
#include <cmath>
#include <future>
#include <vector>

SwitchingAnalysis::SwitchingAnalysis(double dt, int maxSteps)
    : dt_(dt), maxSteps_(maxSteps) {}

// Симуляция одной траектории, возвращает время переключения
// withSignal: true - с переключающим сигналом, false - без
// noiseIntensity: интенсивность шума (D)
double SwitchingAnalysis::simulate(double noiseIntensity, bool withSignal, std::mt19937& local_gen) {
    std::normal_distribution<double> norm(0.0, 1.0);
    double x = -1.0; // стартовое состояние (например, левый минимум)
    double t = 0.0;
    double threshold = 0.0; // граница переключения
    double signalAmp = 0.5; // амплитуда сигнала
    double signalFreq = 1.0; // частота сигнала
    
    // Предрасчет константы выносим за пределы цикла!
    double noiseCoeff = std::sqrt(2.0 * noiseIntensity * dt_);
    
    for (int i = 0; i < maxSteps_; ++i) {
        double signal = 0.0;
        if (withSignal) {
            signal = signalAmp * std::sin(signalFreq * t);
        }
        
        // Пример: двойная потенциальная яма с сигналом
        double force = -4 * x * (x * x - 1) + signal;
        
        // Вычисляем шаг
        double dx = force * dt_ + noiseCoeff * norm(local_gen);
        x += dx;
        t += dt_;
        
        if (x > threshold) {
            return t;
        }
    }
    return t; // если не переключилось, вернуть макс. время
}

double SwitchingAnalysis::meanSwitchingTime(double noiseIntensity, bool withSignal, int nRuns) {
    // Внутренний цикл теперь выполняется быстрее и без накладных расходов на потоки
    std::mt19937 local_gen(std::random_device{}());
    double sum = 0.0;
    for (int i = 0; i < nRuns; ++i) {
        sum += simulate(noiseIntensity, withSignal, local_gen);
    }
    return sum / nRuns;
}

std::vector<double> SwitchingAnalysis::mstVsNoise(const std::vector<double>& noiseList, bool withSignal, int nRuns) {
    std::vector<double> result(noiseList.size());
    std::vector<std::future<void>> futures;

    // Распараллеливаем по точкам на графике (интенсивностям шума). 
    // Это создает порядка 20 потоков, что идеально для процессора М4
    for (size_t i = 0; i < noiseList.size(); ++i) {
        futures.push_back(std::async(std::launch::async, [this, i, &noiseList, &result, withSignal, nRuns]() {
            result[i] = this->meanSwitchingTime(noiseList[i], withSignal, nRuns);
        }));
    }

    // Ожидаем завершения всех потоков-точек
    for (auto& fut : futures) {
        fut.get();
    }

    return result;
}
