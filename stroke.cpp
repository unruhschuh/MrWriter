#include "stroke.h"

namespace MrDoc
{

Stroke::Stroke()
{
}

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

      if (tmpPenWidth != 0)
        dashOffset += 1.0 / tmpPenWidth * (QLineF(zoom * points.at(j - 1), zoom * points.at(j))).length();
    }
  }
}

QRectF Stroke::boundingRect()
{
  QRectF bRect = points.boundingRect();
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
}
