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
    setPoint(QPointF(0.0, 0.0));
    setPressure(1.0);
  }

  Point(QPointF const &pnt)
  {
    setPoint(pnt);
    setPressure(1.0);
  }

  Point(qreal x, qreal y)
  {
    setPoint(x, y);
  }

  Point(QPointF const &pnt, qreal prsr)
  {
    setPoint(pnt);
    setPressure(prsr);
  }

  Point(qreal x, qreal y, qreal p)
  {
    setPoint(x, y);
    setPressure(p);
  }

  QPointF const &getPoint() const
  {
    return m_point;
  }

  qreal getPressure() const
  {
    return m_pressure;
  }

  void setPoint(QPointF const &pnt)
  {
    m_point = pnt;
  }

  void setPoint(qreal x, qreal y)
  {
    m_point = QPointF(x, y);
  }

  void setPressure(qreal pressure)
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
