#include "stroke.h"
#include "tools.h"

#include <cmath>
#include <memory>

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

std::unique_ptr<Element> Stroke::clone() const
{
  return std::make_unique<Stroke>(*this);
}

bool Stroke::containedInPolygon(QPolygonF selectionPolygon)
{
  bool containsStroke;
  if (MrWriter::polygonIsClockwise(selectionPolygon))
  {
    containsStroke = false;
    for (int j = 0; j < points.size(); ++j)
    {
      if (selectionPolygon.containsPoint(points.at(j), Qt::OddEvenFill))
      {
        containsStroke = true;
        break;
      }
    }
    if (MrWriter::polygonLinesIntersect(points, selectionPolygon))
    {
      containsStroke = true;
    }
  }
  else
  {
    containsStroke = true;
    for (int j = 0; j < points.size(); ++j)
    {
      if (!selectionPolygon.containsPoint(points.at(j), Qt::OddEvenFill))
      {
        containsStroke = false;
        break;
      }
    }
    if (MrWriter::polygonLinesIntersect(points, selectionPolygon))
    {
      containsStroke = false;
    }
  }
  return containsStroke;
}

void Stroke::transform(QTransform _transform)
{
  qreal s = sqrt(_transform.determinant());

  this->points = _transform.map(this->points);
  this->penWidth = this->penWidth * s;
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

void Stroke::finalize()
{
  if (points.boundingRect().width() * points.boundingRect().width() + points.boundingRect().height() * points.boundingRect().height() < 1.0)
  {
    QPointF point = points.at(1);
    points.clear();
    points.append(point);
  }
}

}
