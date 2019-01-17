#ifndef ELEMENT_H
#define ELEMENT_H

#include <QPainter>
#include <memory>

namespace MrDoc {

class Element
{
public:
  Element();
  virtual ~Element() { }

  virtual void paint(QPainter &painter, qreal zoom, bool last = false) = 0;
  virtual std::unique_ptr<Element> clone() const = 0;
  virtual QRectF boundingRect() const = 0;
  virtual void transform(QTransform _transform) = 0;
  virtual bool containedInPolygon(QPolygonF selectionPolygon) = 0;
};

}

#endif // ELEMENT_H

