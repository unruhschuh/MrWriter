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

void Stroke::paint(QPainter &painter, QRectF&& rect, qreal zoom, bool last)
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
        //QPixmap tmpPixmap(1,1);
        if(tmpPixmap.rect().width() != rect.width() || tmpPixmap.rect().height() != rect.height()){
            qDebug() << tmpPixmap.rect() << rect;
            tmpPixmap = tmpPixmap.scaled(rect.width(), rect.height());
        }
        tmpPixmap.fill(QColor(0,0,0,0));
        QPainter tmpPainter;
        tmpPainter.begin(&tmpPixmap);
        tmpPainter.setRenderHint(QPainter::Antialiasing, true);
        QPen pen;
        /*if(color.alpha() < 128)
            color = QColor(255-color.red(), 255-color.green(), 255-color.blue(), color.alpha());*/
        pen.setColor(color);
        if (pattern != solidLinePattern)
        {
            pen.setDashPattern(pattern);
        }
        pen.setCapStyle(Qt::RoundCap);
        tmpPainter.setPen(pen);

        if(color.alpha() < 255)
            tmpPainter.setCompositionMode(QPainter::CompositionMode_Source);
        else
            tmpPainter.setCompositionMode(QPainter::CompositionMode_SourceOver);
        qreal dashOffset = 0.0;
        for (int j = 1; j < points.length(); ++j)
        {
            qreal tmpPenWidth = zoom * penWidth * (pressures.at(j - 1) + pressures.at(j)) / 2.0;
            if (pattern != solidLinePattern)
            {
                pen.setDashOffset(dashOffset);
            }
            pen.setWidthF(tmpPenWidth);
            tmpPainter.setPen(pen);
            //                painter.drawLine(zoom * points.at(j-1), zoom * points.at(j));
            if (last == false)
            {
                tmpPainter.drawLine(zoom * points.at(j - 1), zoom * points.at(j));
            }
            else if (last == true && j == points.length() - 1)
            {
                tmpPainter.drawLine(zoom * points.at(j - 1), zoom * points.at(j));
            }

            if (tmpPenWidth != 0.0)
                dashOffset += 1.0 / tmpPenWidth * (QLineF(zoom * points.at(j - 1), zoom * points.at(j))).length();
        }
        //tmpPainter.setCompositionMode(QPainter::CompositionMode_SourceOver);
        tmpPainter.end();
        painter.drawPixmap(rect.toRect(), tmpPixmap);
    }

}

QRectF Stroke::boundingRect() const
{
  QRectF bRect = boundingRectSansPenWidth();
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
