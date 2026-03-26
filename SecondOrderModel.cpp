#include "SecondOrderModel.hpp"
#include <cmath>
#include <future>
#include <vector>

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

    QList<QPointF> pointsNoise;
    QList<QPointF> pointsClean;
    pointsNoise.reserve(m_steps);
    pointsClean.reserve(m_steps);

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

        pointsNoise.append(QPointF(t, x));
        if (series_clean) pointsClean.append(QPointF(t, x_c));

        t += h;
    }

    if (series_noise) series_noise->replace(pointsNoise);
    if (series_clean) series_clean->replace(pointsClean);
}

std::vector<std::pair<double, double>> SecondOrderModel::computeMSTvsNoise(const std::vector<double>& noiseIntensities, double threshold, int trials, bool withSwitchingSignal, double switchingAmplitude, double switchingFrequency) {
    std::vector<std::pair<double, double>> results;
    std::vector<std::future<std::pair<double, double>>> futures;

    for (size_t i = 0; i < noiseIntensities.size(); ++i) {
        double D = noiseIntensities[i];
        unsigned int localSeed = m_gen(); // Get thread-safe seed from main generator

        futures.push_back(std::async(std::launch::async, [this, D, threshold, trials, withSwitchingSignal, switchingAmplitude, switchingFrequency, localSeed]() -> std::pair<double, double> {
            std::mt19937 local_gen(localSeed);
            std::normal_distribution<> local_dist(0.0, 1.0);
            
            double totalDelay = 0.0;
            int count = 0;
            double stdev = sqrt(D);
            
            // Precompute deterministic signal if enabled to save billions of sin() calls
            std::vector<double> precomputed_signals(m_steps + 1, 0.0);
            if (withSwitchingSignal) {
                for (int step = 0; step <= m_steps; ++step) {
                    double t_step = step * m_dt;
                    precomputed_signals[step] = switchingAmplitude * sin(switchingFrequency * t_step);
                }
            }
            
            double sqrt_h = sqrt(m_dt);
            double noise_factor = stdev * sqrt_h;
            double half_h = 0.5 * m_dt;
            
            for (int trial = 0; trial < trials; ++trial) {
                double x = m_x0;
                double v = m_v0;
                double t = 0.0;
                double h = m_dt;
                for (int step = 0; step < m_steps; ++step) {
                    double z = local_dist(local_gen);
                    double dW = z * noise_factor;

                    if (m_useHeun) {
                        double ax1 = m_a + precomputed_signals[step] - sin(x) - m_gamma * v;
                        double v_pred = v + h * ax1 + dW;
                        double x_pred = x + h * v;
                        double ax2 = m_a + precomputed_signals[step + 1] - sin(x_pred) - m_gamma * v_pred;
                        
                        double v_old = v;
                        v += half_h * (ax1 + ax2) + dW;
                        x += half_h * (v_old + v_pred);
                    } else {
                        double ax = m_a + precomputed_signals[step] - sin(x) - m_gamma * v;
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
            return {D, mst};
        }));
    }

    for (auto& f : futures) {
        results.push_back(f.get());
    }

    return results;
}
