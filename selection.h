#ifndef SELECTION_H
#define SELECTION_H

#include "page.h"
#include "curve.h"

class Selection : public Page
{
public:
    Selection();

    void paint(QPainter &painter, qreal zoom);

    void addCurve(Curve newCurve);

    void transform(QTransform transform, int newPageNum);

    void finalize();

    void updateBuffer(qreal zoom);

    QPolygonF selectionPolygon;

    QImage buffer;
    QPointF buffPos = QPointF(0,0);
    qreal lastZoom = 0.0;

    int pageNum;

private:

};

#endif // SELECTION_H
