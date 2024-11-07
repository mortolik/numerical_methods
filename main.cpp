#include "MainWindow.hpp"
#include "qapplication.h"
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QCoreApplication::setOrganizationName("numerical_methods");
    QCoreApplication::setApplicationName("numerical_method");


    MainWindow w;
    w.show();

    return a.exec();
}
