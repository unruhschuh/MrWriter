#ifndef TEXT_H
#define TEXT_H

#include "element.h"

namespace MrDoc
{

class Text : public Element
{
public:
  Text();

  void paint(QPainter &painter, qreal zoom, bool last = false) override;
  void toXml(QXmlStreamWriter & writer) override;
  void fromXml(QXmlStreamReader & reader) override;
  std::unique_ptr<Element> clone() const override;
  bool containedInPolygon(QPolygonF selectionPolygon) override;
  void transform(QTransform transform) override;
  QRectF boundingRect() const override;

  QString m_text;
  QColor m_color;
  QTransform m_transform;
  qreal m_width;
  QFont m_font;
  QRectF textRect() const;
  QPolygonF boundingPolygon() const;
};

}

#endif // TEXT_H
