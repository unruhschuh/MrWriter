#include "mainwindow.h"
#include "tabletapplication.h"
#include <QApplication>

int main(int argc, char *argv[])
{
//    QApplication a(argc, argv);
    TabletApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}
