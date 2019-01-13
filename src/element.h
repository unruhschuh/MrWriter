#ifndef ELEMENT_H
#define ELEMENT_H

#include <QPainter>

namespace MrDoc {

class Element
{
public:
  Element();
  virtual ~Element() { }

  virtual void paint(QPainter &painter, qreal zoom, bool last = false) = 0;
  virtual Element * clone() const = 0;
  virtual QRectF boundingRect() const = 0;
  virtual bool containedInPolygon(QPolygonF selectionPolygon);
};

}

#endif // ELEMENT_H

