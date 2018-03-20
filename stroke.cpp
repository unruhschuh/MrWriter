#include "stroke.h"

namespace MrDoc
{

Stroke::Stroke()
{
    //tmpPixmap.fill(QColor(0,0,0,0));
}

//void Stroke::paint(QPainter &painter, qreal zoom, bool last)
//{
//    if (points.length() == 1)
//    {
//        QRectF pointRect(zoom * points[0], QSizeF(0, 0));
//        qreal pad = penWidth * zoom / 2;
//        painter.setPen(Qt::NoPen);
//        painter.setBrush(QBrush(color));
//        painter.drawEllipse(pointRect.adjusted(-pad, -pad, pad, pad));
//    }
//    else
//    {
//        QPen pen;
//        /*if(color.alpha() < 128)
//            color = QColor(255-color.red(), 255-color.green(), 255-color.blue(), color.alpha());*/
//        pen.setColor(color);
//        if (pattern != solidLinePattern)
//        {
//            pen.setDashPattern(pattern);
//        }
//        pen.setCapStyle(Qt::RoundCap);
//        painter.setPen(pen);

//        if(color.alpha() < 255)
//            painter.setCompositionMode(QPainter::CompositionMode_Exclusion);
//        else
//            painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
//        qreal dashOffset = 0.0;
//        for (int j = 1; j < points.length(); ++j)
//        {
//            qreal tmpPenWidth = zoom * penWidth * (pressures.at(j - 1) + pressures.at(j)) / 2.0;
//            if (pattern != solidLinePattern)
//            {
//                pen.setDashOffset(dashOffset);
//            }
//            pen.setWidthF(tmpPenWidth);
//            painter.setPen(pen);
//            //                painter.drawLine(zoom * points.at(j-1), zoom * points.at(j));
//            if (last == false)
//            {
//                painter.drawLine(zoom * points.at(j - 1), zoom * points.at(j));
//            }
//            else if (last == true && j == points.length() - 1)
//            {
//                painter.drawLine(zoom * points.at(j - 1), zoom * points.at(j));
//            }

//            if (tmpPenWidth != 0.0)
//                dashOffset += 1.0 / tmpPenWidth * (QLineF(zoom * points.at(j - 1), zoom * points.at(j))).length();
//        }
//        painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
//    }
//}

//void Stroke::paint(QPainter &painter, QRectF&& rect, qreal zoom, bool last)
//{
//    if (points.length() == 1)
//    {
//        QRectF pointRect(zoom * points[0], QSizeF(0, 0));
//        qreal pad = penWidth * zoom / 2;
//        painter.setPen(Qt::NoPen);
//        painter.setBrush(QBrush(color));
//        painter.drawEllipse(pointRect.adjusted(-pad, -pad, pad, pad));
//    }
//    else
//    {
//        if(tmpPixmap.rect().width() != rect.width() || tmpPixmap.rect().height() != rect.height()){
//            tmpPixmap = tmpPixmap.scaled(rect.width(), rect.height());
//        }
//        tmpPixmap.fill(QColor(0,0,0,0));
//        QPainter tmpPainter;
//        tmpPainter.begin(&tmpPixmap);
//        tmpPainter.setRenderHint(QPainter::Antialiasing, true);
//        QPen pen;
//        /*if(color.alpha() < 128)
//            color = QColor(255-color.red(), 255-color.green(), 255-color.blue(), color.alpha());*/
//        pen.setColor(color);
//        if (pattern != solidLinePattern)
//        {
//            pen.setDashPattern(pattern);
//        }
//        pen.setCapStyle(Qt::RoundCap);
//        tmpPainter.setPen(pen);

//        if(color.alpha() < 255)
//            tmpPainter.setCompositionMode(QPainter::CompositionMode_Source);
//        else
//            tmpPainter.setCompositionMode(QPainter::CompositionMode_SourceOver);
//        qreal dashOffset = 0.0;
//        for (int j = 1; j < points.length(); ++j)
//        {
//            qreal tmpPenWidth = zoom * penWidth * (pressures.at(j - 1) + pressures.at(j)) / 2.0;
//            if (pattern != solidLinePattern)
//            {
//                pen.setDashOffset(dashOffset);
//            }
//            pen.setWidthF(tmpPenWidth);
//            tmpPainter.setPen(pen);
//            //                painter.drawLine(zoom * points.at(j-1), zoom * points.at(j));
//            if (last == false)
//            {
//                tmpPainter.drawLine(zoom*(points.at(j - 1)-QPointF(boundingRect().x(),boundingRect().y())), zoom*(points.at(j)-QPointF(boundingRect().x(),boundingRect().y())));
//            }
//            else if (last == true && j == points.length() - 1)
//            {
//                tmpPainter.drawLine(zoom*(points.at(j - 1)-QPointF(boundingRect().x(),boundingRect().y())), zoom*(points.at(j)-QPointF(boundingRect().x(),boundingRect().y())));
//            }

//            if (tmpPenWidth != 0.0)
//                dashOffset += 1.0 / tmpPenWidth * (QLineF(zoom * points.at(j - 1), zoom * points.at(j))).length();
//        }
//        //tmpPainter.setCompositionMode(QPainter::CompositionMode_SourceOver);
//        tmpPainter.end();
//        qDebug() << boundingRect() << " vs. " << rect << " vs. " << tmpPixmap.rect();
//        qDebug() << zoom;
//        painter.drawPixmap(rect.toRect(), tmpPixmap);
//    }

//}

void Stroke::paint(QPainter &painter, qreal zoom, bool last)
{
    if (points.length() == 1)
    {
        QRectF pointRect(zoom * points[0], QSizeF(0, 0));
        qreal pad = penWidth * zoom / 2;
        painter.setPen(Qt::NoPen);
        painter.setBrush(QBrush(color));
        painter.drawEllipse(pointRect.adjusted(-pad, -pad, pad, pad));
    }
    else
    {
        if(!isHighlighter){
            QPen pen;
            pen.setColor(color);
            if (pattern != solidLinePattern)
            {
                pen.setDashPattern(pattern);
            }
            pen.setCapStyle(Qt::RoundCap);
            painter.setPen(pen);

            qreal dashOffset = 0.0;
            for (int j = 1; j < points.length(); ++j)
            {
                qreal tmpPenWidth = zoom * penWidth * (pressures.at(j - 1) + pressures.at(j)) / 2.0;
                if (pattern != solidLinePattern)
                {
                    pen.setDashOffset(dashOffset);
                }
                pen.setWidthF(tmpPenWidth);
                painter.setPen(pen);
                //                painter.drawLine(zoom * points.at(j-1), zoom * points.at(j));
                if (last == false)
                {
                    painter.drawLine(zoom * points.at(j - 1), zoom * points.at(j));
                }
                else if (last == true && j == points.length() - 1)
                {
                    painter.drawLine(zoom * points.at(j - 1), zoom * points.at(j));
                }

                if (tmpPenWidth != 0.0)
                    dashOffset += 1.0 / tmpPenWidth * (QLineF(zoom * points.at(j - 1), zoom * points.at(j))).length();
            }
        }
        else{
            QRectF bRect = boundingRect();
            QRectF bRectZ(boundingRect().x()*zoom, boundingRect().y()*zoom, boundingRect().width()*zoom, boundingRect().height()*zoom);
            if(tmpPixmap.rect().width() != bRectZ.width() || tmpPixmap.rect().height() != bRectZ.height()){
                tmpPixmap = tmpPixmap.scaled(bRectZ.width(), bRectZ.height());
            }
            tmpPixmap.fill(QColor(0,0,0,0));
            QPainter tmpPainter;
            tmpPainter.begin(&tmpPixmap);
            tmpPainter.setRenderHint(QPainter::Antialiasing, true);
            tmpPainter.setCompositionMode(QPainter::CompositionMode_Source);
            QPen pen;

            pen.setColor(color);
            if (pattern != solidLinePattern)
            {
                pen.setDashPattern(pattern);
            }
            pen.setCapStyle(Qt::RoundCap);
            tmpPainter.setPen(pen);

            qreal dashOffset = 0.0;
            for (int j = 1; j < points.length(); ++j)
            {
                qreal tmpPenWidth = zoom * penWidth;
                if (pattern != solidLinePattern)
                {
                    pen.setDashOffset(dashOffset);
                }
                pen.setWidthF(tmpPenWidth);
                tmpPainter.setPen(pen);
                //                painter.drawLine(zoom * points.at(j-1), zoom * points.at(j));
                QPointF upperLeftCorner(bRect.x(), bRect.y());
                if (last == false)
                {
                    tmpPainter.drawLine(zoom*(points.at(j - 1)-upperLeftCorner), zoom*(points.at(j)-upperLeftCorner));
                }
                else if (last == true && j == points.length() - 1)
                {
                    tmpPainter.drawLine(zoom*(points.at(j - 1)-upperLeftCorner), zoom*(points.at(j)-upperLeftCorner));
                }

                if (tmpPenWidth != 0.0)
                    dashOffset += 1.0 / tmpPenWidth * (QLineF(zoom * points.at(j - 1), zoom * points.at(j))).length();
            }
            //tmpPainter.setCompositionMode(QPainter::CompositionMode_SourceOver);
            tmpPainter.end();
            painter.drawPixmap(bRectZ.toRect(), tmpPixmap);
        }
    }

}

QRectF Stroke::boundingRect() const
{
    QRectF bRect = boundingRectSansPenWidth();
    if(!isHighlighter){
        qreal maxPressure = 0.0;
        for (qreal p : pressures)
        {
            if (p > maxPressure)
            {
                maxPressure = p;
            }
        }
        qreal pad = maxPressure * penWidth;
        return bRect.adjusted(-pad, -pad, pad, pad);
    }
    else{
        return bRect.adjusted(-penWidth, -penWidth, penWidth, penWidth);
    }
}

QRectF Stroke::boundingRectSansPenWidth() const
{
  QPolygonF tmpPoints = points;
  QRectF bRect = tmpPoints.boundingRect();
  if (bRect.isNull())
  {
    qreal ad = 0.0001;
    bRect.adjust(-ad,-ad,ad,ad);
  }
  return bRect;
}
}
