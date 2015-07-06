#include "page.h"

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

void Page::paint(QPainter &painter, qreal zoom, QRectF region)
{
    for (int i = 0; i < curves.length(); ++i)
    {
        Curve curve = curves.at(i);
        if (region.isNull() || curve.points.boundingRect().intersects(region))
        {
            QPen pen;
            pen.setColor(curve.color);
            pen.setCapStyle(Qt::RoundCap);
            painter.setPen(pen);

            for (int j = 1; j < curve.points.length(); ++j)
            {
                qreal tmpPenWidth = zoom * curve.penWidth * (curve.pressures.at(j-1) + curve.pressures.at(j)) / 2.0;
                pen.setWidthF(tmpPenWidth);
                painter.setPen(pen);
                painter.drawLine(zoom * curve.points.at(j-1), zoom * curve.points.at(j));
            }
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
