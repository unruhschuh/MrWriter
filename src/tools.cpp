
#include "tools.h"
#include <QDebug>

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
    area += x1 * y0 - x0 * y1;
  }

  x0 = polygon.last().x();
  y0 = polygon.last().y();
  x1 = polygon.first().x();
  y1 = polygon.first().y();

  area += x1 * y0 - x0 * y1;

  return area;
}

bool MrWriter::polygonIsClockwise(const QPolygonF &polygon)
{
  double area = polygonSignedArea(polygon);
  qDebug() << area;
  if (area > 0.0)
  {
    return true;
  }
  else
  {
    return false;
  }
}
