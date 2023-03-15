
#include "tools.h"
#include <QDebug>
#include <QLineF>
#include <QRgb>
#include <QImage>

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
      if (lineA.intersects(lineB, nullptr) == QLineF::BoundedIntersection)
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

QString MrWriter::transformToString(const QTransform & transform)
{
  QString string;
  string
      .append(QString::number(transform.m11())).append(" ")
      .append(QString::number(transform.m12())).append(" ")
      .append(QString::number(transform.m13())).append(" ")
      .append(QString::number(transform.m21())).append(" ")
      .append(QString::number(transform.m22())).append(" ")
      .append(QString::number(transform.m23())).append(" ")
      .append(QString::number(transform.m31())).append(" ")
      .append(QString::number(transform.m32())).append(" ")
      .append(QString::number(transform.m33()));
  return string;
}

QTransform MrWriter::stringToTransform(const QString & string)
{
  QTransform transform;
  QStringList list = string.split(" ", Qt::SkipEmptyParts);
  if (list.size() < 9)
  {
    return transform;
  }
  qreal m11 = list.at(0).toDouble();
  qreal m12 = list.at(1).toDouble();
  qreal m13 = list.at(2).toDouble();
  qreal m21 = list.at(3).toDouble();
  qreal m22 = list.at(4).toDouble();
  qreal m23 = list.at(5).toDouble();
  qreal m31 = list.at(6).toDouble();
  qreal m32 = list.at(7).toDouble();
  qreal m33 = list.at(8).toDouble();
  transform.setMatrix(m11, m12, m13, m21, m22, m23, m31, m32, m33);

  return transform;
}

bool MrWriter::hasTransparancy(const QImage& image)
{
  bool useAlpha = false;
  const uchar* pixelData = image.bits();
  int bytes = image.sizeInBytes();

  for (const QRgb* pixel = reinterpret_cast<const QRgb*>(pixelData); bytes > 0; pixel++, bytes -= sizeof(QRgb)) {
    if (qAlpha(*pixel) != UCHAR_MAX) {
      useAlpha = true;
        break;
    }
  }
  return useAlpha;
}
