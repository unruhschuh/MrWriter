//#include <QApplication>
#include "mainwindow.h"
#include "tabletapplication.h"

int main(int argc, char *argv[])
{
//    QApplication a(argc, argv);
    TabletApplication a(argc, argv);
//    MainWindow w;
//    w.show();

    MainWindow *w = new MainWindow();
    a.mainWindows.append(w);
    w->show();

    return a.exec();
}
