#include <QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(wvdialer_icons);
    QApplication a(argc, argv);
    QString name("WvDialer");
    a.setOrganizationName(name);
    a.setApplicationName(name);
    MainWindow w;
    w.hide();

    return a.exec();
}
