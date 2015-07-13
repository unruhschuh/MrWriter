#ifndef TABLETAPPLICATION_H
#define TABLETAPPLICATION_H

#include <QApplication>

#include "selection.h"
//#include "mainwindow.h"
#include <QMainWindow>
#include <QVector>

class TabletApplication : public QApplication
{
public:
    TabletApplication(int &argv, char **args)
        : QApplication(argv, args) {}

    bool isUsingTablet();

    QVector<QMainWindow*> mainWindows;

    Selection clipboard;

protected:
    bool event(QEvent *event) Q_DECL_OVERRIDE;

private:
//    bool event(QEvent *event) Q_DECL_OVERRIDE;

    bool usingTablet = false;


};

#endif // TABLETAPPLICATION_H
