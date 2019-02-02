#include "image.h"
#include "tools.h"
#include <QDebug>
#include <QBuffer>

namespace MrDoc
{

Image::Image(QPointF position)
{
  QTransform transform;
  transform.translate(position.x(), position.y());
  m_transform = transform;
}

void Image::paint(QPainter& painter, qreal zoom, bool last)
{
  (void)last;

  QTransform oldTransform = painter.transform();
  //QTransform newTransform = m_transform * zoom;
  QTransform newTransform = MrWriter::reallyScaleTransform(m_transform, zoom);
  painter.setTransform(newTransform, true);

  painter.drawImage(m_image.rect(), m_image, m_image.rect());
  painter.setTransform(oldTransform);
}

void Image::toXml(QXmlStreamWriter& writer)
{
  QByteArray ba;
  QBuffer buffer(&ba);
  buffer.open(QIODevice::WriteOnly);
  if (MrWriter::hasTransparancy(m_image))
  {
    m_image.save(&buffer, "PNG");
  }
  else
  {
    m_image.save(&buffer, "JPG");
  }

  writer.writeStartElement("image");
  writer.writeAttribute(QXmlStreamAttribute("matrix", MrWriter::transformToString(m_transform)));
  writer.writeCDATA(ba.toBase64());
  writer.writeEndElement();
}

void Image::fromXml(QXmlStreamReader& reader)
{
  QXmlStreamAttributes attributes = reader.attributes();
  QString elementText = reader.readElementText();
  QByteArray ba = QByteArray::fromBase64(elementText.toUtf8());
  m_image.loadFromData(ba);

  QString matrixString = attributes.value("", "matrix").toString();
  m_transform = MrWriter::stringToTransform(matrixString);
}

std::unique_ptr<Element> Image::clone() const
{
  return std::make_unique<Image>(*this);
}

bool Image::containedInPolygon(QPolygonF selectionPolygon)
{
  QPolygonF poly = boundingPolygon();

  bool containsImage;
  if (MrWriter::polygonIsClockwise(selectionPolygon))
  {
    //if (selectionPolygon.intersects(poly)) // requires Qt >= 5.10
    if (!selectionPolygon.intersected(poly).empty())
    {
      containsImage = true;
    }
    else
    {
      containsImage = false;
    }
  }
  else
  {
    containsImage = true;
    for (auto & point : poly)
    {
      if (!selectionPolygon.containsPoint(point, Qt::OddEvenFill))
      {
        containsImage = false;
      }
    }
    if (MrWriter::polygonLinesIntersect(selectionPolygon, poly))
    {
      containsImage = false;
    }
  }

  return containsImage;
}

void Image::transform(QTransform transform)
{
  m_transform = m_transform * transform;
}

QPolygonF Image::boundingPolygon() const
{
  QRectF rect = m_image.rect();

  QPolygonF poly(rect);
  poly = m_transform.map(poly);

  return poly;
}

QRectF Image::boundingRect() const
{
  QPolygonF poly = boundingPolygon();
  QRectF rect = poly.boundingRect();

  return rect;
}

}
