#pragma once

#include <QObject>
#include <QtCharts/QLineSeries>
#include <random>

using namespace QtCharts;

namespace Heun
{

class HeunModel : public QObject
{
    Q_OBJECT

public:
    explicit HeunModel(double a, int steps, QObject *parent = nullptr);

    void method(QLineSeries *series_x, QLineSeries *series_dxdt,
                QLineSeries *series_clean_x, QLineSeries *series_clean_dxdt);

    void setA(double a);
    void setDt(double dt);
    double getA() const;
    double getDt() const;
    int getMaxSteps() const;

private:
    double m_a;
    double m_x0;
    double m_t0;
    double m_dt;
    int m_steps;    

    std::mt19937 m_gen;
    std::normal_distribution<> m_dist;

    double dxdt(double x, double a);
};

}
