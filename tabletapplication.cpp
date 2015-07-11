#include "tabletapplication.h"
//#include "mainwindow.h"

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
    return QApplication::event(event);
}

bool TabletApplication::isUsingTablet()
{
    return usingTablet;
}
