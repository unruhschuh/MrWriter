#include "selection.h"

#include <iostream>
#include <QDebug>

Selection::Selection()
{
    setWidth(10.0);
    setHeight(10.0);
    setBackgroundColor(QColor(255,255,255, 0)); // transparent
    lastZoom = 0.0;
}

void Selection::updateBuffer(qreal zoom)
{
    QPainter imgPainter;
    buffer = QImage(zoom * getWidth(), zoom * getHeight(), QImage::Format_ARGB32_Premultiplied);
    buffer.fill(qRgba(0,0,0,0));
    buffer.setAlphaChannel(buffer);
    imgPainter.begin(&buffer);
    imgPainter.setRenderHint(QPainter::Antialiasing, true);

    QTransform trans;
    qreal dx = selectionPolygon.boundingRect().topLeft().x();
    qreal dy = selectionPolygon.boundingRect().topLeft().y();
    trans = trans.translate(-dx, -dy);
    transform(trans, pageNum);
    Page::paint(imgPainter, zoom);
    trans = trans.translate(2*dx, 2*dy);
    transform(trans, pageNum);
    imgPainter.end();
    buffPos = QPointF(0,0);
}

void Selection::paint(QPainter &painter, qreal zoom)
{
    if (lastZoom != zoom)
    {
        lastZoom = zoom;
        updateBuffer(zoom);
    }

//    Page::paint(painter, zoom);

//    painter.setCompositionMode(QPainter::CompositionMode_Xor);
    painter.drawImage(zoom * selectionPolygon.boundingRect().topLeft(), buffer);

    painter.setRenderHint(QPainter::Antialiasing, true);
    QPen pen;
    pen.setStyle(Qt::DashLine);
    pen.setCapStyle(Qt::RoundCap);
    painter.setBrush(QBrush(QColor(127, 127, 127, 50), Qt::SolidPattern));
    painter.setPen(pen);
    QTransform scaleTrans;
    scaleTrans = scaleTrans.scale(zoom,zoom);
    painter.drawPolygon(scaleTrans.map(selectionPolygon), Qt::OddEvenFill);

    // draw edges for grabbing to resize
    pen.setWidthF(0.5);
    painter.setPen(pen);
    QRect brect = scaleTrans.map(selectionPolygon).boundingRect().toRect();
    painter.drawLine(brect.topLeft() + QPointF(ad/2, 0), brect.bottomLeft() + QPointF(ad/2, 0));
    painter.drawLine(brect.topRight() - QPointF(ad/2, 0), brect.bottomRight() - QPointF(ad/2, 0));
    painter.drawLine(brect.topLeft() + QPointF(0, ad/2), brect.topRight() + QPointF(0, ad/2));
    painter.drawLine(brect.bottomLeft() - QPointF(0, ad/2), brect.bottomRight() - QPointF(0, ad/2));
    painter.setRenderHint(QPainter::Antialiasing, false);

}

void Selection::transform(QTransform transform, int newPageNum)
{
    selectionPolygon = transform.map(selectionPolygon);
    for (int i = 0; i < curves.size(); ++i)
    {
        curves[i].points = transform.map(curves[i].points);
    }
    buffPos = transform.map(buffPos);
    pageNum = newPageNum;
}

void Selection::finalize()
{
    QRectF boundingRect;
    for (int i = 0; i < curves.size(); ++i)
    {
        boundingRect = boundingRect.united(curves.at(i).points.boundingRect());
    }

    boundingRect = boundingRect.adjusted(-ad,-ad,ad,ad);
    selectionPolygon = QPolygonF(boundingRect);

    setWidth(boundingRect.width());
    setHeight(boundingRect.height());


//    std::cout << getWidth() << ", " << getHeight() << std::endl;
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
