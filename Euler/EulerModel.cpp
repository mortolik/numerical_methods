#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <cmath>
#include "EulerModel.hpp"

using namespace QtCharts;

namespace Euler
{

EulerModel::EulerModel(double a, int steps, QObject *parent)
    : QObject{parent}, m_a(a), m_x0(0), m_t0(0), m_dt(0.01), m_steps(steps) {}

double EulerModel::dxdt(double x, double a) {
    return a - sin(x);
}

void EulerModel::method(QLineSeries *series_x, QLineSeries *series_dxdt) {
    double x = m_x0;
    double t = m_t0;
    series_x->clear();
    series_dxdt->clear();

    for (int i = 0; i < m_steps; ++i) {
        double dx_dt = dxdt(x, m_a);
        series_x->append(t, x);
        series_dxdt->append(t, dx_dt);

        x += m_dt * dx_dt;
        t += m_dt;
    }
}

void EulerModel::setA(double a) {
    m_a = a;
}

void EulerModel::setDt(double dt) {
    m_dt = dt;
}

double EulerModel::getA() const {
    return m_a;
}
double EulerModel::getDt() const {
    return m_dt;
}

int EulerModel::getMaxSteps() const
{
    return 1000;
}

}
