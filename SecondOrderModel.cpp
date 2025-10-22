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
void SecondOrderModel::simulateSingleTrajectory(QLineSeries *series_noise,
                                                QLineSeries *series_clean)
{
    series_noise->clear();
    series_clean->clear();

    double x_noise = m_x0;
    double x_clean = m_x0;
    double v_noise = m_v0;
    double v_clean = m_v0;
    double t = 0.0;
    double h = m_dt;

    for (int i = 0; i < m_steps; ++i) {
        double xi_t = m_dist(m_gen);
        double noise = xi_t * sqrt(h);

        // С шумом
        double a_det_n = m_a - sin(x_noise) - m_gamma * v_noise;
        double a_total_n = a_det_n + noise;

        v_noise += h * a_total_n;
        x_noise += h * v_noise;

        // Без шума
        double a_det_c = m_a - sin(x_clean) - m_gamma * v_clean;

        v_clean += h * a_det_c;
        x_clean += h * v_clean;

        series_noise->append(t, x_noise);
        series_clean->append(t, x_clean);

        t += h;

        if (x_noise > M_PI + 2.0) break;
    }
}

void SecondOrderModel::simulateTrajectoryHeun(QLineSeries *series_noise,
                                              QLineSeries *series_clean)
{
    series_noise->clear();
    series_clean->clear();

    double x_noise = m_x0;
    double v_noise = m_v0;
    double x_clean = m_x0;
    double v_clean = m_v0;

    double t = 0.0;
    double h = m_dt;
    double g = 0.5;  // амплитуда шума

    for (int i = 0; i < m_steps; ++i) {
        double xi = m_dist(m_gen);
        double Z_h = xi * sqrt(h);

        // --- С ШУМОМ ---
        double f1 = m_a - sin(x_noise) - m_gamma * v_noise;
        double v_pred = v_noise + h * f1 + g * Z_h;
        double x_pred = x_noise + h * v_noise;
        double f2 = m_a - sin(x_pred) - m_gamma * v_pred;

        double v_new = v_noise + 0.5 * h * (f1 + f2) + g * Z_h;
        double x_new = x_noise + 0.5 * h * (v_noise + v_pred);

        series_noise->append(t, x_new);

        x_noise = x_new;
        v_noise = v_new;

        // --- БЕЗ ШУМА ---
        double f1c = m_a - sin(x_clean) - m_gamma * v_clean;
        double v_pred_c = v_clean + h * f1c;
        double x_pred_c = x_clean + h * v_clean;
        double f2c = m_a - sin(x_pred_c) - m_gamma * v_pred_c;

        double v_new_c = v_clean + 0.5 * h * (f1c + f2c);
        double x_new_c = x_clean + 0.5 * h * (v_clean + v_pred_c);

        series_clean->append(t, x_new_c);

        x_clean = x_new_c;
        v_clean = v_new_c;

        t += h;
    }
}

