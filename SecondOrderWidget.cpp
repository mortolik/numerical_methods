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



    // --- Элементы управления для MST-эксперимента ---
    QHBoxLayout *mstRow1 = new QHBoxLayout();
    mstRow1->setAlignment(Qt::AlignLeft);
    m_dMinSpinBox = new QDoubleSpinBox();
    m_dMinSpinBox->setFixedWidth(120);
    m_dMinSpinBox->setRange(0.0, 10.0);
    m_dMinSpinBox->setSingleStep(0.01);
    m_dMinSpinBox->setPrefix("D min = ");
    m_dMinSpinBox->setValue(0.01);
    mstRow1->addWidget(m_dMinSpinBox);

    m_dMaxSpinBox = new QDoubleSpinBox();
    m_dMaxSpinBox->setFixedWidth(120);
    m_dMaxSpinBox->setRange(0.0, 10.0);
    m_dMaxSpinBox->setSingleStep(0.01);
    m_dMaxSpinBox->setPrefix("D max = ");
    m_dMaxSpinBox->setValue(0.5);
    mstRow1->addWidget(m_dMaxSpinBox);

    m_dStepSpinBox = new QDoubleSpinBox();
    m_dStepSpinBox->setFixedWidth(120);
    m_dStepSpinBox->setRange(0.001, 1.0);
    m_dStepSpinBox->setSingleStep(0.001);
    m_dStepSpinBox->setPrefix("D шаг = ");
    m_dStepSpinBox->setValue(0.02);
    mstRow1->addWidget(m_dStepSpinBox);

    m_thresholdSpinBox = new QDoubleSpinBox();
    m_thresholdSpinBox->setFixedWidth(120);
    m_thresholdSpinBox->setRange(0.0, 10.0);
    m_thresholdSpinBox->setSingleStep(0.01);
    m_thresholdSpinBox->setPrefix("Порог = ");
    m_thresholdSpinBox->setValue(M_PI);
    mstRow1->addWidget(m_thresholdSpinBox);

    m_trialsSpinBox = new QSpinBox();
    m_trialsSpinBox->setFixedWidth(120);
    m_trialsSpinBox->setRange(1, 10000);
    m_trialsSpinBox->setPrefix("Траекторий = ");
    m_trialsSpinBox->setValue(100);
    mstRow1->addWidget(m_trialsSpinBox);

    m_switchingSignalCheckBox = new QCheckBox("Переключающий сигнал");
    m_switchingSignalCheckBox->setFixedWidth(160);
    mstRow1->addWidget(m_switchingSignalCheckBox);
    layout->addLayout(mstRow1);

    QHBoxLayout *mstRow2 = new QHBoxLayout();
    mstRow2->setAlignment(Qt::AlignLeft);
    m_switchingAmplitudeSpinBox = new QDoubleSpinBox();
    m_switchingAmplitudeSpinBox->setFixedWidth(140);
    m_switchingAmplitudeSpinBox->setRange(0.0, 10.0);
    m_switchingAmplitudeSpinBox->setSingleStep(0.01);
    m_switchingAmplitudeSpinBox->setPrefix("Амплитуда = ");
    m_switchingAmplitudeSpinBox->setValue(0.0);
    mstRow2->addWidget(m_switchingAmplitudeSpinBox);

    m_switchingFrequencySpinBox = new QDoubleSpinBox();
    m_switchingFrequencySpinBox->setFixedWidth(140);
    m_switchingFrequencySpinBox->setRange(0.01, 10.0);
    m_switchingFrequencySpinBox->setSingleStep(0.01);
    m_switchingFrequencySpinBox->setPrefix("Частота = ");
    m_switchingFrequencySpinBox->setValue(1.0);
    mstRow2->addWidget(m_switchingFrequencySpinBox);
    layout->addLayout(mstRow2);

    m_mstVsNoiseButton = new QPushButton("Построить MST vs шум");
    m_mstVsNoiseButton->setFixedWidth(180);
    m_runButton->setFixedWidth(180);
    connect(m_mstVsNoiseButton, &QPushButton::clicked, this, &SecondOrderWidget::runMSTvsNoiseExperiment);
    layout->addWidget(m_mstVsNoiseButton);

    m_mstSeries = new QLineSeries();
    m_mstSeries->setName("MST vs D");
    m_mstChart = new QChart();
    m_mstChart->addSeries(m_mstSeries);
    m_mstChart->setTitle("Среднее время переключения vs интенсивность шума");
    QValueAxis *mstAxisX = new QValueAxis();
    mstAxisX->setTitleText("Интенсивность шума D");
    mstAxisX->setLabelFormat("%.3f");
    m_mstChart->addAxis(mstAxisX, Qt::AlignBottom);
    m_mstSeries->attachAxis(mstAxisX);
    QValueAxis *mstAxisY = new QValueAxis();
    mstAxisY->setTitleText("Среднее время переключения");
    mstAxisY->setLabelFormat("%.2f");
    m_mstChart->addAxis(mstAxisY, Qt::AlignLeft);
    m_mstSeries->attachAxis(mstAxisY);
    m_mstChartView = new QChartView(m_mstChart);
    m_mstChartView->setRenderHint(QPainter::Antialiasing);
    layout->addWidget(m_mstChartView);

    setLayout(layout);
}
void SecondOrderWidget::runMSTvsNoiseExperiment()
{
    // Диапазон интенсивностей шума из UI
    std::vector<double> noiseIntensities;
    double dMin = m_dMinSpinBox->value();
    double dMax = m_dMaxSpinBox->value();
    double dStep = m_dStepSpinBox->value();
    for (double D = dMin; D <= dMax + 1e-8; D += dStep) noiseIntensities.push_back(D);
    double threshold = m_thresholdSpinBox->value();
    int trials = m_trialsSpinBox->value();
    bool withSwitching = m_switchingSignalCheckBox->isChecked();
    double switchingAmplitude = m_switchingAmplitudeSpinBox->value();
    double switchingFrequency = m_switchingFrequencySpinBox->value();
    auto results = m_model->computeMSTvsNoise(noiseIntensities, threshold, trials, withSwitching, switchingAmplitude, switchingFrequency);
    m_mstSeries->clear();
    for (const auto& pair : results) {
        m_mstSeries->append(pair.first, pair.second);
    }
    // Автоматически подобрать оси
    if (!results.empty()) {
        double minX = results.front().first, maxX = results.back().first;
        double minY = results.front().second, maxY = results.front().second;
        for (const auto& pair : results) {
            if (pair.second > 0) {
                if (pair.second < minY) minY = pair.second;
                if (pair.second > maxY) maxY = pair.second;
            }
        }
        m_mstChart->axes(Qt::Horizontal).first()->setRange(minX, maxX);
        m_mstChart->axes(Qt::Vertical).first()->setRange(minY, maxY);
    }
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

