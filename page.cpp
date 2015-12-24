#include "page.h"
#include "mrdoc.h"
#include <QDebug>

namespace MrDoc {

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
        for (int i = 0; i < strokes.length(); ++i)
        {
            Stroke stroke = strokes.at(i);
            if (region.isNull() || stroke.points.boundingRect().intersects(region))
            {
                QPen pen;
                pen.setColor(stroke.color);
                if (stroke.pattern != Stroke::solidLinePattern)
                {
                    pen.setDashPattern(stroke.pattern);
                }
                pen.setCapStyle(Qt::RoundCap);
                painter.setPen(pen);
                qreal dashOffset = 0.0;
                if (stroke.pattern != Stroke::solidLinePattern && pdf == true)
                {
                    QTransform scaleTrans;
                    scaleTrans = scaleTrans.scale(zoom, zoom);
                    pen.setWidthF(stroke.penWidth);
                    painter.setPen(pen);
                    painter.drawPolyline(scaleTrans.map(stroke.points));
                } else {
                    for (int j = 1; j < stroke.points.length(); ++j)
                    {
                        qreal tmpPenWidth = zoom * stroke.penWidth * (stroke.pressures.at(j-1) + stroke.pressures.at(j)) / 2.0;
                        if (stroke.pattern != Stroke::solidLinePattern)
                        {
                            pen.setDashOffset(dashOffset);
                        }
                        pen.setWidthF(tmpPenWidth);
                        painter.setPen(pen);
                        painter.drawLine(zoom * stroke.points.at(j-1), zoom * stroke.points.at(j));
                        if (tmpPenWidth != 0)
                            dashOffset += 1.0/tmpPenWidth * (QLineF(zoom * stroke.points.at(j-1), zoom * stroke.points.at(j))).length();
                    }
                }
            }
            if (stroke.points.length() == 1)
            {
                QRectF pointRect(zoom * stroke.points[0], QSizeF(0,0));
                qreal pad = stroke.penWidth * zoom / 2;
                painter.setPen(Qt::NoPen);
                painter.setBrush(QBrush(stroke.color));
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
}
