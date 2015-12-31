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

//#include <QApplication>
#include <QCommandLineParser>
#include <QDebug>
#include <QMessageBox>
#include "mainwindow.h"
#include "tabletapplication.h"

int main(int argc, char *argv[])
{
    TabletApplication a(argc, argv);

    QCommandLineParser parser;

    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("file", QCoreApplication::translate("main", "File to open."));

    parser.process(a);

    const QStringList args = parser.positionalArguments();

    MainWindow *w = new MainWindow();
    a.mainWindows.append(w);

    if (args.size() == 1)
    {
        QString fileName = args.at(0);
        QStringList fileNameSplitted = fileName.split(".");
        bool success = false;
        if (fileNameSplitted.last().compare(QString("xoj"), Qt::CaseInsensitive) == 0)
        {
            success = w->loadXOJ(fileName);
        } else if(fileNameSplitted.last().compare(QString("moj"), Qt::CaseInsensitive) == 0) {
            success = w->loadMOJ(fileName);
        }
        if (!success)
        {
            QMessageBox errMsgBox;
            errMsgBox.setText("Couldn't open file");
            errMsgBox.exec();
        }
    }
    w->show();
    w->updateGUI();

    return a.exec();
}
