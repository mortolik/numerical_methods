#include "SecondOrderModel.hpp"
#include <cmath>

SecondOrderModel::SecondOrderModel(double a, double gamma, int steps, QObject *parent)
    : QObject{parent}, m_a(a), m_gamma(gamma), m_signalAmp(0.5), m_signalFreq(1.0),
      m_x0(0.0), m_v0(0.0), m_dt(0.001), m_steps(steps), m_useHeun(false),
      m_gen(std::random_device{}()), m_dist(0.0, 1.0) {}

void SecondOrderModel::setDt(double dt) { m_dt = dt; }
void SecondOrderModel::setA(double a) { m_a = a; }
void SecondOrderModel::setGamma(double gamma) { m_gamma = gamma; }
void SecondOrderModel::setSignalAmp(double A) { m_signalAmp = A; }
void SecondOrderModel::setSignalFreq(double w) { m_signalFreq = w; }
void SecondOrderModel::setSteps(int steps) { m_steps = steps; }
void SecondOrderModel::setSeed(int seed) { m_gen.seed(seed); }
void SecondOrderModel::setUseHeun(bool useHeun) { m_useHeun = useHeun; }

void SecondOrderModel::simulateTrajectory(QtCharts::QLineSeries *series_noise, QtCharts::QLineSeries *series_clean) {
    if (series_noise) series_noise->clear();
    if (series_clean) series_clean->clear();

    double x = m_x0;
    double v = m_v0;
    double x_c = m_x0;
    double v_c = m_v0;
    double t = 0.0;
    double h = m_dt;

    for (int i = 0; i < m_steps; ++i) {
        double xi = m_dist(m_gen);
        double dW = xi * sqrt(h);
        double signal = m_signalAmp * sin(m_signalFreq * t);
        double signal_next = m_signalAmp * sin(m_signalFreq * (t + h));

        if (m_useHeun) {
            // Heun for noise
            double ax1 = m_a + signal - sin(x) - m_gamma * v;
            double v_pred = v + h * ax1 + dW;
            double x_pred = x + h * v;
            double ax2 = m_a + signal_next - sin(x_pred) - m_gamma * v_pred;
            double v_new = v + 0.5 * h * (ax1 + ax2) + dW;
            double x_new = x + 0.5 * h * (v + v_pred);
            x = x_new;
            v = v_new;

            // Heun for clean
            if (series_clean) {
                double ax1_c = m_a + signal - sin(x_c) - m_gamma * v_c;
                double v_pred_c = v_c + h * ax1_c;
                double x_pred_c = x_c + h * v_c;
                double ax2_c = m_a + signal_next - sin(x_pred_c) - m_gamma * v_pred_c;
                double v_new_c = v_c + 0.5 * h * (ax1_c + ax2_c);
                double x_new_c = x_c + 0.5 * h * (v_c + v_pred_c);
                x_c = x_new_c;
                v_c = v_new_c;
            }
        } else {
            // Euler for noise
            double ax = m_a + signal - sin(x) - m_gamma * v;
            v += h * ax + dW;
            x += h * v;

            // Euler for clean
            if (series_clean) {
                double ax_c = m_a + signal - sin(x_c) - m_gamma * v_c;
                v_c += h * ax_c;
                x_c += h * v_c;
            }
        }

        if (series_noise) series_noise->append(t, x);
        if (series_clean) series_clean->append(t, x_c);

        t += h;
    }
}

std::vector<std::pair<double, double>> SecondOrderModel::computeMSTvsNoise(const std::vector<double>& noiseIntensities, double threshold, int trials, bool withSwitchingSignal, double switchingAmplitude, double switchingFrequency) {
    std::vector<std::pair<double, double>> results;
    for (double D : noiseIntensities) {
        double totalDelay = 0.0;
        int count = 0;
        double stdev = sqrt(D);
        for (int trial = 0; trial < trials; ++trial) {
            double x = m_x0;
            double v = m_v0;
            double t = 0.0;
            double h = m_dt;
            for (int i = 0; i < m_steps; ++i) {
                double z = m_dist(m_gen);
                double dW = z * stdev * sqrt(h);
                double signal = 0.0;
                double signal_next = 0.0;
                if (withSwitchingSignal) {
                    signal = switchingAmplitude * sin(switchingFrequency * t);
                    signal_next = switchingAmplitude * sin(switchingFrequency * (t + h));
                }

                if (m_useHeun) {
                    double ax1 = m_a + signal - sin(x) - m_gamma * v;
                    double v_pred = v + h * ax1 + dW;
                    double x_pred = x + h * v;
                    double ax2 = m_a + signal_next - sin(x_pred) - m_gamma * v_pred;
                    double v_new = v + 0.5 * h * (ax1 + ax2) + dW;
                    double x_new = x + 0.5 * h * (v + v_pred);
                    x = x_new;
                    v = v_new;
                } else {
                    double ax = m_a + signal - sin(x) - m_gamma * v;
                    // Note: original euler had dW on standard equation, which already included stdev 
                    v += h * ax + dW;
                    x += h * v;
                }

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
    return results;
}
