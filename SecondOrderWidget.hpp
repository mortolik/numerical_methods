#ifndef SECONDORDERWIDGET_HPP
#define SECONDORDERWIDGET_HPP

#include <QWidget>
#include <QtCharts>
#include <QPushButton>
#include <QDoubleSpinBox>
#include <QLineEdit>
#include <QSpinBox>
#include <QGraphicsTextItem>
#include "SecondOrderModel.hpp"

using namespace QtCharts;

class SecondOrderWidget : public QWidget {
    Q_OBJECT

public:
    SecondOrderWidget(SecondOrderModel *model, QWidget *parent = nullptr);
    bool exportPresetChart(
        int presetIndex,
        const QString &filePath,
        double dMinOverride = -1.0,
        double dMaxOverride = -1.0,
        int pointsOverride = -1);

private slots:
    void runSimulation();
    void runMSTvsNoiseExperiment();
    void clearMstChart();
    void copyMstChart();
    void expandMstChart();
    void expandTrajectoryChart();
    void copyTrajectoryChart();
    void loadPreset(int index);
    void buildAllPresets();
    void saveMstChart();

private:
    SecondOrderModel *m_model;
    QDoubleSpinBox *m_aSpinBox;
    QLineEdit *m_gammaLineEdit;
    QSpinBox *m_timeSpinBox;
    QSpinBox *m_seedSpinBox;
    QCheckBox *m_randomSeedCheckBox;

    QtCharts::QChart *m_chart;
    QtCharts::QChartView *m_chartView;
    QtCharts::QLineSeries *m_series;
    QtCharts::QLineSeries *m_seriesClean;

    QPushButton *m_runButton;
    QPushButton *m_mstVsNoiseButton;
    QPushButton *m_clearMstButton;
    QLabel *m_resultLabel;

    QCheckBox *m_useHeunCheckBox;

    // Для MST-эксперимента
    // Presets and saving
    QComboBox *m_presetsComboBox;
    QPushButton *m_saveMstButton;
    QPushButton *m_buildAllPresetsButton;
    
    QComboBox *m_xAxisMode;
    QLineEdit *m_noiseDLineEdit;
    QDoubleSpinBox *m_dMinSpinBox;
    QDoubleSpinBox *m_dMaxSpinBox;
    QDoubleSpinBox *m_dStepSpinBox;
    QDoubleSpinBox *m_thresholdSpinBox;
    QSpinBox *m_trialsSpinBox;
    QCheckBox *m_switchingSignalCheckBox;
    QDoubleSpinBox *m_switchingAmplitudeSpinBox;
    QDoubleSpinBox *m_switchingFrequencySpinBox;

    // Для графика зависимости MST от шума
    QtCharts::QChart *m_mstChart;
    QtCharts::QChartView *m_mstChartView;
    QGraphicsTextItem *m_mstLastPointLabel;
    QPointF m_mstLastPoint;
    QVBoxLayout *m_chartsLayout;
    QPushButton *m_copyMstButton;
    QPushButton *m_expandMstButton;
    QPushButton *m_showTrajectoryButton;
};

#endif // SECONDORDERWIDGET_HPP
