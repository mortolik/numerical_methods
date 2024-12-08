#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <cmath>
#include <random>
#include "EulerModel.hpp"

using namespace QtCharts;

namespace Euler
{

EulerModel::EulerModel(double a, int steps, QObject *parent)
    : QObject{parent}, m_a(a), m_x0(0), m_t0(0), m_dt(0.01), m_steps(steps),
    m_gen(std::random_device{}()), m_dist(0.0, 1.0) {}


double EulerModel::dxdt(double x, double a) {
    return a - sin(x);
}
void EulerModel::method(QLineSeries *series_x, QLineSeries *series_dxdt,
                        QLineSeries *series_clean_x, QLineSeries *series_clean_dxdt) {
    double x = m_x0;
    double t = m_t0;
    series_x->clear();
    series_dxdt->clear();
    series_clean_x->clear();
    series_clean_dxdt->clear();

    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution<> dist(0.0, 1.0);

    for (int i = 0; i < m_steps; ++i)
    {
        double xi_t = dist(m_gen);
        double g = 1.0;

        double dx_dt_clean = dxdt(x, m_a);
        double dx_dt_noisy = dx_dt_clean + g * xi_t;

        series_clean_x->append(t, x);
        series_clean_dxdt->append(t, dx_dt_clean);
        series_x->append(t, x + g * xi_t);
        series_dxdt->append(t, dx_dt_noisy);

        x += m_dt * dx_dt_clean;
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
