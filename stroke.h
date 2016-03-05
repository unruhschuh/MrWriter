#ifndef STROKE_H
#define STROKE_H

#include <QObject>
#include <QPainter>
#include <QVector>
#include <QVector2D>

#include <algorithm>

#include "mrdoc.h"
#include "point.h"

namespace MrDoc
{

/**
 * @brief The Stroke struct
 * @todo turn this into a class. make sure, points and pressures are of the same length using a setter
 */
class Stroke
{
public:
  Stroke();
  Stroke(QVector<qreal> pattern, qreal penWidth, QColor color)
  {
    setPattern(pattern);
    setPenWidth(penWidth);
    setColor(color);
  }

  /**
   * @brief Stroke
   * @param pattern
   * @param penWidth
   * @param color
   * @param points
   * @param pressures
   *
   * @todo error handling
   */
  Stroke(QVector<qreal> pattern, qreal penWidth, QColor color, QPolygonF const &points, QVector<qreal> const &pressures)
  {
    if (points.size() != pressures.size())
    {
      throw 42;
    }
    Stroke(pattern, penWidth, color);
    m_points    = points;
    m_pressures = pressures;
  }

  //    enum class dashPattern { SolidLine, DashLine, DashDotLine, DotLine };
  void paint(QPainter &painter, qreal zoom, bool last = false);

  QRectF boundingRect() const;
  QRectF boundingRectSansPenWidth() const;

  void addPoint(QPointF point, qreal pressure)
  {
    m_points.append(point);
    m_pressures.append(pressure);
  }

  void clearPoints()
  {
    m_points.clear();
    m_pressures.clear();
  }

  /**
   * @brief pattern
   * @param pattern
   * @todo assert pattern to be of valid
   */
  void setPattern(QVector<qreal> pattern)
  {
    m_pattern = pattern;
  }

  void setPenWidth(qreal penWidth)
  {
    if (penWidth > 0)
    {
      m_penWidth = penWidth;
    }
  }

  void setColor(QColor const &color)
  {
    m_color = color;
  }

  /**
   * @brief containedInPolygon
   *
   * Only if all points are inside the polygon, and there is no bounded intersection of the stroke and the polygon, the stroke is inside the polygon.
   *
   * @param polygon
   * @param fillRule
   * @return
   * @todo should become a free function as soon as there are more objects besides strokes
   */
  bool containedInPolygon(QPolygonF const &polygon, Qt::FillRule fillRule) const;

private:
  friend QVector<MrDoc::Stroke> splitStroke(MrDoc::Stroke origStroke, QLineF line, QLineF::IntersectType intersectType);

public:
  QPolygonF m_points;
  QVector<qreal> m_pressures;
  QVector<qreal> m_pattern;
  qreal m_penWidth;
  QColor m_color;
};
}

#endif // STROKE_H
