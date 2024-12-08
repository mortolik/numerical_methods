#include "MainWindow.hpp"
#include "Euler/EulerModel.hpp"
#include "Euler/EulerWidget.hpp"
#include "Heun/HeunModel.hpp"
#include "Heun/HeunWidget.hpp"
#include <QHBoxLayout>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent) {
    setObjectName("mainWindow");

    QTabWidget *tabWidget = new QTabWidget(this);
    setCentralWidget(tabWidget);

    m_eulerModel = new Euler::EulerModel(0.5, 1000);
    m_eulerModel->setDt(0.01);
    m_eulerWidget = new Euler::EulerWidget(m_eulerModel, this);

    m_heunModel = new Heun::HeunModel(0.5, 1000);
    m_heunModel->setDt(0.01);
    m_heunWidget = new Heun::HeunWidget(m_heunModel, this);

    tabWidget->addTab(m_eulerWidget, "Метод Эйлера");
    tabWidget->addTab(m_heunWidget, "Метод Хьюна");

    setMinimumSize(800, 600);

}

MainWindow::~MainWindow() {
    delete m_eulerModel;
    delete m_eulerWidget;
}
