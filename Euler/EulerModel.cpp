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
    : QObject{parent}, m_a(a), m_x0(0), m_t0(0), m_dt(0.001), m_steps(steps),
    m_gen(std::random_device{}()), m_dist(0.0, 1.0) {}

double EulerModel::dxdt(double x, double a)
{
    return a - sin(x);
}

void EulerModel::method(QLineSeries *series_x, QLineSeries *series_dxdt,
                        QLineSeries *series_clean_x, QLineSeries *series_clean_dxdt)
{
    double x = m_x0;  // Начальное значение x
    double t = m_t0;  // Начальное время
    double h = m_dt;  // Шаг интегрирования
    double end_time = m_t0 + m_steps * h;  // Конечное время

    series_x->clear();
    series_dxdt->clear();
    series_clean_x->clear();
    series_clean_dxdt->clear();

    while (t < end_time) {
        // Генерация шума
        double xi_t = m_dist(m_gen);  // Гауссовский шум с нулевым средним и дисперсией 1
        double Z_h = xi_t * sqrt(h);  // Учет стохастической природы

        // Детерминированная часть
        double f_x = dxdt(x, m_a);  // f(x) = a - sin(x)

        // Стохастическая часть
        double noise = 1.0 * Z_h;  // g = 1.0

        // Обновление x
        double x_new = x + h * f_x + noise;

        // Сохранение данных
        series_clean_x->append(t, x);
        series_clean_dxdt->append(t, f_x);
        series_x->append(t, x_new);
        series_dxdt->append(t, f_x + noise / h);

        // Обновление x и t
        x = x_new;
        t += h;
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
