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

#ifndef SELECTION_H
#define SELECTION_H

//#include "mrdoc.h"
#include "page.h"

namespace MrDoc {

    class Selection : public Page
    {
    public:
        Selection();

        virtual void paint(QPainter &painter, qreal zoom, QRectF region = QRect(0,0,0,0)) override;

        void transform(QTransform transform, int newPageNum);

        void finalize();

        void updateBuffer(qreal zoom);

        QPolygonF selectionPolygon;

        QImage buffer;
        QPointF buffPos = QPointF(0,0);
        qreal lastZoom = 0.0;

        qreal ad = 10;

        int pageNum;

    private:

    };
}
#endif // SELECTION_H
