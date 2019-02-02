#include "text.h"
#include "tools.h"
#include "mrdoc.h"
#include <QtDebug>

namespace MrDoc
{

Text::Text()
{

}

void Text::paint(QPainter& painter, qreal zoom, bool last)
{
  (void)last;

  painter.setPen(m_color);

  QTransform oldTransform = painter.transform();
  QTransform newTransform = MrWriter::reallyScaleTransform(m_transform, zoom);
  painter.setTransform(newTransform, true);

  painter.setFont(m_font);
  qDebug() << "pointSize: " << painter.font().pointSizeF();
  //painter.drawText(boundingRect(), m_text);
  painter.drawText(textRect(), m_text);
  painter.setTransform(oldTransform);
}

void Text::toXml(QXmlStreamWriter& writer)
{
  writer.writeStartElement("text");
  writer.writeAttribute(QXmlStreamAttribute("matrix", MrWriter::transformToString(m_transform)));
  writer.writeAttribute(QXmlStreamAttribute("color", MrDoc::toRGBA(m_color.name(QColor::HexArgb))));
  writer.writeAttribute(QXmlStreamAttribute("font", m_font.toString()));
  writer.writeCharacters(m_text);
  writer.writeEndElement();

}

void Text::fromXml(QXmlStreamReader& reader)
{
  QXmlStreamAttributes attributes = reader.attributes();
  m_text = reader.readElementText();

  QString matrixString = attributes.value("", "matrix").toString();
  m_transform = MrWriter::stringToTransform(matrixString);

  QStringRef color = attributes.value("", "color");
  m_color = stringToColor(color.toString());

  QStringRef font = attributes.value("", "font");
  m_font.fromString(font.toString());
}

std::unique_ptr<Element> Text::clone() const
{
  return std::make_unique<Text>(*this);
}

bool Text::containedInPolygon(QPolygonF selectionPolygon)
{
  QPolygonF poly = boundingPolygon();

  bool containsText;
  if (MrWriter::polygonIsClockwise(selectionPolygon))
  {
    //if (selectionPolygon.intersects(poly)) // requires Qt >= 5.10
    if (!selectionPolygon.intersected(poly).empty())
    {
      containsText = true;
    }
    else
    {
      containsText = false;
    }
  }
  else
  {
    containsText = true;
    for (auto & point : poly)
    {
      if (!selectionPolygon.containsPoint(point, Qt::OddEvenFill))
      {
        containsText = false;
      }
    }
    if (MrWriter::polygonLinesIntersect(selectionPolygon, poly))
    {
      containsText = false;
    }
  }

  return containsText;
}

void Text::transform(QTransform transform)
{
  qDebug() << transform;
  qDebug() << m_transform;
  m_transform = m_transform * transform;
}

QPolygonF Text::boundingPolygon() const
{
  QRectF rect = textRect();

  QPolygonF poly(rect);
  poly = m_transform.map(poly);

  return poly;
}

QRectF Text::boundingRect() const
{
  QPolygonF poly = boundingPolygon();
  QRectF rect = poly.boundingRect();

  return rect;
}

QRectF Text::textRect() const
{
  QFontMetricsF fontMetrics(m_font);
  return fontMetrics.boundingRect(QRectF(0,0,5,5), Qt::AlignLeft, m_text);


//  int pixelWidth = 5;
//  int pixelHeight = 5;
//  QPainter painter;
//  QPixmap pixmap(pixelWidth, pixelHeight);
//
//  painter.begin(&pixmap);
//  painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
//
//  QRectF rect;
//  painter.drawText(QRectF(0,0,5,5), Qt::AlignLeft, m_text, &rect);
//  painter.end();
//  return rect;
}

}
