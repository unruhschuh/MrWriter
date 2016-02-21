#ifndef POINT_H
#define POINT_H

#include <QPointF>

namespace MrDoc
{
class Point
{
public:
  Point()
  {
    point(QPointF(0.0, 0.0));
    pressure(1.0);
  }

  Point(QPointF const &pnt)
  {
    point(pnt);
    pressure(1.0);
  }

  Point(qreal x, qreal y)
  {
    point(x, y);
  }

  Point(QPointF const &pnt, qreal prsr)
  {
    point(pnt);
    pressure(prsr);
  }

  Point(qreal x, qreal y, qreal p)
  {
    point(x, y);
    pressure(p);
  }

  QPointF const &point()
  {
    return m_point;
  }
  qreal pressure()
  {
    return m_pressure;
  }

  void point(QPointF const &pnt)
  {
    m_point = pnt;
  }

  void point(qreal x, qreal y)
  {
    m_point = QPointF(x, y);
  }

  void pressure(qreal pressure)
  {
    if (pressure > 0)
    {
      m_pressure = pressure;
    }
  }

private:
  QPointF m_point;
  qreal m_pressure;
};
}

#endif // POINT_H
