#pragma once

#include <QObject>
#include <QtCharts/QLineSeries>

using namespace QtCharts;

namespace Heun
{

class HeunModel : public QObject
{
    Q_OBJECT

public:
    HeunModel(double a, int steps, QObject *parent = nullptr);

    double dxdt(double x, double a);

    void method(QLineSeries *series_x, QLineSeries *series_dxdt);

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
};

}
