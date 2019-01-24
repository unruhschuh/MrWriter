#ifndef IMAGE_H
#define IMAGE_H

#include "element.h"

namespace MrDoc
{

class Image : public Element
{
public:
  explicit Image(QPointF position);

  void paint(QPainter &painter, qreal zoom, bool last = false) override;
  std::unique_ptr<Element> clone() const override;
  bool containedInPolygon(QPolygonF selectionPolygon) override;
  void transform(QTransform transform) override;
  QPolygonF boundingPolygon() const;
  QRectF boundingRect() const override;

  QImage m_image;
  QTransform m_transform;
};

}

#endif // IMAGE_H
