#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <cmath>
#include <random>

#include "HeunModel.hpp"

namespace Heun
{
Heun::HeunModel::HeunModel(double a, int steps, QObject *parent)
    : QObject{parent}, m_a(a), m_x0(0), m_t0(0), m_dt(0.001), m_steps(steps),
    m_gen(std::random_device{}()), m_dist(0.0, 1.0) {}

double Heun::HeunModel::dxdt(double x, double a)
{
    return a - sin(x);
}

void Heun::HeunModel::method(QLineSeries *series_x, QLineSeries *series_dxdt,
                             QLineSeries *series_clean_x, QLineSeries *series_clean_dxdt) {
    double x = m_x0;  // Начальное значение x
    double t = m_t0;  // Начальное время
    double h = m_dt;  // Шаг интегрирования
    double g = 0.5;   // Интенсивность шума

    series_x->clear();
    series_dxdt->clear();
    series_clean_x->clear();
    series_clean_dxdt->clear();

    for (int i = 0; i < m_steps; ++i) {
        // Генерация шума
        double xi_t = m_dist(m_gen);  // Гауссовский шум с нулевым средним и дисперсией 1
        double Z_h = xi_t * sqrt(h);  // Учет стохастической природы

        // Детерминированная часть
        double f_x = dxdt(x, m_a);  // f(x) = a - sin(x)

        // Предиктор (с учетом шума)
        double x_predict = x + h * f_x + g * Z_h;

        // Корректор (без шума)
        double f_x_predict = dxdt(x_predict, m_a);
        double x_next = x + (h / 2.0) * (f_x + f_x_predict) + g * Z_h;

        // Сохранение данных
        series_clean_x->append(t, x);             // Чистый x(t)
        series_clean_dxdt->append(t, f_x);        // Чистый dx/dt
        series_x->append(t, x_next);              // x(t) с шумом
        series_dxdt->append(t, f_x + g * xi_t);   // dx/dt с шумом

        // Обновление состояния
        x = x_next;
        t += h;
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
