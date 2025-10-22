#include "SecondOrderWidget.hpp"
#include <QVBoxLayout>
#include <QMessageBox>

SecondOrderWidget::SecondOrderWidget(SecondOrderModel *model, QWidget *parent)
    : QWidget{parent}, m_model(model)
{
    m_aSpinBox = new QDoubleSpinBox();
    m_aSpinBox->setRange(-10.0, 10.0);
    m_aSpinBox->setSingleStep(0.1);
    m_aSpinBox->setPrefix("a = ");
    m_aSpinBox->setValue(0.5);

    m_gammaSpinBox = new QDoubleSpinBox();
    m_gammaSpinBox->setRange(0.01, 10.0);
    m_gammaSpinBox->setSingleStep(0.1);
    m_gammaSpinBox->setPrefix("γ = ");
    m_gammaSpinBox->setValue(0.5);

    m_timeSpinBox = new QSpinBox();
    m_timeSpinBox->setRange(1, 1000);
    m_timeSpinBox->setPrefix("Время = ");
    m_timeSpinBox->setValue(1000);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(m_aSpinBox);
    layout->addWidget(m_gammaSpinBox);
    layout->addWidget(m_timeSpinBox);
    m_useHeunCheckBox = new QCheckBox("Использовать Хьюна (вместо Эйлера)");
    m_useHeunCheckBox->setChecked(false);  // по умолчанию Эйлер

    layout->addWidget(m_useHeunCheckBox);

    m_series = new QLineSeries();
    m_chart = new QChart();
    m_chart->addSeries(m_series);
    m_chart->setTitle("x(t) — траектория с шумом");

    QValueAxis *axisX = new QValueAxis();
    axisX->setTitleText("t");
    axisX->setLabelFormat("%.2f");
    m_chart->addAxis(axisX, Qt::AlignBottom);
    m_series->attachAxis(axisX);

    QValueAxis *axisY = new QValueAxis();
    axisY->setTitleText("x(t)");
    axisY->setLabelFormat("%.2f");
    m_chart->addAxis(axisY, Qt::AlignLeft);
    m_series->attachAxis(axisY);

    m_seriesClean = new QLineSeries();
    m_seriesClean->setName("Без шума");
    m_series->setName("С шумом");

    QPen cleanPen(Qt::blue);
    cleanPen.setStyle(Qt::DashLine);
    m_seriesClean->setPen(cleanPen);

    m_chart->addSeries(m_seriesClean);
    m_seriesClean->attachAxis(axisX);
    m_seriesClean->attachAxis(axisY);

    m_chartView = new QChartView(m_chart);
    m_chartView->setRenderHint(QPainter::Antialiasing);

    m_runButton = new QPushButton("Запустить моделирование");
    connect(m_runButton, &QPushButton::clicked, this, &SecondOrderWidget::runSimulation);

    m_resultLabel = new QLabel("Задержка включения: -");
    m_resultLabel->setAlignment(Qt::AlignCenter);
    m_resultLabel->setStyleSheet("font-weight: bold; font-size: 14px;");
    layout->addWidget(m_chartView);
    layout->addWidget(m_runButton);
    layout->addWidget(m_resultLabel);

    setLayout(layout);
}

void SecondOrderWidget::runSimulation()
{
    double a = m_aSpinBox->value();
    double gamma = m_gammaSpinBox->value();
    int maxTime = m_timeSpinBox->value();

    m_model->setA(a);
    m_model->setGamma(gamma);
    double dt = static_cast<double>(maxTime) / 1000;
    m_model->setDt(dt);

    if (m_useHeunCheckBox->isChecked())
    {
        m_model->simulateTrajectoryHeun(m_series, m_seriesClean);
    }
    else
    {
        m_model->simulateSingleTrajectory(m_series, m_seriesClean);
    }


    auto points = m_series->pointsVector();
    if (!points.empty()) {
        double minY = points[0].y(), maxY = points[0].y();
        for (const QPointF &p : points)
        {
            if (p.y() < minY) minY = p.y();
            if (p.y() > maxY) maxY = p.y();
        }
        m_chart->axes(Qt::Horizontal).first()->setRange(0, points.last().x());
        m_chart->axes(Qt::Vertical).first()->setRange(minY, maxY);
    }

    double threshold = M_PI;
    int trials = 100;
    double delay = m_model->computeSwitchDelay(threshold, trials);

    m_resultLabel->setText(QString("Средняя задержка: %1 (по %2 траекториям)").arg(delay, 0, 'f', 4).arg(trials));
}

