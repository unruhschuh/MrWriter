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

#ifndef STROKE_H
#define STROKE_H

#include <QObject>
#include <QPainter>
#include <QVector>
#include <QVector2D>

#include "mrdoc.h"

namespace MrDoc {

    struct Stroke
    {
    public:
    //    enum class dashPattern { SolidLine, DashLine, DashDotLine, DotLine };
        void paint(QPainter &painter, qreal zoom, bool last = false);

        QRectF boundingRect();

        Stroke();
        QPolygonF points;
        QVector<qreal> pressures;
        QVector<qreal> pattern;
        qreal penWidth;
        QColor color;
    };

}

#endif // STROKE_H
