#include "image.h"
#include <QDebug>

namespace MrDoc
{

Image::Image()
{

}

void Image::paint(QPainter& painter, qreal zoom, bool last)
{
  (void)last;

  QTransform oldTransform = painter.transform();
  QTransform newTransform = m_transform;
  painter.setTransform(newTransform, true);
  QRectF targetRect = QRectF(m_image.rect()).translated(zoom * m_pos);
  targetRect.setWidth(zoom * targetRect.width());
  targetRect.setHeight(zoom * targetRect.height());
  painter.drawImage(targetRect, m_image, m_image.rect());
  painter.setTransform(oldTransform);
}

std::unique_ptr<Element> Image::clone() const
{
  return std::make_unique<Image>(*this);
}

bool Image::containedInPolygon(QPolygonF selectionPolygon)
{
  QRectF rect = boundingRect();
  if (selectionPolygon.containsPoint(rect.topLeft(), Qt::OddEvenFill) &&
      selectionPolygon.containsPoint(rect.topRight(), Qt::OddEvenFill) &&
      selectionPolygon.containsPoint(rect.bottomLeft(), Qt::OddEvenFill) &&
      selectionPolygon.containsPoint(rect.bottomRight(), Qt::OddEvenFill) )
  {
    return true;
  }
  else
  {
    return false;
  }
}

void Image::transform(QTransform transform)
{
  m_pos = transform.map(m_pos);
  transform.translate(-transform.dx(), -transform.dy());
  qDebug() << transform.dx();
  qDebug() << transform.dy();
  m_transform = m_transform * transform;
}

QRectF Image::boundingRect() const
{
  QRectF rect = m_image.rect();

  QPolygonF poly(rect);
  m_transform.map(poly);
  rect.translate(m_pos);

  rect = poly.boundingRect();
  return rect;
}

}
