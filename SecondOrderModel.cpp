#include "SecondOrderModel.hpp"
#include <cmath>

// Новый метод для серии экспериментов по MST
std::vector<std::pair<double, double>> SecondOrderModel::computeMSTvsNoise(const std::vector<double>& noiseIntensities, double threshold, int trials, bool withSwitchingSignal, double switchingAmplitude, double switchingFrequency) {
    std::vector<std::pair<double, double>> results;
    double oldStd = m_dist.stddev();
    for (double D : noiseIntensities) {
        // Установить новую интенсивность шума (stddev)
        m_dist = std::normal_distribution<>(0.0, sqrt(D));
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
                double switching = 0.0;
                if (withSwitchingSignal) {
                    switching = switchingAmplitude * sin(2 * M_PI * switchingFrequency * t);
                }
                double a_det = m_a - sin(x) - m_gamma * v + switching;
                v += h * a_det + xi_t * sqrt(h);
                x += h * v;
                t += h;
                if (x >= threshold) {
                    totalDelay += t;
                    ++count;
                    break;
                }
            }
        }
        double maxTime = m_steps * m_dt;
        double mst = (count > 0) ? totalDelay / count : maxTime;
        results.emplace_back(D, mst);
    }
    // Восстановить исходную дисперсию
    m_dist = std::normal_distribution<>(0.0, oldStd);
    return results;
}


SecondOrderModel::SecondOrderModel(double a, double gamma, int steps, QObject *parent)
    : QObject{parent}, m_a(a), m_gamma(gamma),
    m_signalAmp(0.5), m_signalFreq(1.0),
    m_x0(0.0), m_v0(0.0), m_dt(0.001), m_steps(steps),
    m_gen(std::random_device{}()), m_dist(0.0, 1.0) {}

void SecondOrderModel::setSeed(int seed) {
    m_gen.seed(seed);
}

void SecondOrderModel::setDt(double dt) {
    m_dt = dt;
}

void SecondOrderModel::setA(double a) {
    m_a = a;
}

void SecondOrderModel::setGamma(double gamma) {
    m_gamma = gamma;
}

void SecondOrderModel::setSignalAmp(double A) {
    m_signalAmp = A;
}

void SecondOrderModel::setSignalFreq(double w) {
    m_signalFreq = w;
}

void SecondOrderModel::setSteps(int steps) {
    m_steps = steps;
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

            double a_det = m_a + m_signalAmp * sin(m_signalFreq * t) - sin(x) - m_gamma * v;
            v += h * a_det + xi_t * sqrt(h);

            x += h * v;
            t += h;

            if (x >= threshold) {
                totalDelay += t;
                ++count;
                break;
            }
        }
    }

    double maxTime = m_steps * m_dt;
    return (count > 0) ? totalDelay / count : maxTime;
}
void SecondOrderModel::simulateSingleTrajectory(QtCharts::QLineSeries *series_noise,
                                                QtCharts::QLineSeries *series_clean)
{
    if (series_noise) series_noise->clear();
    if (series_clean) series_clean->clear();

    double x_noise = m_x0;
    double v_noise = m_v0;
    double x_clean = m_x0; // Clean trajectory state
    double v_clean = m_v0; 

    double t = 0.0;
    double h = m_dt;

    for (int i = 0; i < m_steps; ++i) {
        double xi_t = m_dist(m_gen);
        double noise = xi_t * sqrt(h);
        
        // Signal term
        double signal = m_signalAmp * sin(m_signalFreq * t);

        // --- With NOISE ---
        double a_det_n = m_a + signal - sin(x_noise) - m_gamma * v_noise;
        v_noise += h * a_det_n + xi_t * sqrt(h);

        x_noise += h * v_noise;
        
        if (series_noise) series_noise->append(t, x_noise);

        // --- Without NOISE (Clean) ---
        if (series_clean) {
            double a_det_c = m_a + signal - sin(x_clean) - m_gamma * v_clean;
            v_clean += h * a_det_c;
            x_clean += h * v_clean;
            series_clean->append(t, x_clean);
        }

        t += h;
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

