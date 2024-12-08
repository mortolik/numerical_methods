#include "EulerWidget.hpp"
#include "Euler/EulerModel.hpp"
#include <QtCharts/QChartView>
#include <QtCharts/QValueAxis>
#include <QtCharts/QLineSeries>
#include <QPushButton>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QVBoxLayout>
#include <QDebug>

using namespace QtCharts;

namespace Euler {

EulerWidget::EulerWidget(EulerModel *model, QWidget *parent)
    : QWidget{parent}
    , m_eulerModel(model)
    , m_chartX(new QChart())
    , m_seriesEuler(new QLineSeries())
    , m_seriesHeun(new QLineSeries())
{
    // Настраиваем график
    m_chartX->setTitle("Сравнение методов: x(t)");

    m_seriesEuler->setName("Метод Эйлера");
    m_seriesEuler->setColor(Qt::blue);

    m_seriesHeun->setName("Метод Хьюна");
    m_seriesHeun->setColor(Qt::red);

    m_chartX->addSeries(m_seriesEuler);
    m_chartX->addSeries(m_seriesHeun);

    // Оси графика
    m_axisX = new QValueAxis();
    m_axisX->setTitleText("t");
    m_axisX->setLabelFormat("%.2f");
    m_chartX->addAxis(m_axisX, Qt::AlignBottom);
    m_seriesEuler->attachAxis(m_axisX);
    m_seriesHeun->attachAxis(m_axisX);

    m_axisY = new QValueAxis();
    m_axisY->setTitleText("x(t)");
    m_axisY->setLabelFormat("%.2f");
    m_chartX->addAxis(m_axisY, Qt::AlignLeft);
    m_seriesEuler->attachAxis(m_axisY);
    m_seriesHeun->attachAxis(m_axisY);

    // Отображение графика
    m_chartViewX = new QChartView(m_chartX);
    m_chartViewX->setRenderHint(QPainter::Antialiasing);

    // Элементы управления
    m_aSpinBox = new QDoubleSpinBox();
    m_aSpinBox->setRange(-10.0, 10.0);
    m_aSpinBox->setSingleStep(0.1);
    m_aSpinBox->setPrefix("a = ");

    m_timeSpinBox = new QSpinBox();
    m_timeSpinBox->setRange(1, 1000);
    m_timeSpinBox->setPrefix("Время = ");

    m_updateButton = new QPushButton("Обновить графики");

    connect(m_updateButton, &QPushButton::clicked, this, &EulerWidget::updateChart);

    // Макет
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(m_chartViewX);
    mainLayout->addWidget(m_aSpinBox);
    mainLayout->addWidget(m_timeSpinBox);
    mainLayout->addWidget(m_updateButton);

    setLayout(mainLayout);
}

void EulerWidget::updateChart() {
    double a = m_aSpinBox->value();
    int maxTime = m_timeSpinBox->value();

    if (maxTime <= 0) {
        maxTime = 1;
        m_timeSpinBox->setValue(maxTime);
    }

    m_eulerModel->setA(a);
    const int maxPoints = 1000;
    double dt = static_cast<double>(maxTime) / maxPoints;
    m_eulerModel->setDt(dt);

    m_seriesEuler->clear();
    m_seriesHeun->clear();

    // Вычисляем результаты для обоих методов
    m_eulerModel->eulerMethod(m_seriesEuler);
    m_eulerModel->heunMethod(m_seriesHeun);

    // Обновляем оси графика
    auto pointsEuler = m_seriesEuler->pointsVector();
    auto pointsHeun = m_seriesHeun->pointsVector();

    double minY = std::numeric_limits<double>::max();
    double maxY = std::numeric_limits<double>::lowest();

    for (const auto &point : pointsEuler) {
        minY = std::min(minY, point.y());
        maxY = std::max(maxY, point.y());
    }
    for (const auto &point : pointsHeun) {
        minY = std::min(minY, point.y());
        maxY = std::max(maxY, point.y());
    }

    m_axisX->setRange(0, maxTime);
    m_axisY->setRange(minY, maxY);
}

}
