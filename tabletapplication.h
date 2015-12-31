/*
#####################################################################
Copyright (C) 2015 Thomas Leitz (thomas.leitz@web.de)
#####################################################################

LICENSE:

This file is part of MrWriter.

MrWriter is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License 2.0 as published
by the Free Software Foundation.

MrWriter is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with MrWriter.  If not, see <http://www.gnu.org/licenses/>.
#####################################################################
*/

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
    TabletApplication(int &argc, char **argv);

    bool isUsingTablet();

    QVector<QMainWindow*> mainWindows;

    MrDoc::Selection clipboard;

public slots:
    void exit();

protected:
    bool event(QEvent *event) Q_DECL_OVERRIDE;

private:
//    bool event(QEvent *event) Q_DECL_OVERRIDE;

    bool usingTablet = false;


};

#endif // TABLETAPPLICATION_H
