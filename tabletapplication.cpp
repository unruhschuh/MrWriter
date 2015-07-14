#include <QFileOpenEvent>
#include "tabletapplication.h"
#include "mainwindow.h"
//#include "mainwindow.h"

TabletApplication::TabletApplication(int &argc, char **argv) : QApplication(argc, argv)
{
    QCoreApplication::setOrganizationName("unruhschuh");
    QCoreApplication::setOrganizationDomain("unruhschuh.com");
    QCoreApplication::setApplicationName("MrWriter");
    QCoreApplication::setApplicationVersion("0.1");
}

bool TabletApplication::event(QEvent *event)
{
    if (event->type() == QEvent::TabletEnterProximity)
    {
        usingTablet = true;
    }
    if (event->type() == QEvent::TabletLeaveProximity)
    {
        usingTablet = false;
    }

    if (event->type() == QEvent::FileOpen)
    {
        QString fileName = static_cast<QFileOpenEvent*>(event)->file();
        MainWindow* newWindow = new MainWindow();
        newWindow->loadXOJ(fileName);
        mainWindows.append(newWindow);
        newWindow->show();
    }

    return QApplication::event(event);
}

bool TabletApplication::isUsingTablet()
{
    return usingTablet;
}


