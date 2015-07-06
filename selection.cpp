#include "selection.h"

#include <iostream>

Selection::Selection()
{
    setWidth(10.0);
    setHeight(10.0);
    setBackgroundColor(QColor(255,255,255, 0)); // transparent
}

void Selection::paint(QPainter &painter, qreal zoom)
{
    painter.setRenderHint(QPainter::Antialiasing, true);

    Page::paint(painter, zoom);

    QPen pen;
    pen.setStyle(Qt::DashLine);
    pen.setCapStyle(Qt::RoundCap);
    painter.setBrush(QBrush(QColor(127, 127, 127, 50), Qt::SolidPattern));
    painter.setPen(pen);
    QTransform scaleTrans;
    scaleTrans = scaleTrans.scale(zoom,zoom);
    painter.drawPolygon(scaleTrans.map(selectionPolygon), Qt::OddEvenFill);
    painter.setRenderHint(QPainter::Antialiasing, false);
}

void Selection::transform(QTransform transform, int newPageNum)
{
    selectionPolygon = transform.map(selectionPolygon);
    for (int i = 0; i < curves.size(); ++i)
    {
        curves[i].points = transform.map(curves[i].points);
    }
    pageNum = newPageNum;
}

void Selection::finalize()
{
    QRectF boundingRect;
    for (int i = 0; i < curves.size(); ++i)
    {
        boundingRect = boundingRect.united(curves.at(i).points.boundingRect());
//        selectionPolygon = selectionPolygon.united(curves.at(i).points);
    }
    qreal ad = 10;
    boundingRect = boundingRect.adjusted(-ad,-ad,ad,ad);
    selectionPolygon = QPolygonF(boundingRect);

    setWidth(boundingRect.width());
    setHeight(boundingRect.height());

    std::cout << getWidth() << ", " << getHeight() << std::endl;
}

void Selection::addCurve(Curve newCurve)
{
    curves.append(newCurve);

//    QRectF newBoundingRect;

//    for (int i = 0; i < curves.size(); ++i)
//    {
//        newBoundingRect = newBoundingRect.united(curves.at(i).points.boundingRect());
//    }
//    boundingRect = newBoundingRect;
}
