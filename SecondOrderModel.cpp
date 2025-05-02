#include "SecondOrderModel.hpp""
#include <cmath>

SecondOrderModel::SecondOrderModel(double a, double gamma, int steps, QObject *parent)
    : QObject{parent}, m_a(a), m_gamma(gamma),
    m_x0(0.0), m_v0(0.0), m_dt(0.001), m_steps(steps),
    m_gen(std::random_device{}()), m_dist(0.0, 1.0) {}

void SecondOrderModel::setDt(double dt) {
    m_dt = dt;
}

void SecondOrderModel::setA(double a) {
    m_a = a;
}

void SecondOrderModel::setGamma(double gamma) {
    m_gamma = gamma;
}

double SecondOrderModel::computeSwitchDelay(double threshold, int trials) {
    double totalDelay = 0.0;
    int count = 0;

    for (int trial = 0; trial < trials; ++trial) {
        double x = m_x0;
        double v = m_v0;
        double t = 0.0;
        double h = m_dt;

        for (int i = 0; i < m_steps; ++i) {
            double xi_t = m_dist(m_gen);
            double noise = xi_t * sqrt(h);

            double a_det = m_a - sin(x) - m_gamma * v;
            double a_total = a_det + noise;

            v += h * a_total;
            x += h * v;
            t += h;

            if (x >= threshold) {
                totalDelay += t;
                ++count;
                break;
            }
        }
    }

    return (count > 0) ? totalDelay / count : -1.0;
}
void SecondOrderModel::simulateSingleTrajectory(QtCharts::QLineSeries *series_x)
{
    series_x->clear();

    double x = m_x0;
    double v = m_v0;
    double t = 0.0;
    double h = m_dt;

    for (int i = 0; i < m_steps; ++i) {
        double xi_t = m_dist(m_gen);
        double noise = xi_t * sqrt(h);

        double a_det = m_a - sin(x) - m_gamma * v;
        double a_total = a_det + noise;

        v += h * a_total;
        x += h * v;
        t += h;

        series_x->append(t, x);

        if (x > M_PI + 2.0) break;  // для наглядности можно обрезать
    }
}
