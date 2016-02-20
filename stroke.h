#ifndef STROKE_H
#define STROKE_H

#include <QObject>
#include <QPainter>
#include <QVector>
#include <QVector2D>

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
  void paint(QPainter &painter, qreal zoom, bool last = false);

  QRectF boundingRect() const;
  QRectF boundingRectSansPenWidth() const;

  QPolygonF m_points;
  QVector<qreal> m_pressures;
  QVector<qreal> m_pattern;
  qreal m_penWidth;
  QColor m_color;
};
}

#endif // STROKE_H
