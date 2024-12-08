#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <cmath>

#include "HeunModel.hpp"

namespace Heun
{
Heun::HeunModel::HeunModel(double a, int steps, QObject *parent)
    : QObject{parent}, m_a(a), m_x0(0), m_t0(0), m_dt(0.01), m_steps(steps) {}

double Heun::HeunModel::dxdt(double x, double a)
{
    return a - sin(x);
}

void Heun::HeunModel::method(QLineSeries *series_x, QLineSeries *series_dxdt)
{
    double x = m_x0;
    double t = m_t0;
    series_x->clear();
    series_dxdt->clear();

    for (int i = 0; i < m_steps; ++i) {
        double dx_dt = dxdt(x, m_a);

        double x_predict = x + m_dt * dx_dt;
        double dx_dt_new = dxdt(x_predict, m_a);
        double x_next = x + (m_dt / 2.0) * (dx_dt + dx_dt_new);

        series_x->append(t, x);
        series_dxdt->append(t, dx_dt);

        x = x_next;
        t += m_dt;
    }
}

void Heun::HeunModel::setA(double a) {
    m_a = a;
}

void Heun::HeunModel::setDt(double dt) {
    m_dt = dt;
}

double Heun::HeunModel::getA() const {
    return m_a;
}

double Heun::HeunModel::getDt() const {
    return m_dt;
}

int Heun::HeunModel::getMaxSteps() const {
    return 1000;
}
}
