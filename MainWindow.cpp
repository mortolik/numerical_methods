#include "MainWindow.hpp"
#include "Euler/EulerModel.hpp"
#include "Euler/EulerWidget.hpp"
#include <QHBoxLayout>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent) {
    setObjectName("mainWindow");

    mainWidget = new QWidget(this);
    QHBoxLayout* pTopLayout = new QHBoxLayout(mainWidget);
    setCentralWidget(mainWidget);

    m_eulerModel = new Euler::EulerModel(0.5, 1000);
    m_eulerModel->setDt(0.01);

    m_eulerWidget = new Euler::EulerWidget(m_eulerModel, this);
    pTopLayout->addWidget(m_eulerWidget);

    setMinimumSize(800, 600);

}

MainWindow::~MainWindow() {
    delete m_eulerModel;
    delete m_eulerWidget;
}
