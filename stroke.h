#ifndef STROKE_H
#define STROKE_H

#include <QObject>
#include <QPainter>
#include <QVector>
#include <QVector2D>
#include <QPixmap>
#include <QDebug>

#include "mrdoc.h"

namespace MrDoc
{

/**
 * @brief The Stroke struct
 * @todo turn this into a class. make sure, points and pressures are of the same length using a setter
 */
struct Stroke
{
public:
  Stroke();
  //    enum class dashPattern { SolidLine, DashLine, DashDotLine, DotLine };
  void paint(QPainter &painter, QRectF &&rect, qreal zoom, bool last = false);

  QRectF boundingRect() const;
  QRectF boundingRectSansPenWidth() const;

  QPolygonF points;
  QVector<qreal> pressures;
  QVector<qreal> pattern;
  qreal penWidth;
  QColor color;
  QPixmap tmpPixmap =QPixmap(1,1);
};
}

#endif // STROKE_H
