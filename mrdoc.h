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

#ifndef MRDOC_H
#define MRDOC_H

#include <QColor>

namespace MrDoc {

    const QColor black      = QColor(0,0,0);
    const QColor blue       = QColor(51,51,204);
    const QColor red        = QColor(255,0,0);
    const QColor green      = QColor(0,128,0);
    const QColor gray       = QColor(128,128,128);
    const QColor lightblue  = QColor(0,192,255);
    const QColor lightgreen = QColor(0,255,0);
    const QColor magenta    = QColor(255,0,255);
    const QColor orange     = QColor(255,128,0);
    const QColor yellow     = QColor(255,255,0);
    const QColor white      = QColor(255,255,255);

    const QVector<QString> standardColorNames = QVector<QString>()
                                                << "black"
                                                << "blue"
                                                << "red"
                                                << "green"
                                                << "gray"
                                                << "lightblue"
                                                << "lightgreen"
                                                << "magenta"
                                                << "orange"
                                                << "yellow"
                                                << "white";

    const QVector<QColor>  standardColors     = QVector<QColor>()
                                                << MrDoc::black
                                                << MrDoc::blue
                                                << MrDoc::red
                                                << MrDoc::green
                                                << MrDoc::gray
                                                << MrDoc::lightblue
                                                << MrDoc::lightgreen
                                                << MrDoc::magenta
                                                << MrDoc::orange
                                                << MrDoc::yellow
                                                << MrDoc::white;

    const QVector<qreal> solidLinePattern = {1, 0};
    const QVector<qreal> dashLinePattern = {6, 3};
    const QVector<qreal> dashDotLinePattern = {6, 3, 0.5, 3};
    const QVector<qreal> dotLinePattern = {0.5, 3};
}

#endif // MRDOC_H

