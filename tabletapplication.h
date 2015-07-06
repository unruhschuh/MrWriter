#include <QApplication>

#include "selection.h"

#ifndef TABLETAPPLICATION_H
#define TABLETAPPLICATION_H


class TabletApplication : public QApplication
{
public:
    TabletApplication(int &argv, char **args)
        : QApplication(argv, args) {}

    bool isUsingTablet();

    Selection clipboard;

private:
    bool event(QEvent *event) Q_DECL_OVERRIDE;

    bool usingTablet = false;


};

#endif // TABLETAPPLICATION_H
