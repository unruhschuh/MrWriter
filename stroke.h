#ifndef STROKE_H
#define STROKE_H

#include <QObject>
#include <QPainter>
#include <QVector>
#include <QVector2D>

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

  //    enum class dashPattern { SolidLine, DashLine, DashDotLine, DotLine };
  void paint(QPainter &painter, qreal zoom, bool last = false);

  QRectF boundingRect() const;
  QRectF boundingRectSansPenWidth() const;

  void addPoint(MrDoc::Point const &point)
  {
    m_points.append(point.getPoint());
    m_pressures.append(point.getPressure());
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

private:
public:
  QPolygonF m_points;
  QVector<qreal> m_pressures;
  QVector<qreal> m_pattern;
  qreal m_penWidth;
  QColor m_color;
};
}

#endif // STROKE_H
