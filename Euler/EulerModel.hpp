#pragma once
#include <QObject>
#include <QtCharts/QLineSeries>

namespace Euler
{

class EulerModel : public QObject {
    Q_OBJECT

public:
    explicit EulerModel(double a = 0.5, int steps = 1000, QObject *parent = nullptr);
    void method(QtCharts::QLineSeries *series_x, QtCharts::QLineSeries *series_dxdt);
    void setA(double a);
    void setDt(double dt);
    double getA() const;
    double getDt() const;
    void setSteps(int steps);

private:
    double dxdt(double x, double a);

    double m_a;
    double m_x0;
    double m_t0;
    double m_dt;
    int m_steps;
};

}
