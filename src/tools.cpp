
#include "tools.h"
#include <QDebug>
#include <QLineF>

/*
 * The coordinate systeom:
 *
 * ------->
 * |      x
 * |
 * V  y
 *
 */
double MrWriter::polygonSignedArea(const QPolygonF &polygon)
{
  if (polygon.length() < 2)
  {
    return 0.0;
  }

  double area = 0.0;

  double x0 = 0.0;
  double y0 = 0.0;
  double x1 = 0.0;
  double y1 = 0.0;

  for (int i = 0; i < polygon.length() - 1; i++)
  {
    x0 = polygon.at(i).x();
    y0 = polygon.at(i).y();
    x1 = polygon.at(i+1).x();
    y1 = polygon.at(i+1).y();
    area += x0 * y1 - x1 * y0;
  }

  x0 = polygon.last().x();
  y0 = polygon.last().y();
  x1 = polygon.first().x();
  y1 = polygon.first().y();

  area += x0 * y1 - x1 * y0;

  return area;
}

bool MrWriter::polygonIsClockwise(const QPolygonF &polygon)
{
  double area = polygonSignedArea(polygon);

  if (area > 0.0)
  {
    return true;
  }
  else
  {
    return false;
  }
}


bool MrWriter::polygonLinesIntersect(const QPolygonF &polygonA, const QPolygonF &polygonB)
{
  QLineF lineA;
  QLineF lineB;
  for (int i = 0; i < polygonA.length() - 1; i++)
  {
    lineA.setP1(polygonA.at(i));
    lineA.setP2(polygonA.at(i+1));
    for (int j = 0; j < polygonB.length() - 1; j++)
    {
      lineB.setP1(polygonB.at(j));
      lineB.setP2(polygonB.at(j+1));
      if (lineA.intersect(lineB, nullptr) == QLineF::BoundedIntersection)
      {
        return true;
      }
    }
  }
  return false;
}

QTransform MrWriter::reallyScaleTransform(QTransform transform, qreal factor)
{
  qreal m11 = transform.m11() * factor;
  qreal m12 = transform.m12() * factor;
  qreal m13 = transform.m13();

  qreal m21 = transform.m21() * factor;
  qreal m22 = transform.m22() * factor;
  qreal m23 = transform.m23();

  qreal m31 = transform.m31() * factor;
  qreal m32 = transform.m32() * factor;
  qreal m33 = transform.m33();

  QTransform scaledTransform;
  scaledTransform.setMatrix(m11, m12, m13, m21, m22, m23, m31, m32, m33);
  return scaledTransform;
}
