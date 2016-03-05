#include "stroke.h"

namespace MrDoc
{

Stroke::Stroke()
{
}

void Stroke::paint(QPainter &painter, qreal zoom, bool last)
{
  if (m_points.length() == 1)
  {
    QRectF pointRect(zoom * m_points[0], QSizeF(0, 0));
    qreal pad = m_penWidth * zoom / 2;
    painter.setPen(Qt::NoPen);
    painter.setBrush(QBrush(m_color));
    painter.drawEllipse(pointRect.adjusted(-pad, -pad, pad, pad));
  }
  else
  {
    QPen pen;
    pen.setColor(m_color);
    if (m_pattern != solidLinePattern)
    {
      pen.setDashPattern(m_pattern);
    }
    pen.setCapStyle(Qt::RoundCap);
    painter.setPen(pen);
    qreal dashOffset = 0.0;
    for (int j = 1; j < m_points.length(); ++j)
    {
      qreal tmpPenWidth = zoom * m_penWidth * (m_pressures.at(j - 1) + m_pressures.at(j)) / 2.0;
      if (m_pattern != solidLinePattern)
      {
        pen.setDashOffset(dashOffset);
      }
      pen.setWidthF(tmpPenWidth);
      painter.setPen(pen);
      //                painter.drawLine(zoom * points.at(j-1), zoom * points.at(j));
      if (last == false)
      {
        painter.drawLine(zoom * m_points.at(j - 1), zoom * m_points.at(j));
      }
      else if (last == true && j == m_points.length() - 1)
      {
        painter.drawLine(zoom * m_points.at(j - 1), zoom * m_points.at(j));
      }

      if (tmpPenWidth != 0)
        dashOffset += 1.0 / tmpPenWidth * (QLineF(zoom * m_points.at(j - 1), zoom * m_points.at(j))).length();
    }
  }
}

QRectF Stroke::boundingRect() const
{
  QRectF bRect = boundingRectSansPenWidth();
  qreal maxPressure = 0.0;
  for (qreal p : m_pressures)
  {
    if (p > maxPressure)
    {
      maxPressure = p;
    }
  }
  qreal pad = maxPressure * m_penWidth;
  return bRect.adjusted(-pad, -pad, pad, pad);
}

QRectF Stroke::boundingRectSansPenWidth() const
{
  QPolygonF tmpPoints = m_points;
  QRectF bRect = tmpPoints.boundingRect();
  if (bRect.isNull())
  {
    qreal ad = 0.0001;
    bRect.adjust(-ad,-ad,ad,ad);
  }
  return bRect;
}

bool Stroke::containedInPolygon(QPolygonF const &polygon, Qt::FillRule fillRule) const
{
  // check if all points are inside the polygon
  bool allPointsInsidePolygon = !std::any_of(m_points.begin(), m_points.end(), [&polygon, fillRule](QPointF const &point)
                                             {
                                               return !polygon.containsPoint(point, fillRule);
                                             });
  // check if the stroke intersects the polygon
  bool allLinesInsidePolygon = true;
  for (auto pointIt = m_points.cbegin(); pointIt != m_points.cend() - 1; ++pointIt)
  {
    for (auto polyIt = polygon.cbegin(); polyIt != polygon.cend() - 1; ++polyIt)
    {
      QLineF pointLine           = QLineF(*pointIt, *(pointIt + 1));
      QLineF polyLine            = QLineF(*polyIt, *(polyIt + 1));
      QPointF *intersectionPoint = nullptr;
      if (pointLine.intersect(polyLine, intersectionPoint) == QLineF::BoundedIntersection)
      {
        allLinesInsidePolygon = false;
      }
    }
  }
  return allPointsInsidePolygon && allLinesInsidePolygon;
}
}
