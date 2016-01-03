#ifndef STROKE_H
#define STROKE_H

#include <QObject>
#include <QPainter>
#include <QVector>
#include <QVector2D>

#include "mrdoc.h"

namespace MrDoc
{

struct Stroke
{
public:
  //    enum class dashPattern { SolidLine, DashLine, DashDotLine, DotLine };
  void paint(QPainter &painter, qreal zoom, bool last = false);

  QRectF boundingRect() const;

  Stroke();
  QPolygonF points;
  QVector<qreal> pressures;
  QVector<qreal> pattern;
  qreal penWidth;
  QColor color;
};
}

#endif // STROKE_H
