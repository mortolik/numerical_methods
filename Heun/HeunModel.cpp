#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <cmath>
#include <random>

#include "HeunModel.hpp"

namespace Heun
{
Heun::HeunModel::HeunModel(double a, int steps, QObject *parent)
    : QObject{parent}, m_a(a), m_x0(0), m_t0(0), m_dt(0.01), m_steps(steps),
    m_gen(std::random_device{}()), m_dist(0.0, 1.0) {}

double Heun::HeunModel::dxdt(double x, double a)
{
    return a - sin(x);
}

void Heun::HeunModel::method(QLineSeries *series_x, QLineSeries *series_dxdt,
                             QLineSeries *series_clean_x, QLineSeries *series_clean_dxdt)
{
    double x = m_x0;
    double t = m_t0;
    series_x->clear();
    series_dxdt->clear();
    series_clean_x->clear();
    series_clean_dxdt->clear();

    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution<> dist(0.0, 0.02);

    for (int i = 0; i < m_steps; ++i) {
        double dx_dt = dxdt(x, m_a);

        double x_predict = x + m_dt * dx_dt;

        double dx_dt_new = dxdt(x_predict, m_a);
        double x_next = x + (m_dt / 2.0) * (dx_dt + dx_dt_new);

        double xi_t = dist(m_gen);
        double g = 1.0;

        series_clean_x->append(t, x);
        series_clean_dxdt->append(t, dx_dt);
        series_x->append(t, x + g * xi_t);
        series_dxdt->append(t, dx_dt + g * xi_t);

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
