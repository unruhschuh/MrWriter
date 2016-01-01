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

#ifndef PAGE_H
#define PAGE_H

#include "stroke.h"

namespace MrDoc {

    class Page
    {
    public:
        Page();

        qreal getWidth() const;
        qreal getHeight() const;

        void setWidth(qreal newWidth);
        void setHeight(qreal newHeight);

        void setBackgroundColor(QColor newBackgroundColor);
        QColor getBackgroundColor(void);

        const QRectF &getDirtyRect() const;
        void clearDirtyRect();

        QVector<QPair<Stroke, int>> getStrokes(QPolygonF &selectionPolygon);
        QVector<QPair<Stroke, int>> removeStrokes(QPolygonF &selectionPolygon);
        void removeStrokeAt(int i);

        void insertStrokes(QVector<QPair<Stroke, int>> &strokesAndPositions);
        void insertStroke(int position, Stroke &stroke);

        void appendStroke(Stroke &stroke);
        void appendStrokes(QVector<Stroke> &strokes);
        void prependStroke(Stroke &stroke);

    //    virtual void paint(QPainter &painter, qreal zoom);
        /**
         * @brief paint
         * @param painter
         * @param zoom
         * @param region
         */
        virtual void paint(QPainter &painter, qreal zoom, QRectF region = QRect(0,0,0,0));

    //    QVector<Stroke> strokes;
        QVector<Stroke> m_strokes;
        QColor backgroundColor;

    private:
        float width;  // post script units
        float height; // post script units

        QRectF dirtyRect;
    };


}

#endif // PAGE_H
