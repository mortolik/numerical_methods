#ifndef SECONDORDERMODEL_HPP
#define SECONDORDERMODEL_HPP

#include <QtCharts>
#include <QObject>
#include <random>
#include <vector>
#include <utility>

class SecondOrderModel : public QObject {
    Q_OBJECT

public:
    SecondOrderModel(double a, double gamma, int steps, QObject *parent = nullptr);
    void setDt(double dt);
    void setA(double a);
    void setGamma(double gamma);
    void setSignalAmp(double A);
    void setSignalFreq(double w);
    void setSteps(int steps);
    void setSeed(int seed);
    void setUseHeun(bool useHeun);

    void simulateTrajectory(QtCharts::QLineSeries *series_noise, QtCharts::QLineSeries *series_clean = nullptr);

    std::vector<std::pair<double, double>> computeMSTvsNoise(const std::vector<double>& noiseIntensities, double threshold, int trials, bool withSwitchingSignal = false, double switchingAmplitude = 0.0, double switchingFrequency = 1.0);

private:
    double m_a;
    double m_gamma;
    double m_signalAmp;
    double m_signalFreq;
    double m_x0;
    double m_v0;
    double m_dt;
    int m_steps;
    bool m_useHeun;
    std::mt19937 m_gen;
    std::normal_distribution<> m_dist;
};

#endif // SECONDORDERMODEL_HPP
