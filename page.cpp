#include "page.h"
#include <QDebug>

Page::Page()
{
    // set up standard page (Letter, white background)
    setWidth(595.0);
    setHeight(842.0);
//    setWidth(600.0);
//    setHeight(800.0);
    setBackgroundColor(QColor(255,255,255));
}


qreal Page::getHeight()
{
    return height;
}

qreal Page::getWidth()
{
    return width;
}

void Page::setHeight(qreal newHeight)
{
    if (newHeight > 0)
    {
        height = newHeight;
    }
}

void Page::setWidth(qreal newWidth)
{
    if (newWidth > 0)
    {
        width = newWidth;
    }
}

void Page::paint(QPainter &painter, qreal zoom, QRectF region, bool pdf)
{
    for (int i = 0; i < curves.length(); ++i)
    {
        Curve curve = curves.at(i);
        if (region.isNull() || curve.points.boundingRect().intersects(region))
        {
            QPen pen;
            pen.setColor(curve.color);
            if (curve.pattern != Curve::solidLinePattern)
            {
                pen.setDashPattern(curve.pattern);
            }
            pen.setCapStyle(Qt::RoundCap);
            painter.setPen(pen);
            qreal dashOffset = 0.0;
            if (curve.pattern != Curve::solidLinePattern && pdf == true)
            {
                QTransform scaleTrans;
                scaleTrans = scaleTrans.scale(zoom, zoom);
                pen.setWidthF(curve.penWidth);
                painter.setPen(pen);
                painter.drawPolyline(scaleTrans.map(curve.points));
            } else {
                for (int j = 1; j < curve.points.length(); ++j)
                {
                    qreal tmpPenWidth = zoom * curve.penWidth * (curve.pressures.at(j-1) + curve.pressures.at(j)) / 2.0;
                    if (curve.pattern != Curve::solidLinePattern)
                    {
                        pen.setDashOffset(dashOffset);
                    }
                    pen.setWidthF(tmpPenWidth);
                    painter.setPen(pen);
                    painter.drawLine(zoom * curve.points.at(j-1), zoom * curve.points.at(j));
                    if (tmpPenWidth != 0)
                        dashOffset += 1.0/tmpPenWidth * (QLineF(zoom * curve.points.at(j-1), zoom * curve.points.at(j))).length();
                }
            }
        }
        if (curve.points.length() == 1)
        {
            QRectF pointRect(zoom * curve.points[0], QSizeF(0,0));
            qreal pad = curve.penWidth * zoom / 2;
            painter.setPen(Qt::NoPen);
            painter.setBrush(QBrush(curve.color));
            painter.drawEllipse(pointRect.adjusted(-pad,-pad,pad,pad));
        }
    }
}

void Page::setBackgroundColor(QColor newBackgroundColor)
{
    backgroundColor = newBackgroundColor;
}

QColor Page::getBackgroundColor()
{
    return backgroundColor;
}
