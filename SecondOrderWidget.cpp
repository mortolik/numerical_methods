#include "SecondOrderWidget.hpp"
#include <QVBoxLayout>
#include <QMessageBox>
#include <QProgressDialog>
#include <QApplication>
#include <QtConcurrent>
#include <QFuture>

SecondOrderWidget::SecondOrderWidget(SecondOrderModel *model, QWidget *parent)
    : QWidget{parent}, m_model(model)
{
    m_aSpinBox = new QDoubleSpinBox();
    m_aSpinBox->setRange(-10.0, 10.0);
    m_aSpinBox->setSingleStep(0.1);
    m_aSpinBox->setPrefix("a = ");
    m_aSpinBox->setValue(0.5); // 1-е приближение из статьи Гордеевой и Панкратова (2008)

    m_gammaSpinBox = new QDoubleSpinBox();
    m_gammaSpinBox->setRange(0.01, 10.0);
    m_gammaSpinBox->setSingleStep(0.1);
    m_gammaSpinBox->setPrefix("γ = ");
    m_gammaSpinBox->setValue(1.0); // Затухание

    m_timeSpinBox = new QSpinBox();
    m_timeSpinBox->setRange(1, 10000);
    m_timeSpinBox->setPrefix("Время = ");
    m_timeSpinBox->setValue(300); // 300 секунд достаточно для большинства переходов, 1000 слишком долго для симуляции 1000 траекторий

    // --- Новый layout: параметры слева, графики справа ---
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    QVBoxLayout *paramsLayout = new QVBoxLayout();
    QVBoxLayout *chartsLayout = new QVBoxLayout();
    // Настроить внешние отступы
    mainLayout->setContentsMargins(10, 10, 10, 10);
    paramsLayout->setContentsMargins(0, 0, 0, 0);
    chartsLayout->setContentsMargins(0, 0, 0, 0);

    // Параметры моделирования
    // --- Одинаковая ширина для всех спинбоксов ---
    int spinBoxWidth = 180;
    m_aSpinBox->setFixedWidth(spinBoxWidth);
    m_gammaSpinBox->setFixedWidth(spinBoxWidth);
    m_timeSpinBox->setFixedWidth(spinBoxWidth);

    // --- Элементы управления для MST-эксперимента ---
    m_dMinSpinBox = new QDoubleSpinBox();
    m_dMinSpinBox->setFixedWidth(spinBoxWidth);
    m_dMinSpinBox->setRange(0.0001, 10.0);
    m_dMinSpinBox->setDecimals(4);
    m_dMinSpinBox->setSingleStep(0.001);
    m_dMinSpinBox->setPrefix("D min = ");
    m_dMinSpinBox->setValue(0.001);

    m_dMaxSpinBox = new QDoubleSpinBox();
    m_dMaxSpinBox->setFixedWidth(spinBoxWidth);
    m_dMaxSpinBox->setRange(0.01, 10.0);
    m_dMaxSpinBox->setSingleStep(0.1);
    m_dMaxSpinBox->setPrefix("D max = ");
    m_dMaxSpinBox->setValue(1.0);

    m_dStepSpinBox = new QDoubleSpinBox();
    m_dStepSpinBox->setFixedWidth(spinBoxWidth);
    m_dStepSpinBox->setRange(0.001, 1.0);
    m_dStepSpinBox->setSingleStep(0.001);
    m_dStepSpinBox->setPrefix("D шаг = ");
    m_dStepSpinBox->setValue(0.02);

    m_thresholdSpinBox = new QDoubleSpinBox();
    m_thresholdSpinBox->setFixedWidth(spinBoxWidth);
    m_thresholdSpinBox->setRange(0.0, 10.0);
    m_thresholdSpinBox->setSingleStep(0.01);
    m_thresholdSpinBox->setPrefix("Порог = ");
    m_thresholdSpinBox->setValue(M_PI);

    m_trialsSpinBox = new QSpinBox();
    m_trialsSpinBox->setFixedWidth(spinBoxWidth);
    m_trialsSpinBox->setRange(1, 100000);
    m_trialsSpinBox->setPrefix("Траекторий = ");
    m_trialsSpinBox->setValue(1000); // 1000 trajectories for better smoothing

    m_switchingSignalCheckBox = new QCheckBox("Переключающий сигнал");
    m_switchingSignalCheckBox->setFixedWidth(spinBoxWidth);
    m_switchingSignalCheckBox->setChecked(true); // Запускаем с переключением по умолчанию

    m_switchingAmplitudeSpinBox = new QDoubleSpinBox();
    m_switchingAmplitudeSpinBox->setFixedWidth(spinBoxWidth);
    m_switchingAmplitudeSpinBox->setRange(0.0, 10.0);
    m_switchingAmplitudeSpinBox->setSingleStep(0.01);
    m_switchingAmplitudeSpinBox->setPrefix("Амплитуда = ");
    m_switchingAmplitudeSpinBox->setValue(1.0); // A=1.0 из статьи (или A=0.7)

    m_switchingFrequencySpinBox = new QDoubleSpinBox();
    m_switchingFrequencySpinBox->setFixedWidth(spinBoxWidth);
    m_switchingFrequencySpinBox->setRange(0.01, 10.0);
    m_switchingFrequencySpinBox->setSingleStep(0.01);
    m_switchingFrequencySpinBox->setPrefix("Частота = ");
    m_switchingFrequencySpinBox->setValue(0.4); // ω=0.4 - там, где наиболее выражен эффект NES

    // --- Вертикальное размещение всех элементов ---
    m_useHeunCheckBox = new QCheckBox("Использовать Хьюна (вместо Эйлера)");
    m_useHeunCheckBox->setChecked(true); // Хьюн по умолчанию

    // --- Seed для генератора случайных чисел ---
    m_seedSpinBox = new QSpinBox();
    m_seedSpinBox->setFixedWidth(spinBoxWidth);
    m_seedSpinBox->setRange(0, 1000000);
    m_seedSpinBox->setPrefix("Seed = ");
    m_seedSpinBox->setValue(42);

    m_randomSeedCheckBox = new QCheckBox("Рандомный seed");
    m_randomSeedCheckBox->setFixedWidth(spinBoxWidth);
    m_randomSeedCheckBox->setChecked(false); // Без рандомного сида по умолчанию
    m_seedSpinBox->setEnabled(true);
    connect(m_randomSeedCheckBox, &QCheckBox::toggled, this, [this](bool checked){
        m_seedSpinBox->setEnabled(!checked);
    });

    paramsLayout->addWidget(m_aSpinBox);
    paramsLayout->addWidget(m_gammaSpinBox);
    paramsLayout->addWidget(m_timeSpinBox);
    paramsLayout->addWidget(m_useHeunCheckBox);
    paramsLayout->addWidget(m_randomSeedCheckBox);
    paramsLayout->addWidget(m_seedSpinBox);
    paramsLayout->addWidget(m_dMinSpinBox);
    paramsLayout->addWidget(m_dMaxSpinBox);
    paramsLayout->addWidget(m_dStepSpinBox);
    paramsLayout->addWidget(m_thresholdSpinBox);
    paramsLayout->addWidget(m_trialsSpinBox);
    paramsLayout->addWidget(m_switchingSignalCheckBox);
    paramsLayout->addWidget(m_switchingAmplitudeSpinBox);
    paramsLayout->addWidget(m_switchingFrequencySpinBox);
    // Фиксированная ширина левой панели
    QWidget *paramsWidget = new QWidget;
    paramsWidget->setLayout(paramsLayout);
    paramsWidget->setFixedWidth(220);

    // --- Кнопки зелёного цвета ---
    m_mstVsNoiseButton = new QPushButton("Построить MST vs шум");
    m_mstVsNoiseButton->setFixedWidth(180);
    m_mstVsNoiseButton->setStyleSheet("background-color: #4CAF50; color: white; font-weight: bold;");
    m_runButton = new QPushButton("Запустить моделирование");
    m_runButton->setFixedWidth(180);
    m_runButton->setStyleSheet("background-color: #4CAF50; color: white; font-weight: bold;");
    connect(m_runButton, &QPushButton::clicked, this, &SecondOrderWidget::runSimulation);
    connect(m_mstVsNoiseButton, &QPushButton::clicked, this, &SecondOrderWidget::runMSTvsNoiseExperiment);
    paramsLayout->addWidget(m_runButton);
    paramsLayout->addWidget(m_mstVsNoiseButton);

    m_resultLabel = new QLabel("Задержка включения: -");
    m_resultLabel->setAlignment(Qt::AlignCenter);
    m_resultLabel->setStyleSheet("font-weight: bold; font-size: 14px;");
    paramsLayout->addWidget(m_resultLabel);
    paramsLayout->addStretch();

    // --- Графики ---
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
    chartsLayout->addWidget(m_chartView);

    m_mstSeries = new QLineSeries();
    m_mstSeries->setName("MST vs D");
    m_mstChart = new QChart();
    m_mstChart->addSeries(m_mstSeries);
    m_mstChart->setTitle("Среднее время переключения vs интенсивность шума");
    QLogValueAxis *mstAxisX = new QLogValueAxis();
    mstAxisX->setBase(10.0);
    mstAxisX->setTitleText("Интенсивность шума D (log scale)");
    mstAxisX->setLabelFormat("%g");
    m_mstChart->addAxis(mstAxisX, Qt::AlignBottom);
    m_mstSeries->attachAxis(mstAxisX);
    QLogValueAxis *mstAxisY = new QLogValueAxis();
    mstAxisY->setBase(10.0);
    mstAxisY->setTitleText("Среднее время MST, c (log scale)");
    mstAxisY->setLabelFormat("%g");
    m_mstChart->addAxis(mstAxisY, Qt::AlignLeft);
    m_mstSeries->attachAxis(mstAxisY);
    m_mstChartView = new QChartView(m_mstChart);
    m_mstChartView->setRenderHint(QPainter::Antialiasing);
    chartsLayout->addWidget(m_mstChartView);

    mainLayout->addWidget(paramsWidget, 0);
    mainLayout->addLayout(chartsLayout, 2);
    setLayout(mainLayout);
}
void SecondOrderWidget::runMSTvsNoiseExperiment()
{
    // Считываем текущие параметры системы из UI
    double a = m_aSpinBox->value();
    double gamma = m_gammaSpinBox->value();
    int maxTime = m_timeSpinBox->value();
    double dt = 0.01;
    int steps = static_cast<int>(maxTime / dt);

    m_model->setA(a);
    m_model->setGamma(gamma);
    m_model->setDt(dt);
    m_model->setSteps(steps);
    m_model->setUseHeun(m_useHeunCheckBox->isChecked());

    // Диапазон интенсивностей шума из UI (Теперь логарифмический масштаб для точного повторения статьи)
    std::vector<double> noiseIntensities;
    double dMin = m_dMinSpinBox->value();
    double dMax = m_dMaxSpinBox->value();
    if (dMin <= 0) dMin = 1e-4; // Защита от <= 0 для логарифма

    // Генерируем точки в логарифмическом масштабе 
    // Вместо 50 точек (которое считалось очень долго), сделаем 20 точек.
    // На 10-ядерном процессоре M4 это займет ровно 2 вычислительных "волны", что существенно быстрее.
    int numPoints = 20; 
    double logDMin = std::log10(dMin);
    double logDMax = std::log10(dMax);
    for (int i = 0; i < numPoints; ++i) {
        double D = std::pow(10, logDMin + i * (logDMax - logDMin) / (numPoints - 1));
        noiseIntensities.push_back(D);
    }
    double threshold = m_thresholdSpinBox->value();
    int trials = 1000; // ЖЕСТКО ЗАДАЕМ 1000 ТРАЕКТОРИЙ КАК И ПРОСИЛИ
    m_trialsSpinBox->setValue(1000); 

    bool withSwitching = m_switchingSignalCheckBox->isChecked();
    double switchingAmplitude = m_switchingAmplitudeSpinBox->value();
    double switchingFrequency = m_switchingFrequencySpinBox->value();
    // Установить seed перед экспериментом
    if (m_randomSeedCheckBox->isChecked()) {
        m_model->setSeed(static_cast<int>(std::random_device{}()));
    } else {
        m_model->setSeed(m_seedSpinBox->value());
    }
    
    QProgressDialog progress("Вычисление траекторий... Пожалуйста, подождите.", "Отмена", 0, 100, this);
    progress.setWindowModality(Qt::WindowModal);
    progress.setMinimumDuration(0);
    progress.setValue(0);
    QApplication::processEvents();

    // Copy these into simple local variables so they can be captured by value in the lambda safely
    SecondOrderModel* model_ptr_safe = m_model;
    bool withSwitchingBool = withSwitching; 
    
    QFuture<std::vector<std::pair<double, double>>> future = QtConcurrent::run(
        [model_ptr_safe, noiseIntensities, threshold, trials, withSwitchingBool, switchingAmplitude, switchingFrequency]() {
            return model_ptr_safe->computeMSTvsNoise(noiseIntensities, threshold, trials, withSwitchingBool, switchingAmplitude, switchingFrequency);
        }
    );
    
    while (!future.isFinished()) {
        QApplication::processEvents(QEventLoop::AllEvents, 50);
        if (progress.wasCanceled()) {
            future.cancel(); // We can't strictly cancel std::sync inside immediately, but we can stop UI waiting
            return;
        }
        // Minimal visual spinning delay
        QThread::msleep(50);
    }
    progress.setValue(100);
    
    auto results = future.result();

    m_mstSeries->clear();
    for (const auto& pair : results) {
        double D = pair.first;
        double MST = pair.second > 0 ? pair.second : 1e-4;
        m_mstSeries->append(D, MST);
    }
    // Автоматически подобрать оси
    if (!results.empty()) {
        double minX = results.front().first;
        double maxX = results.back().first;
        
        double minY = 1e9, maxY = -1e9;
        bool foundPositive = false;
        
        for (const auto& pair : results) {
            if (pair.second > 0) {
                double valMST = pair.second;
                if (valMST < minY) minY = valMST;
                if (valMST > maxY) maxY = valMST;
                foundPositive = true;
            }
        }
        
        m_mstChart->axes(Qt::Horizontal).first()->setRange(minX, maxX);
        
        if (foundPositive) {
            // Если хотя бы одна точка посчиталась, масштабируем график
            double marginFactor = 1.1;
            m_mstChart->axes(Qt::Vertical).first()->setRange(std::max(1e-4, minY / marginFactor), maxY * marginFactor);
        } else {
            // Если все тесты не достигли порога, ставим стандартную заглушку
            m_mstChart->axes(Qt::Vertical).first()->setRange(1e-4, maxTime);
        }
    }
}

void SecondOrderWidget::runSimulation()
{
    double a = m_aSpinBox->value();
    double gamma = m_gammaSpinBox->value();
    double amp = m_switchingSignalCheckBox->isChecked() ? m_switchingAmplitudeSpinBox->value() : 0.0;
    double freq = m_switchingFrequencySpinBox->value();
    int maxTime = m_timeSpinBox->value();

    m_model->setA(a);
    m_model->setGamma(gamma);
    m_model->setSignalAmp(amp);
    m_model->setSignalFreq(freq);
    
    // Fixed time step (dt) for stability, calculate steps based on maxTime
    double dt = 0.01; 
    int steps = static_cast<int>(maxTime / dt);
    m_model->setDt(dt);
    m_model->setSteps(steps);
    m_model->setUseHeun(m_useHeunCheckBox->isChecked());

    // Установить seed перед одиночным запуском
    if (m_randomSeedCheckBox->isChecked()) {
        m_model->setSeed(static_cast<int>(std::random_device{}()));
    } else {
        m_model->setSeed(m_seedSpinBox->value());
    }

    m_model->simulateTrajectory(m_series, m_seriesClean);

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
    int trials = m_trialsSpinBox->value();
    
    std::vector<double> currentNoise = {1.0}; // Default base noise for visual
    auto res = m_model->computeMSTvsNoise(currentNoise, threshold, trials, m_switchingSignalCheckBox->isChecked(), amp, freq);
    double delay = res.empty() ? 0.0 : res[0].second;

    m_resultLabel->setText(QString("Средняя задержка: %1\n(по %2 траекториям)").arg(delay, 0, 'f', 4).arg(trials));
}

