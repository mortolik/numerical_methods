#include "SecondOrderWidget.hpp"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QMessageBox>
#include <QProgressDialog>
#include <QApplication>
#include <QClipboard>
#include <QDialog>
#include <QFileDialog>
#include <QDir>
#include <QDateTime>
#include <QPainter>
#include <QtConcurrent>
#include <QFuture>
#include <QGraphicsTextItem>
#include <limits>

static void updateMstLastPointLabel(QChart *chart, QGraphicsTextItem *label, const QPointF &point, const QString &text)
{
    if (!chart || !label) {
        return;
    }

    label->setHtml(QString(
        "<div style='background-color: rgba(255,255,255,220); border: 1px solid #444; padding: 2px 4px;'>"
        "<b>%1</b></div>").arg(text.toHtmlEscaped()));
    label->setZValue(1001.0);
    label->adjustSize();

    const QPointF pos = chart->mapToPosition(point);
    const QRectF bounds = label->boundingRect();
    const QRectF plotArea = chart->plotArea();
    const qreal x = std::min(std::max(pos.x() - bounds.width() / 2.0, plotArea.left()), plotArea.right() - bounds.width());
    const qreal y = plotArea.bottom() + 4.0;
    label->setPos(x, y);
    label->setVisible(true);
}

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
    m_chartsLayout = new QVBoxLayout();
    // Настроить внешние отступы
    mainLayout->setContentsMargins(10, 10, 10, 10);
    paramsLayout->setContentsMargins(0, 0, 0, 0);
    m_chartsLayout->setContentsMargins(0, 0, 0, 0);

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
    paramsLayout->addRow("α (через запятую) =", m_gammaLineEdit);
    paramsLayout->addRow("Время:", m_timeSpinBox);
    paramsLayout->addRow("", m_useHeunCheckBox);
    paramsLayout->addRow("", m_randomSeedCheckBox);
    paramsLayout->addRow("Seed:", m_seedSpinBox);

    m_presetsComboBox = new QComboBox();
    m_presetsComboBox->addItem("Пресеты: Свой...");
    m_presetsComboBox->addItem("Рис 1: Частотный отклик (A=1.0)");
    m_presetsComboBox->addItem("Рис 2: NES. Влияние частоты (α=1.0)");
    m_presetsComboBox->addItem("Рис 3: NES. Влияние затухания (ω=0.1)");
    m_presetsComboBox->addItem("Рис 4: Подпороговый сигнал (A=0.2)");
    m_presetsComboBox->addItem("Рис 5: Резонансная активация");
    m_presetsComboBox->addItem("Рис 6: Модель Крамерса (A=0)");
    connect(m_presetsComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &SecondOrderWidget::loadPreset);
    paramsLayout->addRow("Набор параметров:", m_presetsComboBox);

    m_buildAllPresetsButton = new QPushButton("Сгенерировать все пресеты");
    m_buildAllPresetsButton->setStyleSheet("background-color: #E91E63; color: white; font-weight: bold;");
    connect(m_buildAllPresetsButton, &QPushButton::clicked, this, &SecondOrderWidget::buildAllPresets);
    paramsLayout->addRow(m_buildAllPresetsButton);

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
        if (!m_mstChart || m_mstChart->axes(Qt::Horizontal).isEmpty()) {
            return;
        }

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

    m_copyMstButton = new QPushButton("Скопировать изображение");
    m_saveMstButton = new QPushButton("Сохранить график MST");
    m_saveMstButton->setStyleSheet("background-color: #FF9800; color: white; font-weight: bold;");
    connect(m_saveMstButton, &QPushButton::clicked, this, &SecondOrderWidget::saveMstChart);
    m_copyMstButton->setFixedWidth(180);
    m_copyMstButton->setStyleSheet("background-color: #2196F3; color: white; font-weight: bold;");

    m_expandMstButton = new QPushButton("Открыть график MST");
    m_expandMstButton->setFixedWidth(180);
    m_expandMstButton->setStyleSheet("background-color: #FF9800; color: white; font-weight: bold;");

    m_showTrajectoryButton = new QPushButton("Открыть график x(t)");
    m_showTrajectoryButton->setFixedWidth(180);
    m_showTrajectoryButton->setStyleSheet("background-color: #9C27B0; color: white; font-weight: bold;");

    connect(m_runButton, &QPushButton::clicked, this, &SecondOrderWidget::runSimulation);
    connect(m_mstVsNoiseButton, &QPushButton::clicked, this, &SecondOrderWidget::runMSTvsNoiseExperiment);
    connect(m_clearMstButton, &QPushButton::clicked, this, &SecondOrderWidget::clearMstChart);
    connect(m_copyMstButton, &QPushButton::clicked, this, &SecondOrderWidget::copyMstChart);
    connect(m_expandMstButton, &QPushButton::clicked, this, &SecondOrderWidget::expandMstChart);
    connect(m_showTrajectoryButton, &QPushButton::clicked, this, &SecondOrderWidget::expandTrajectoryChart);
    
    paramsLayout->addRow(m_runButton);
    paramsLayout->addRow(m_showTrajectoryButton);
    paramsLayout->addRow(m_mstVsNoiseButton);
    paramsLayout->addRow(m_clearMstButton);
    paramsLayout->addRow(m_copyMstButton);
    paramsLayout->addRow(m_saveMstButton);
    paramsLayout->addRow(m_expandMstButton);

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
    m_chartView->hide(); // Скрываем по умолчанию
    m_chartsLayout->addWidget(m_chartView);

    // m_mstSeries удалена, будем добавлять новые серии динамически
    m_mstChart = new QChart();
    m_mstChart->setTitle("Среднее время переключения vs интенсивность шума");
    m_mstChart->setMargins(QMargins(12, 12, 88, 40));
    m_mstChart->setPlotAreaBackgroundVisible(true);
    m_mstChart->setPlotAreaBackgroundBrush(Qt::NoBrush);
    m_mstChart->setPlotAreaBackgroundPen(QPen(QColor(120, 120, 120), 1));
    m_mstLastPointLabel = new QGraphicsTextItem(m_mstChart);
    m_mstLastPointLabel->setVisible(false);
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
    m_chartsLayout->addWidget(m_mstChartView);

    connect(m_mstChart, &QChart::plotAreaChanged, this, [this](const QRectF &) {
        if (m_mstLastPointLabel && m_mstLastPointLabel->isVisible()) {
            const QRectF bounds = m_mstLastPointLabel->boundingRect();
            const QPointF pos = m_mstChart->mapToPosition(m_mstLastPoint);
            const QRectF plotArea = m_mstChart->plotArea();
            const qreal x = std::min(std::max(pos.x() - bounds.width() / 2.0, plotArea.left()), plotArea.right() - bounds.width());
            m_mstLastPointLabel->setPos(x, plotArea.bottom() + 4.0);
        }
    });

    mainLayout->addWidget(paramsWidget, 0);
    mainLayout->addLayout(m_chartsLayout, 2);
    setLayout(mainLayout);
}
void SecondOrderWidget::runMSTvsNoiseExperiment()
{
    // Для пресетов NES (2 и 3) обычный запуск должен строить полный набор кривых,
    // а не одиночную линию текущих параметров.
    static bool presetAutoBuildInProgress = false;
    if (!presetAutoBuildInProgress) {
        int idx = m_presetsComboBox->currentIndex();
        if (idx == 2 || idx == 3) {
            presetAutoBuildInProgress = true;
            clearMstChart();

            // Keep caller-provided sweep range (used by export zooms).
            const double preservedDMin = m_dMinSpinBox->value();
            const double preservedDMax = m_dMaxSpinBox->value();
            const double preservedPoints = m_dStepSpinBox->value();
            loadPreset(idx);
            m_dMinSpinBox->setValue(preservedDMin);
            m_dMaxSpinBox->setValue(preservedDMax);
            m_dStepSpinBox->setValue(preservedPoints);

            if (idx == 2) {
                // NES vs frequency: несколько частот + статический предел (A=0, a=1.5)
                m_switchingSignalCheckBox->setChecked(true);
                m_aSpinBox->setValue(0.5);
                m_gammaLineEdit->setText("1.0");
                m_switchingAmplitudeSpinBox->setValue(1.0);
                m_switchingFrequencySpinBox->setValue(0.4);
                runMSTvsNoiseExperiment();
                m_switchingFrequencySpinBox->setValue(0.45);
                runMSTvsNoiseExperiment();
                m_switchingFrequencySpinBox->setValue(0.48);
                runMSTvsNoiseExperiment();
                m_switchingFrequencySpinBox->setValue(0.5);
                runMSTvsNoiseExperiment();

                m_switchingSignalCheckBox->setChecked(false);
                m_aSpinBox->setValue(1.5);
                m_switchingAmplitudeSpinBox->setValue(0.0);
                runMSTvsNoiseExperiment();
                m_switchingSignalCheckBox->setChecked(true);
            } else {
                // NES vs alpha: sweep по alpha + статическая асимптотика (A=0, a=1.5)
                m_switchingSignalCheckBox->setChecked(true);
                m_aSpinBox->setValue(0.5);
                m_switchingAmplitudeSpinBox->setValue(1.0);
                runMSTvsNoiseExperiment();

                m_switchingSignalCheckBox->setChecked(false);
                m_aSpinBox->setValue(1.5);
                m_switchingAmplitudeSpinBox->setValue(0.0);
                m_gammaLineEdit->setText("1.0");
                runMSTvsNoiseExperiment();
                m_switchingSignalCheckBox->setChecked(true);
            }

            presetAutoBuildInProgress = false;
            return;
        }
    }

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
                seriesName = QString("α=%1, A=%2, D=%3").arg(gamma).arg(withSwitching ? switchingAmplitude : 0.0).arg(D);
            } else {
                seriesName = QString("α=%1, A=%2, ω=%3").arg(gamma).arg(withSwitching ? switchingAmplitude : 0.0).arg(withSwitching ? fixedFreq : 0.0);
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
    
    // Rebuild log axes with stable bounds and readable labels.
    for (QAbstractAxis *axis : m_mstChart->axes()) {
        m_mstChart->removeAxis(axis);
    }

        // X range is known from sweep controls and should stay consistent.
        double minX = std::max(1e-5, minVal);
        double maxX = std::max(minX * 1.01, maxVal);

        // Keep the right boundary fixed at 1 only for sweeps that actually end near 1.
        if (!sweepFrequency && maxVal <= 1.0 && maxX >= 0.95) {
            maxX = 1.0;
        }

        // Compute Y range from all currently visible series.
        double minY = std::numeric_limits<double>::max();
        double maxY = 0.0;
        for (QAbstractSeries *series : m_mstChart->series()) {
            auto *line = qobject_cast<QLineSeries*>(series);
            if (!line) continue;
            const auto pts = line->pointsVector();
            for (const QPointF &p : pts) {
                if (p.y() > 0.0) {
                    minY = std::min(minY, p.y());
                    maxY = std::max(maxY, p.y());
                }
            }
        }
        if (!(minY < std::numeric_limits<double>::max()) || maxY <= 0.0) {
            minY = 1e-3;
            maxY = 1.0;
        }

        // Small multiplicative padding keeps curves away from plot borders.
        minY = std::max(1e-5, minY * 0.9);
        maxY = std::max(minY * 1.2, maxY * 1.1);

        double minDecade = std::pow(10.0, std::floor(std::log10(minX)));
        double maxDecade = std::pow(10.0, std::ceil(std::log10(maxX)));
        if (!sweepFrequency && maxVal <= 1.0 && maxX >= 0.95) {
            maxDecade = 1.0;
        }

        QLogValueAxis *logAxisX = new QLogValueAxis();
        if (sweepFrequency) {
            logAxisX->setTitleText("Частота сигнала ω (log scale)");
        } else {
            logAxisX->setTitleText("Интенсивность шума D (log scale)");
        }
        logAxisX->setBase(10.0);
        logAxisX->setMinorTickCount(-1);
        logAxisX->setLabelFormat("%.3g");
        logAxisX->setMin(minDecade);
        // Keep the rightmost decade label (e.g., 10 or 0.01) clearly inside plot area.
        logAxisX->setMax(maxDecade * 1.2);

        QLogValueAxis *logAxisY = new QLogValueAxis();
        logAxisY->setTitleText("Среднее время MST, с (log scale)");
        logAxisY->setBase(10.0);
        logAxisY->setMinorTickCount(-1);
        logAxisY->setLabelFormat("%.3g");
        logAxisY->setMin(minY);
        logAxisY->setMax(maxY);

        m_mstChart->addAxis(logAxisX, Qt::AlignBottom);
        m_mstChart->addAxis(logAxisY, Qt::AlignLeft);


        for (QAbstractSeries *series : m_mstChart->series()) {
            series->attachAxis(logAxisX);
            series->attachAxis(logAxisY);
        }

        m_mstChart->setMargins(QMargins(12, 12, 88, 40));
        QApplication::processEvents();
        if (!m_mstChart->series().isEmpty()) {
            auto *firstSeries = qobject_cast<QLineSeries*>(m_mstChart->series().first());
            if (firstSeries && !firstSeries->pointsVector().isEmpty()) {
                m_mstLastPoint = firstSeries->pointsVector().last();
                updateMstLastPointLabel(
                    m_mstChart,
                    m_mstLastPointLabel,
                    m_mstLastPoint,
                    QString::number(m_mstLastPoint.x(), 'g', 3));
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
    const QList<QAbstractAxis*> axes = m_mstChart->axes();
    for (QAbstractAxis *axis : axes) {
        m_mstChart->removeAxis(axis);
        axis->deleteLater();
    }
}

void SecondOrderWidget::copyMstChart() {
    QSize oldSize = m_mstChartView->size();
    m_mstChartView->setFixedSize(1000, 700);
        
    m_mstChart->legend()->setAlignment(Qt::AlignRight);
    QFont font = m_mstChart->legend()->font();
    font.setPointSize(12);
    m_mstChart->legend()->setFont(font);
        
    QApplication::processEvents();
    QPixmap p = m_mstChartView->grab();
        
    m_mstChartView->setMinimumSize(0, 0);
    m_mstChartView->setMaximumSize(16777215, 16777215);
    m_mstChartView->resize(oldSize);
    
    m_mstChart->legend()->setAlignment(Qt::AlignTop);
    font.setPointSize(10);
    m_mstChart->legend()->setFont(font);

    QApplication::clipboard()->setPixmap(p);
}

void SecondOrderWidget::expandMstChart() {
    QDialog dialog(this);
    dialog.setWindowTitle("График MST");
    dialog.resize(1000, 700);

    QVBoxLayout *layout = new QVBoxLayout(&dialog);
    layout->setContentsMargins(0, 0, 0, 0);
    
    QPushButton *copyDialogBtn = new QPushButton("Скопировать изображение (HD)", &dialog);
    copyDialogBtn->setStyleSheet("background-color: #2196F3; color: white; font-weight: bold; padding: 10px; margin: 5px;");
    connect(copyDialogBtn, &QPushButton::clicked, this, &SecondOrderWidget::copyMstChart);

    // Вынимаем chartView из главного окна
    m_chartsLayout->removeWidget(m_mstChartView);

    // Make legend more readable and beautiful
    m_mstChart->legend()->setAlignment(Qt::AlignRight);
    m_mstChart->legend()->setMarkerShape(QLegend::MarkerShapeFromSeries);
    QFont font = m_mstChart->legend()->font();
    font.setPointSize(12);
    m_mstChart->legend()->setFont(font);

    layout->addWidget(copyDialogBtn);
    layout->addWidget(m_mstChartView);
    
    dialog.exec();

    // Возвращаем все обратно после закрытия
    layout->removeWidget(m_mstChartView);
    m_mstChartView->setParent(this);
    m_chartsLayout->addWidget(m_mstChartView);

    m_mstChart->legend()->setAlignment(Qt::AlignTop);
    font.setPointSize(10);
    m_mstChart->legend()->setFont(font);
}

void SecondOrderWidget::copyTrajectoryChart() {
    QSize oldSize = m_chartView->size();
    m_chartView->setFixedSize(1000, 400); // Диалоговый размер для траекторий
    
    m_chart->legend()->setAlignment(Qt::AlignRight);
    QFont font = m_chart->legend()->font();
    font.setPointSize(12);
    m_chart->legend()->setFont(font);
    
    QApplication::processEvents();
    QPixmap p = m_chartView->grab();
    
    m_chartView->setMinimumSize(0, 0);
    m_chartView->setMaximumSize(16777215, 16777215);
    m_chartView->resize(oldSize);
    
    m_chart->legend()->setAlignment(Qt::AlignTop);
    font.setPointSize(10);
    m_chart->legend()->setFont(font);

    QApplication::clipboard()->setPixmap(p);
}

void SecondOrderWidget::expandTrajectoryChart() {
    QDialog dialog(this);
    dialog.setWindowTitle("График траектории x(t)");
    dialog.resize(1000, 400);

    QVBoxLayout *layout = new QVBoxLayout(&dialog);
    layout->setContentsMargins(0, 0, 0, 0);

    QPushButton *copyDialogBtn = new QPushButton("Скопировать изображение (HD)", &dialog);
    copyDialogBtn->setStyleSheet("background-color: #9C27B0; color: white; font-weight: bold; padding: 10px; margin: 5px;");
    connect(copyDialogBtn, &QPushButton::clicked, this, &SecondOrderWidget::copyTrajectoryChart);

    m_chartsLayout->removeWidget(m_chartView);
    layout->addWidget(copyDialogBtn);
    layout->addWidget(m_chartView);
    m_chartView->show();
    
    dialog.exec();

    // Возвращаем все обратно после закрытия
    layout->removeWidget(m_chartView);
    m_chartView->setParent(this);
    m_chartsLayout->insertWidget(0, m_chartView);
    m_chartView->hide(); // Скрываем на главном экране
}


void SecondOrderWidget::loadPreset(int index) {
    if (index == 0) return; // Custom
    
    m_timeSpinBox->setValue(1500); // Гладкие кривые
    m_thresholdSpinBox->setValue(3.14);
    m_trialsSpinBox->setValue(1500); // Много усреднений по просьбе научника
    
    if (index == 1) { // Рис 1: freq_gamma_sweep
        m_xAxisMode->setCurrentIndex(1); // freq mode
        m_dMinSpinBox->setValue(0.001);
        m_dMaxSpinBox->setValue(1.0);
        m_dStepSpinBox->setValue(20);
        m_aSpinBox->setValue(0.5);
        m_switchingAmplitudeSpinBox->setValue(1.0);
        m_gammaLineEdit->setText("0.2, 1.0, 5.0");
        m_noiseDLineEdit->setText("0.05");
    } else if (index == 2) { // Рис 2: nes_freq_sweep
        m_xAxisMode->setCurrentIndex(0); // noise mode
        m_dMinSpinBox->setValue(0.0001);
        m_dMaxSpinBox->setValue(10.0);
        m_dStepSpinBox->setValue(30);
        m_aSpinBox->setValue(0.5);
        m_switchingAmplitudeSpinBox->setValue(1.0);
        m_gammaLineEdit->setText("1.0");
        m_switchingFrequencySpinBox->setValue(0.4);
    } else if (index == 3) { // Рис 3: nes_alpha_sweep
        m_xAxisMode->setCurrentIndex(0);
        m_dMinSpinBox->setValue(0.001);
        m_dMaxSpinBox->setValue(1.0);
        m_dStepSpinBox->setValue(20);
        m_aSpinBox->setValue(0.5);
        m_switchingAmplitudeSpinBox->setValue(1.0);
        m_gammaLineEdit->setText("0.1, 1.0, 5.0");
        m_switchingFrequencySpinBox->setValue(0.1);
    } else if (index == 4) { // Рис 4: subthreshold
        m_xAxisMode->setCurrentIndex(0);
        m_dMinSpinBox->setValue(0.001);
        m_dMaxSpinBox->setValue(1.0);
        m_dStepSpinBox->setValue(20);
        m_aSpinBox->setValue(0.5);
        m_switchingAmplitudeSpinBox->setValue(0.2);
        m_gammaLineEdit->setText("0.1, 1.0, 3.0");
        m_switchingFrequencySpinBox->setValue(0.4);
    } else if (index == 5) { // Рис 5: sr_classical
        m_xAxisMode->setCurrentIndex(1); // freq mode
        m_dMinSpinBox->setValue(0.001);
        m_dMaxSpinBox->setValue(1.0);
        m_dStepSpinBox->setValue(20);
        m_aSpinBox->setValue(0.5);
        m_switchingAmplitudeSpinBox->setValue(1.0);
        m_gammaLineEdit->setText("1.0");
        m_noiseDLineEdit->setText("0.01, 0.05, 0.1, 0.5, 1.0");
    } else if (index == 6) { // Рис 6: kramers_pure
        m_xAxisMode->setCurrentIndex(0);
        m_dMinSpinBox->setValue(0.001);
        m_dMaxSpinBox->setValue(1.0);
        m_dStepSpinBox->setValue(20);
        m_aSpinBox->setValue(0.5);
        m_switchingAmplitudeSpinBox->setValue(0.0);
        m_gammaLineEdit->setText("0.5, 1.0, 2.0, 5.0");
        m_switchingFrequencySpinBox->setValue(0.4);
    }
}

void SecondOrderWidget::saveMstChart() {
    int idx = m_presetsComboBox->currentIndex();
    QString defName = "mst_chart_" + QDateTime::currentDateTime().toString("yyyy-MM-dd_HH-mm-ss") + ".png";
    if (idx == 1) defName = "freq_gamma_sweep.png";
    else if (idx == 2) defName = "nes_freq_sweep.png";
    else if (idx == 3) defName = "nes_alpha_sweep.png";
    else if (idx == 4) defName = "subthreshold.png";
    else if (idx == 5) defName = "sr_classical.png";
    else if (idx == 6) defName = "kramers_pure.png";
    else if (idx > 0) defName = m_presetsComboBox->currentText().replace(QRegExp("[^a-zA-Z0-9_а-яА-Я]"), "_") + ".png";
    
    QString defPath = QDir::currentPath() + "/analysis/images/" + defName;
    QString fileName = QFileDialog::getSaveFileName(this, "Сохранить график HD", defPath, "Images (*.png)");
    if (!fileName.isEmpty()) {
        QSize oldSize = m_mstChartView->size();
        m_mstChartView->setFixedSize(1000, 700); // Как в диалоговом окне
        
        m_mstChart->legend()->setAlignment(Qt::AlignRight);
        QFont font = m_mstChart->legend()->font();
        font.setPointSize(12);
        m_mstChart->legend()->setFont(font);
        
        QApplication::processEvents();
        QPixmap p = m_mstChartView->grab();
        
        m_mstChartView->setMinimumSize(0, 0);
        m_mstChartView->setMaximumSize(16777215, 16777215);
        m_mstChartView->resize(oldSize);
        
        m_mstChart->legend()->setAlignment(Qt::AlignTop);
        font.setPointSize(10);
        m_mstChart->legend()->setFont(font);
        
        p.save(fileName);
    }
}

bool SecondOrderWidget::exportPresetChart(
    int presetIndex,
    const QString &filePath,
    double dMinOverride,
    double dMaxOverride,
    int pointsOverride) {
    if (presetIndex <= 0 || presetIndex >= m_presetsComboBox->count() || filePath.isEmpty()) {
        return false;
    }

    const int oldPresetIndex = m_presetsComboBox->currentIndex();
    const double oldDMin = m_dMinSpinBox->value();
    const double oldDMax = m_dMaxSpinBox->value();
    const int oldPoints = static_cast<int>(m_dStepSpinBox->value());

    clearMstChart();
    m_presetsComboBox->setCurrentIndex(presetIndex);

    const bool hasRangeOverride = (dMinOverride > 0.0 && dMaxOverride > dMinOverride);
    if (hasRangeOverride) {
        m_dMinSpinBox->setValue(dMinOverride);
        m_dMaxSpinBox->setValue(dMaxOverride);
        if (pointsOverride >= 5) {
            m_dStepSpinBox->setValue(pointsOverride);
        }
    }

    runMSTvsNoiseExperiment();

    QChart *tempChart = new QChart();
    tempChart->setTitle(m_mstChart->title());
    tempChart->legend()->setVisible(true);
    tempChart->legend()->setAlignment(Qt::AlignTop);
    tempChart->setMargins(QMargins(20, 20, 88, 40));
    tempChart->setPlotAreaBackgroundVisible(true);
    tempChart->setPlotAreaBackgroundBrush(Qt::NoBrush);
    tempChart->setPlotAreaBackgroundPen(QPen(QColor(120, 120, 120), 1));
    tempChart->resize(1000, 700);

    double minX = std::numeric_limits<double>::max();
    double maxX = 0.0;
    double minY = std::numeric_limits<double>::max();
    double maxY = 0.0;

    const auto sourceSeries = m_mstChart->series();
    for (QAbstractSeries *series : sourceSeries) {
        auto *src = qobject_cast<QLineSeries*>(series);
        if (!src) continue;

        auto *copy = new QLineSeries();
        copy->setName(src->name());
        copy->setPen(src->pen());
        copy->setPointsVisible(src->pointsVisible());

        const auto points = src->pointsVector();
        for (const QPointF &p : points) {
            copy->append(p);
            if (p.x() > 0.0) {
                minX = std::min(minX, p.x());
                maxX = std::max(maxX, p.x());
            }
            if (p.y() > 0.0) {
                minY = std::min(minY, p.y());
                maxY = std::max(maxY, p.y());
            }
        }
        tempChart->addSeries(copy);
    }

    if (!(minX < std::numeric_limits<double>::max()) || maxX <= 0.0) {
        minX = 1e-4;
        maxX = 1.0;
    }
    if (!(minY < std::numeric_limits<double>::max()) || maxY <= 0.0) {
        minY = 1e-3;
        maxY = 1.0;
    }

    QLogValueAxis *logAxisX = new QLogValueAxis();
    logAxisX->setBase(10.0);
    logAxisX->setMinorTickCount(-1);
    logAxisX->setLabelFormat("%.3g");
    double minDecadeX = std::pow(10.0, std::floor(std::log10(minX)));
    double maxDecadeX = std::pow(10.0, std::ceil(std::log10(maxX)));
    logAxisX->setMin(minDecadeX);
    // Keep the rightmost decade label (e.g., 10 or 0.01) clearly inside plot area.
    logAxisX->setMax(maxDecadeX * 1.2);
    if (m_xAxisMode->currentIndex() == 1) {
        logAxisX->setTitleText("Частота сигнала ω (log scale)");
    } else {
        logAxisX->setTitleText("Интенсивность шума D (log scale)");
    }

    QLogValueAxis *logAxisY = new QLogValueAxis();
    logAxisY->setBase(10.0);
    logAxisY->setMinorTickCount(-1);
    logAxisY->setLabelFormat("%.3g");
    logAxisY->setMin(std::max(1e-5, minY * 0.9));
    logAxisY->setMax(std::max(minY * 1.2, maxY * 1.1));
    logAxisY->setTitleText("Среднее время MST, с (log scale)");

    tempChart->addAxis(logAxisX, Qt::AlignBottom);
    tempChart->addAxis(logAxisY, Qt::AlignLeft);

    const auto renderedSeries = tempChart->series();
    for (QAbstractSeries *series : renderedSeries) {
        series->attachAxis(logAxisX);
        series->attachAxis(logAxisY);
    }

    QChartView tempView(tempChart);
    tempView.setRenderHint(QPainter::Antialiasing);
    tempView.setFixedSize(1000, 700);
    tempView.setSceneRect(QRectF(0, 0, 1000, 700));
    tempView.show();
    QApplication::processEvents();

    QGraphicsTextItem *exportLastPointLabel = new QGraphicsTextItem(tempChart);
    if (!tempChart->series().isEmpty()) {
        auto *firstSeries = qobject_cast<QLineSeries*>(tempChart->series().first());
        if (firstSeries && !firstSeries->pointsVector().isEmpty()) {
            const QPointF lastPoint = firstSeries->pointsVector().last();
            updateMstLastPointLabel(
                tempChart,
                exportLastPointLabel,
                lastPoint,
                QString::number(lastPoint.x(), 'g', 3));
        }
    }

    QPixmap p(tempView.size());
    p.fill(Qt::white);
    QPainter painter(&p);
    tempView.render(&painter);

    painter.end();

    const bool saved = p.save(filePath);

    if (hasRangeOverride) {
        m_dMinSpinBox->setValue(oldDMin);
        m_dMaxSpinBox->setValue(oldDMax);
        m_dStepSpinBox->setValue(oldPoints);
    }
    if (oldPresetIndex != presetIndex) {
        m_presetsComboBox->setCurrentIndex(oldPresetIndex);
    }

    return saved;
}

void SecondOrderWidget::buildAllPresets() {
    QString defDir = QDir::currentPath() + "/analysis/images";
    QString saveDir = QFileDialog::getExistingDirectory(this, "Выберите папку для сохранения графиков", defDir);
    if (saveDir.isEmpty()) return;

    for (int i = 1; i < m_presetsComboBox->count(); ++i) {
        clearMstChart();
        m_presetsComboBox->setCurrentIndex(i);
        
        runMSTvsNoiseExperiment();
        
        QString fileNameString = "preset_" + QString::number(i) + ".png";
        if (i == 1) fileNameString = "freq_gamma_sweep.png";
        else if (i == 2) fileNameString = "nes_freq_sweep.png";
        else if (i == 3) fileNameString = "nes_alpha_sweep.png";
        else if (i == 4) fileNameString = "subthreshold.png";
        else if (i == 5) fileNameString = "sr_classical.png";
        else if (i == 6) fileNameString = "kramers_pure.png";
        else fileNameString = m_presetsComboBox->currentText().replace(QRegExp("[^a-zA-Z0-9_а-яА-Я=-]"), "_") + ".png";
        
        QString fileName = QDir(saveDir).filePath(fileNameString);
        
        QSize oldSize = m_mstChartView->size();
        m_mstChartView->setFixedSize(1000, 700);
        
        m_mstChart->legend()->setAlignment(Qt::AlignRight);
        QFont font = m_mstChart->legend()->font();
        font.setPointSize(12);
        m_mstChart->legend()->setFont(font);
        
        QApplication::processEvents(); // Ensure charts update before render
        QPixmap p = m_mstChartView->grab();
        
        m_mstChartView->setMinimumSize(0, 0);
        m_mstChartView->setMaximumSize(16777215, 16777215);
        m_mstChartView->resize(oldSize);
        
        m_mstChart->legend()->setAlignment(Qt::AlignTop);
        font.setPointSize(10);
        m_mstChart->legend()->setFont(font);
        
        p.save(fileName);
    }
}
