#include "HeunWidget.hpp"
#include "Heun/HeunModel.hpp"
#include <QtCharts/QChartView>
#include <QtCharts/QValueAxis>
#include <QtCharts/QLineSeries>
#include <QPushButton>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QVBoxLayout>
#include <QDebug>

using namespace QtCharts;

namespace Heun
{

HeunWidget::HeunWidget(HeunModel *model, QWidget *parent)
    : QWidget{parent}
    , m_heunModel(model)
    , m_chartX(new QChart())
    , m_chartDxdt(new QChart())
    , m_seriesX(new QLineSeries())
    , m_seriesDxdt(new QLineSeries())
{
    m_chartX->setTitle("График x(t)");
    m_chartDxdt->setTitle("График dx/dt");

    m_chartX->addSeries(m_seriesX);
    m_chartDxdt->addSeries(m_seriesDxdt);

    m_axisX_x = new QValueAxis();
    m_axisX_x->setTitleText("t");
    m_axisX_x->setLabelFormat("%.2f");
    m_chartX->addAxis(m_axisX_x, Qt::AlignBottom);
    m_seriesX->attachAxis(m_axisX_x);

    m_axisY_x = new QValueAxis();
    m_axisY_x->setTitleText("x(t)");
    m_axisY_x->setLabelFormat("%.2f");
    m_chartX->addAxis(m_axisY_x, Qt::AlignLeft);
    m_seriesX->attachAxis(m_axisY_x);

    m_axisX_dxdt = new QValueAxis();
    m_axisX_dxdt->setTitleText("t");
    m_axisX_dxdt->setLabelFormat("%.2f");
    m_chartDxdt->addAxis(m_axisX_dxdt, Qt::AlignBottom);
    m_seriesDxdt->attachAxis(m_axisX_dxdt);

    m_axisY_dxdt = new QValueAxis();
    m_axisY_dxdt->setTitleText("dx/dt");
    m_axisY_dxdt->setLabelFormat("%.2f");
    m_chartDxdt->addAxis(m_axisY_dxdt, Qt::AlignLeft);
    m_seriesDxdt->attachAxis(m_axisY_dxdt);

    m_chartViewX = new QChartView(m_chartX);
    m_chartViewX->setRenderHint(QPainter::Antialiasing);

    m_chartViewDxdt = new QChartView(m_chartDxdt);
    m_chartViewDxdt->setRenderHint(QPainter::Antialiasing);

    m_aSpinBox = new QDoubleSpinBox();
    m_aSpinBox->setRange(-10.0, 10.0);
    m_aSpinBox->setSingleStep(0.1);
    m_aSpinBox->setPrefix("a = ");

    m_timeSpinBox = new QSpinBox();
    m_timeSpinBox->setRange(1, 1000);
    m_timeSpinBox->setPrefix("Время = ");

    m_updateButton = new QPushButton("Обновить графики");

    connect(m_updateButton, &QPushButton::clicked, this, &HeunWidget::updateChart);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(m_chartViewX);
    mainLayout->addWidget(m_chartViewDxdt);
    mainLayout->addWidget(m_aSpinBox);
    mainLayout->addWidget(m_timeSpinBox);
    mainLayout->addWidget(m_updateButton);

    setLayout(mainLayout);
}

QLineSeries *Heun::HeunWidget::HeunWidget::getSeriesX() const
{
    return m_seriesX;
}

void HeunWidget::updateChart() {
    double a = m_aSpinBox->value();
    int maxTime = m_timeSpinBox->value();

    if (maxTime <= 0) {
        maxTime = 1;
        m_timeSpinBox->setValue(maxTime);
    }

    m_heunModel->setA(a);
    const int maxPoints = 1000;
    double dt = static_cast<double>(maxTime) / maxPoints;
    m_heunModel->setDt(dt);

    m_seriesX->clear();
    m_seriesDxdt->clear();

    m_heunModel->method(m_seriesX, m_seriesDxdt);

    m_axisX_x->setRange(0, maxTime);
    m_axisX_dxdt->setRange(0, maxTime);

    auto pointsX = m_seriesX->pointsVector();
    auto pointsDxdt = m_seriesDxdt->pointsVector();

    if (!pointsX.empty()) {
        auto [minX, maxX] = std::minmax_element(pointsX.begin(), pointsX.end(),
                                                [](const QPointF &a, const QPointF &b) { return a.y() < b.y(); });
        m_axisY_x->setRange(minX->y(), maxX->y());
    }

    if (!pointsDxdt.empty()) {
        auto [minDxdt, maxDxdt] = std::minmax_element(pointsDxdt.begin(), pointsDxdt.end(),
                                                      [](const QPointF &a, const QPointF &b) { return a.y() < b.y(); });
        m_axisY_dxdt->setRange(minDxdt->y(), maxDxdt->y());
    }
}

}
