#pragma once
#include <QtCharts>
#include <QObject>
#include <random>

class SecondOrderModel : public QObject {
    Q_OBJECT

public:
    SecondOrderModel(double a, double gamma, int steps, QObject *parent = nullptr);
    void setDt(double dt);
    void setA(double a);
    void setGamma(double gamma);
    double computeSwitchDelay(double threshold, int trials);
    void simulateSingleTrajectory(QtCharts::QLineSeries *series_x,
                                  QLineSeries *series_clean);
    void simulateTrajectoryHeun(QtCharts::QLineSeries *series_noise,
                                QtCharts::QLineSeries *series_clean);


private:
    double m_a;
    double m_gamma;
    double m_x0;
    double m_v0;
    double m_dt;
    int m_steps;
    std::mt19937 m_gen;
    std::normal_distribution<> m_dist;
};
