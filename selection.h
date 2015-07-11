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

    QPolygonF selectionPolygon;

    int pageNum;

private:

};

#endif // SELECTION_H
