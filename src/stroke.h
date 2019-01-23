#ifndef STROKE_H
#define STROKE_H

#include <QObject>
#include <QPainter>
#include <QVector>
#include <QVector2D>
#include <memory>

#include "element.h"

#include "mrdoc.h"

namespace MrDoc
{

/**
 * @brief The Stroke struct
 * @todo turn this into a class. make sure, points and pressures are of the same length using a setter
 */
struct Stroke : public Element
{
public:
  Stroke();
  //    enum class dashPattern { SolidLine, DashLine, DashDotLine, DotLine };
  void paint(QPainter &painter, qreal zoom, bool last = false) override;
  std::unique_ptr<Element> clone() const override;
  bool containedInPolygon(QPolygonF selectionPolygon) override;
  void transform(QTransform _transform) override;
  void finalize();

  QRectF boundingRect() const override;
  QRectF boundingRectSansPenWidth() const;

  QPolygonF points;
  QVector<qreal> pressures;
  QVector<qreal> pattern;
  qreal penWidth;
  QColor color;
};
}

#endif // STROKE_H
