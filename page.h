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

        const QRectF &getDirtyRect();
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
