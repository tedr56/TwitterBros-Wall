#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QCoreApplication::setOrganizationName("Adrien Rocher VJ");
    QCoreApplication::setOrganizationDomain("arochervj.wix.com/arochervj");
    QCoreApplication::setApplicationName("TwitterWall");

    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}
