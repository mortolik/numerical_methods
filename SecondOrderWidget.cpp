#include "SecondOrderWidget.hpp"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QLabel>
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
    m_aSpinBox->setValue(0.5); // 1-е приближение из статьи Гордеевой и Панкратова (2008)

    m_gammaLineEdit = new QLineEdit("1.0"); // Затухание

    m_timeSpinBox = new QSpinBox();
    m_timeSpinBox->setRange(1, 10000);
    m_timeSpinBox->setValue(300); // 300 секунд достаточно для большинства переходов, 1000 слишком долго для симуляции 1000 траекторий

    // --- Новый layout: параметры слева, графики справа ---
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    QFormLayout *paramsLayout = new QFormLayout();
    QVBoxLayout *chartsLayout = new QVBoxLayout();
    // Настроить внешние отступы
    mainLayout->setContentsMargins(10, 10, 10, 10);
    paramsLayout->setContentsMargins(0, 0, 0, 0);
    chartsLayout->setContentsMargins(0, 0, 0, 0);

    // Параметры моделирования
    // --- Одинаковая ширина для всех спинбоксов ---
    int spinBoxWidth = 80;
    m_aSpinBox->setFixedWidth(spinBoxWidth);
    m_gammaLineEdit->setFixedWidth(spinBoxWidth);
    m_timeSpinBox->setFixedWidth(spinBoxWidth);

    // --- Элементы управления для MST-эксперимента ---
    m_xAxisMode = new QComboBox();
    
    m_xAxisMode->addItem("По оси X: Шум (D)");
    m_xAxisMode->addItem("По оси X: Частота (ω)");

    m_noiseDLineEdit = new QLineEdit("0.01");
    m_noiseDLineEdit->setFixedWidth(spinBoxWidth);
    m_noiseDLineEdit->setVisible(false);

    m_dMinSpinBox = new QDoubleSpinBox();
    m_dMinSpinBox->setFixedWidth(spinBoxWidth);
    m_dMinSpinBox->setRange(0.0001, 10.0);
    m_dMinSpinBox->setDecimals(4);
    m_dMinSpinBox->setSingleStep(0.001);
    m_dMinSpinBox->setValue(0.001);

    m_dMaxSpinBox = new QDoubleSpinBox();
    m_dMaxSpinBox->setFixedWidth(spinBoxWidth);
    m_dMaxSpinBox->setRange(0.01, 10.0);
    m_dMaxSpinBox->setSingleStep(0.1);
    m_dMaxSpinBox->setValue(1.0);

    m_dStepSpinBox = new QDoubleSpinBox();
    m_dStepSpinBox->setFixedWidth(spinBoxWidth);
    m_dStepSpinBox->setRange(5, 500);
    m_dStepSpinBox->setSingleStep(5);
    m_dStepSpinBox->setValue(20);

    m_thresholdSpinBox = new QDoubleSpinBox();
    m_thresholdSpinBox->setFixedWidth(spinBoxWidth);
    m_thresholdSpinBox->setRange(0.0, 10.0);
    m_thresholdSpinBox->setSingleStep(0.01);
    m_thresholdSpinBox->setValue(M_PI);

    m_trialsSpinBox = new QSpinBox();
    m_trialsSpinBox->setFixedWidth(spinBoxWidth);
    m_trialsSpinBox->setRange(1, 100000);
    m_trialsSpinBox->setValue(1000); // 1000 trajectories for better smoothing

    m_switchingSignalCheckBox = new QCheckBox("Переключающий сигнал");
    
    m_switchingSignalCheckBox->setChecked(true); // Запускаем с переключением по умолчанию

    m_switchingAmplitudeSpinBox = new QDoubleSpinBox();
    m_switchingAmplitudeSpinBox->setFixedWidth(spinBoxWidth);
    m_switchingAmplitudeSpinBox->setRange(0.0, 10.0);
    m_switchingAmplitudeSpinBox->setSingleStep(0.01);
    m_switchingAmplitudeSpinBox->setValue(1.0); // A=1.0 из статьи (или A=0.7)

    m_switchingFrequencySpinBox = new QDoubleSpinBox();
    m_switchingFrequencySpinBox->setFixedWidth(spinBoxWidth);
    m_switchingFrequencySpinBox->setRange(0.01, 10.0);
    m_switchingFrequencySpinBox->setSingleStep(0.01);
    m_switchingFrequencySpinBox->setValue(0.4); // ω=0.4 - там, где наиболее выражен эффект NES

    // --- Вертикальное размещение всех элементов ---
    m_useHeunCheckBox = new QCheckBox("Использовать Хьюна (вместо Эйлера)");
    m_useHeunCheckBox->setChecked(true); // Хьюн по умолчанию

    // --- Seed для генератора случайных чисел ---
    m_seedSpinBox = new QSpinBox();
    m_seedSpinBox->setFixedWidth(spinBoxWidth);
    m_seedSpinBox->setRange(0, 1000000);
    m_seedSpinBox->setValue(42);

    m_randomSeedCheckBox = new QCheckBox("Рандомный seed");
    
    m_randomSeedCheckBox->setChecked(false); // Без рандомного сида по умолчанию
    m_seedSpinBox->setEnabled(true);
    connect(m_randomSeedCheckBox, &QCheckBox::toggled, this, [this](bool checked){
        m_seedSpinBox->setEnabled(!checked);
    });

    paramsLayout->addRow("a =", m_aSpinBox);
    paramsLayout->addRow("γ (через запятую) =", m_gammaLineEdit);
    paramsLayout->addRow("Время:", m_timeSpinBox);
    paramsLayout->addRow("", m_useHeunCheckBox);
    paramsLayout->addRow("", m_randomSeedCheckBox);
    paramsLayout->addRow("Seed:", m_seedSpinBox);
    paramsLayout->addRow(m_xAxisMode);
    paramsLayout->addRow("D min =", m_dMinSpinBox);
    paramsLayout->addRow("D max =", m_dMaxSpinBox);
    paramsLayout->addRow("D шаг =", m_dStepSpinBox);
    paramsLayout->addRow("Порог =", m_thresholdSpinBox);
    paramsLayout->addRow("Траекторий =", m_trialsSpinBox);
    paramsLayout->addRow("", m_switchingSignalCheckBox);
    paramsLayout->addRow("Амплитуда =", m_switchingAmplitudeSpinBox);
    paramsLayout->addRow("Частота ω =", m_switchingFrequencySpinBox);
    paramsLayout->addRow("D (через запятую) =", m_noiseDLineEdit);

    connect(m_xAxisMode, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this, paramsLayout](int index) {
        auto setRowVisible = [paramsLayout](QWidget* w, bool visible) {
            w->setVisible(visible);
            if (auto lbl = paramsLayout->labelForField(w)) {
                lbl->setVisible(visible);
            }
        };
        auto setRowLabel = [paramsLayout](QWidget* w, const QString& text) {
            if (auto lbl = qobject_cast<QLabel*>(paramsLayout->labelForField(w))) {
                lbl->setText(text);
            }
        };

        if (index == 0) { // Шум (D)
            setRowVisible(m_noiseDLineEdit, false);
            setRowVisible(m_switchingFrequencySpinBox, true);
            setRowLabel(m_dMinSpinBox, "D min =");
            setRowLabel(m_dMaxSpinBox, "D max =");
            setRowLabel(m_dStepSpinBox, "Точек D =");
            m_mstChart->setTitle("Среднее время переключения vs интенсивность шума");
            m_mstChart->axes(Qt::Horizontal).first()->setTitleText("Интенсивность шума D (log scale)");
        } else { // Частота (ω)
            setRowVisible(m_noiseDLineEdit, true);
            setRowVisible(m_switchingFrequencySpinBox, false);
            setRowLabel(m_dMinSpinBox, "ω min =");
            setRowLabel(m_dMaxSpinBox, "ω max =");
            setRowLabel(m_dStepSpinBox, "Точек ω =");
            m_mstChart->setTitle("Среднее время переключения vs частота сигнала");
            m_mstChart->axes(Qt::Horizontal).first()->setTitleText("Частота сигнала ω (log scale)");
        }
    });
    // Trigger it once to initialize labels properly
    m_xAxisMode->setCurrentIndex(0);

    // Фиксированная ширина левой панели
    QWidget *paramsWidget = new QWidget;
    paramsWidget->setLayout(paramsLayout);
    paramsWidget->setMinimumWidth(260);
    paramsWidget->setMaximumWidth(300);

    // --- Кнопки зелёного цвета ---
    m_mstVsNoiseButton = new QPushButton("Построить MST vs шум");
    m_mstVsNoiseButton->setFixedWidth(180);
    m_mstVsNoiseButton->setStyleSheet("background-color: #4CAF50; color: white; font-weight: bold;");
    m_runButton = new QPushButton("Запустить моделирование");
    m_runButton->setFixedWidth(180);
    m_runButton->setStyleSheet("background-color: #4CAF50; color: white; font-weight: bold;");
    
    m_clearMstButton = new QPushButton("Очистить график MST");
    m_clearMstButton->setFixedWidth(180);
    m_clearMstButton->setStyleSheet("background-color: #f44336; color: white; font-weight: bold;");

    connect(m_runButton, &QPushButton::clicked, this, &SecondOrderWidget::runSimulation);
    connect(m_mstVsNoiseButton, &QPushButton::clicked, this, &SecondOrderWidget::runMSTvsNoiseExperiment);
    connect(m_clearMstButton, &QPushButton::clicked, this, &SecondOrderWidget::clearMstChart);
    
    paramsLayout->addRow(m_runButton);
    paramsLayout->addRow(m_mstVsNoiseButton);
    paramsLayout->addRow(m_clearMstButton);

    m_resultLabel = new QLabel("Задержка включения: -");
    m_resultLabel->setAlignment(Qt::AlignCenter);
    m_resultLabel->setStyleSheet("font-weight: bold; font-size: 14px;");
    paramsLayout->addRow(m_resultLabel);
    

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

    // m_mstSeries удалена, будем добавлять новые серии динамически
    m_mstChart = new QChart();
    m_mstChart->setTitle("Среднее время переключения vs интенсивность шума");
    QLogValueAxis *mstAxisX = new QLogValueAxis();
    mstAxisX->setBase(10.0);
    mstAxisX->setTitleText("Интенсивность шума D (log scale)");
    mstAxisX->setLabelFormat("%g");
    m_mstChart->addAxis(mstAxisX, Qt::AlignBottom);
    QValueAxis *mstAxisY = new QValueAxis();
    mstAxisY->setTitleText("Среднее время MST, c (linear)");
    mstAxisY->setLabelFormat("%g");
    m_mstChart->addAxis(mstAxisY, Qt::AlignLeft);
    m_mstChartView = new QChartView(m_mstChart);
    m_mstChartView->setRenderHint(QPainter::Antialiasing);
    chartsLayout->addWidget(m_mstChartView);

    mainLayout->addWidget(paramsWidget, 0);
    mainLayout->addLayout(chartsLayout, 2);
    setLayout(mainLayout);
}
void SecondOrderWidget::runMSTvsNoiseExperiment()
{
    double a = m_aSpinBox->value();
    int maxTime = m_timeSpinBox->value();
    double dt = 0.01;
    int steps = static_cast<int>(maxTime / dt);
    
    m_model->setA(a);
    m_model->setDt(dt);
    m_model->setSteps(steps);
    m_model->setUseHeun(m_useHeunCheckBox->isChecked());

    double minVal = m_dMinSpinBox->value();
    double maxVal = m_dMaxSpinBox->value();
    if (minVal <= 0) minVal = 1e-4;

    bool sweepFrequency = (m_xAxisMode->currentIndex() == 1);
    
    // Parse QLineEdits
    QStringList gammaStrs = m_gammaLineEdit->text().split(',', Qt::SkipEmptyParts);
    QStringList fixedDStrs = m_noiseDLineEdit->text().split(',', Qt::SkipEmptyParts);
    
    std::vector<double> gammas;
    for (const auto& s : gammaStrs) gammas.push_back(s.toDouble());
    if (gammas.empty()) gammas.push_back(1.0);

    std::vector<double> fixedDs;
    for (const auto& s : fixedDStrs) fixedDs.push_back(s.toDouble());
    if (fixedDs.empty()) fixedDs.push_back(0.01);

    double fixedFreq = m_switchingFrequencySpinBox->value();

    std::vector<double> sweepPoints;
    int numPoints = static_cast<int>(m_dStepSpinBox->value()); 
    double logMin = std::log10(minVal);
    double logMax = std::log10(maxVal);
    for (int i = 0; i < numPoints; ++i) {
        double val = std::pow(10, logMin + i * (logMax - logMin) / (numPoints - 1));
        sweepPoints.push_back(val);
    }
    
    double threshold = m_thresholdSpinBox->value();
    int trials = m_trialsSpinBox->value();

    bool withSwitching = m_switchingSignalCheckBox->isChecked();
    double switchingAmplitude = m_switchingAmplitudeSpinBox->value();
    
    if (m_randomSeedCheckBox->isChecked()) {
        m_model->setSeed(static_cast<int>(std::random_device{}()));
    } else {
        m_model->setSeed(m_seedSpinBox->value());
    }

    QProgressDialog progress("Вычисление траекторий...", "Отмена", 0, 100, this);
    progress.setWindowModality(Qt::WindowModal);
    progress.setMinimumDuration(0);
    progress.setValue(0);
    QApplication::processEvents();

    SecondOrderModel* model_ptr_safe = m_model;
    bool withSwitchingBool = withSwitching; 

    // Compute combinations
    int total_combos = sweepFrequency ? (gammas.size() * fixedDs.size()) : gammas.size();
    int completed_combos = 0;

    for (double gamma : gammas) {
        if (progress.wasCanceled()) break;
        
        // Loop over fixedDs if we sweep Frequency, otherwise just one pass
        std::vector<double> D_list = sweepFrequency ? fixedDs : std::vector<double>{fixedDs.front()};
        for (double D : D_list) {
            if (progress.wasCanceled()) break;
            
            QFuture<std::vector<std::pair<double, double>>> future = QtConcurrent::run(
                [model_ptr_safe, gamma, sweepFrequency, D, fixedFreq, sweepPoints, threshold, trials, withSwitchingBool, switchingAmplitude]() {
                    model_ptr_safe->setGamma(gamma);
                    return model_ptr_safe->computeMSTvsSweep(sweepFrequency, D, fixedFreq, sweepPoints, threshold, trials, withSwitchingBool, switchingAmplitude);
                }
            );
            
            while (!future.isFinished()) {
                QApplication::processEvents(QEventLoop::AllEvents, 50);
                if (progress.wasCanceled()) {
                    future.cancel();
                    break;
                }
                QThread::msleep(50);
            }
            if (progress.wasCanceled()) break;
            
            auto results = future.result();
            QLineSeries *newMstSeries = new QLineSeries();
            newMstSeries->setPointsVisible(true); // <-- ДОБАВИТЬ ТОЧКИ
            
            QString seriesName;
            if (sweepFrequency) {
                seriesName = QString("γ=%1, A=%2, D=%3").arg(gamma).arg(withSwitching ? switchingAmplitude : 0.0).arg(D);
            } else {
                seriesName = QString("γ=%1, A=%2, ω=%3").arg(gamma).arg(withSwitching ? switchingAmplitude : 0.0).arg(withSwitching ? fixedFreq : 0.0);
            }
            newMstSeries->setName(seriesName);

            for (const auto& pair : results) {
                double val = pair.first;
                double MST = pair.second > 0 ? pair.second : 1e-4;
                newMstSeries->append(val, MST);
            }
            m_mstChart->addSeries(newMstSeries);
            
            completed_combos++;
            progress.setValue(completed_combos * 100 / total_combos);
        }
    }
    
    // Automatically re-create standard axes around new logic to rescale correctly across series
    m_mstChart->createDefaultAxes();
    if (!m_mstChart->axes(Qt::Horizontal).isEmpty() && !m_mstChart->axes(Qt::Vertical).isEmpty()) {
        QAbstractAxis *oldAxisX = m_mstChart->axes(Qt::Horizontal).first();
        QAbstractAxis *oldAxisY = m_mstChart->axes(Qt::Vertical).first();
        
        double minX = static_cast<QValueAxis*>(oldAxisX)->min();
        double maxX = static_cast<QValueAxis*>(oldAxisX)->max();
        double minY = static_cast<QValueAxis*>(oldAxisY)->min();
        double maxY = static_cast<QValueAxis*>(oldAxisY)->max();
        
        QLogValueAxis *logAxisX = new QLogValueAxis();
        if (sweepFrequency)
            logAxisX->setTitleText("Частота сигнала ω (log scale)");
        else
            logAxisX->setTitleText("Интенсивность шума D (log scale)");
        logAxisX->setBase(10.0);
        logAxisX->setMinorTickCount(-1);
        logAxisX->setMin(std::max(1e-5, minX * 0.9));
        logAxisX->setMax(std::max(1e-4, maxX * 1.1));
        
        QLogValueAxis *logAxisY = new QLogValueAxis();
        logAxisY->setTitleText("Среднее время MST, с (log scale)");
        logAxisY->setBase(10.0);
        logAxisY->setMinorTickCount(-1);
        logAxisY->setMin(std::max(1e-5, minY * 0.9));
        logAxisY->setMax(std::max(1e-4, maxY * 1.1));
        
        m_mstChart->removeAxis(oldAxisX);
        m_mstChart->removeAxis(oldAxisY);
        m_mstChart->addAxis(logAxisX, Qt::AlignBottom);
        m_mstChart->addAxis(logAxisY, Qt::AlignLeft);
        
        for(auto series : m_mstChart->series()) {
            series->attachAxis(logAxisX);
            series->attachAxis(logAxisY);
        }
    }

}

void SecondOrderWidget::runSimulation()
{
    double a = m_aSpinBox->value();
    QStringList gammas = m_gammaLineEdit->text().split(","); double gamma = gammas.isEmpty() ? 1.0 : gammas.first().toDouble();
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


void SecondOrderWidget::clearMstChart() {
    m_mstChart->removeAllSeries();
}
