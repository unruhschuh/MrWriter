#include "stroke.h"
#include "tools.h"

#include <cmath>
#include <memory>

namespace MrDoc
{

Stroke::Stroke()
{
}

void Stroke::paint(QPainter &painter, qreal zoom, bool last)
{
  if (points.length() == 1)
  {
    QRectF pointRect(zoom * points[0], QSizeF(0, 0));
    qreal pad = penWidth * zoom / 2;
    painter.setPen(Qt::NoPen);
    painter.setBrush(QBrush(color));
    painter.drawEllipse(pointRect.adjusted(-pad, -pad, pad, pad));
  }
  else
  {
    QPen pen;
    pen.setColor(color);
    if (pattern != solidLinePattern)
    {
      pen.setDashPattern(pattern);
    }
    pen.setCapStyle(Qt::RoundCap);
    painter.setPen(pen);
    qreal dashOffset = 0.0;
    for (int j = 1; j < points.length(); ++j)
    {
      qreal tmpPenWidth = zoom * penWidth * (pressures.at(j - 1) + pressures.at(j)) / 2.0;
      if (pattern != solidLinePattern)
      {
        pen.setDashOffset(dashOffset);
      }
      pen.setWidthF(tmpPenWidth);
      painter.setPen(pen);
      //                painter.drawLine(zoom * points.at(j-1), zoom * points.at(j));
      if (last == false)
      {
        painter.drawLine(zoom * points.at(j - 1), zoom * points.at(j));
      }
      else if (last == true && j == points.length() - 1)
      {
        painter.drawLine(zoom * points.at(j - 1), zoom * points.at(j));
      }

      if (tmpPenWidth != 0)
        dashOffset += 1.0 / tmpPenWidth * (QLineF(zoom * points.at(j - 1), zoom * points.at(j))).length();
    }
  }
}

void Stroke::toXml(QXmlStreamWriter& writer)
{
  writer.writeStartElement("stroke");
  writer.writeAttribute(QXmlStreamAttribute("tool", "pen"));
  writer.writeAttribute(QXmlStreamAttribute("color", MrDoc::toRGBA(this->color.name(QColor::HexArgb))));
  QString patternString;
  if (this->pattern == MrDoc::solidLinePattern)
  {
    patternString = "solid";
  }
  else if (this->pattern == MrDoc::dashLinePattern)
  {
    patternString = "dash";
  }
  else if (this->pattern == MrDoc::dashDotLinePattern)
  {
    patternString = "dashdot";
  }
  else if (this->pattern == MrDoc::dotLinePattern)
  {
    patternString = "dot";
  }
  else
  {
    patternString = "solid";
  }
  writer.writeAttribute(QXmlStreamAttribute("style", patternString));
  qreal width = this->penWidth;
  writer.writeAttribute(QXmlStreamAttribute("width", QString::number(width)));
  QString pressures;
  for (int k = 0; k < this->pressures.length(); ++k)
  {
    pressures.append(QString::number(this->pressures[k])).append(" ");
  }
  writer.writeAttribute((QXmlStreamAttribute("pressures", pressures.trimmed())));
  QString points;
  for (int k = 0; k < this->points.size(); ++k)
  {
    points.append(QString::number(this->points[k].x()));
    points.append(" ");
    points.append(QString::number(this->points[k].y()));
    points.append(" ");
  }
  writer.writeCharacters(points.trimmed());
  writer.writeEndElement(); // closing "stroke"
}

void Stroke::fromXml(QXmlStreamReader& reader)
{
  this->pattern.clear();
  this->points.clear();
  this->pressures.clear();

  QXmlStreamAttributes attributes = reader.attributes();
  QStringView color = attributes.value("", "color");
  this->color = stringToColor(color.toString());
  QStringView style = attributes.value("", "style");
  if (style.toString().compare("solid") == 0)
  {
    this->pattern = MrDoc::solidLinePattern;
  }
  else if (style.toString().compare("dash") == 0)
  {
    this->pattern = MrDoc::dashLinePattern;
  }
  else if (style.toString().compare("dashdot") == 0)
  {
    this->pattern = MrDoc::dashDotLinePattern;
  }
  else if (style.toString().compare("dot") == 0)
  {
    this->pattern = MrDoc::dotLinePattern;
  }
  else
  {
    this->pattern = MrDoc::solidLinePattern;
  }
  QStringView strokeWidth = attributes.value("", "width");
  this->penWidth = strokeWidth.toDouble();
  QString elementText = reader.readElementText();
  QStringList elementTextList = elementText.trimmed().split(" ");
  for (int i = 0; i + 1 < elementTextList.size(); i = i + 2)
  {
    this->points.append(QPointF(elementTextList.at(i).toDouble(), elementTextList.at(i + 1).toDouble()));
  }
  QStringView pressures = attributes.value("pressures");
  QStringList pressuresList = pressures.toString().trimmed().split(" ");
  for (int i = 0; i < pressuresList.length(); ++i)
  {
    if (pressuresList.length() == 0)
    {
      this->pressures.append(1.0);
    }
    else
    {
      this->pressures.append(pressuresList.at(i).toDouble());
    }
  }
}

std::unique_ptr<Element> Stroke::clone() const
{
  return std::make_unique<Stroke>(*this);
}

bool Stroke::containedInPolygon(QPolygonF selectionPolygon)
{
  bool containsStroke;
  if (MrWriter::polygonIsClockwise(selectionPolygon))
  {
    containsStroke = false;
    for (int j = 0; j < points.size(); ++j)
    {
      if (selectionPolygon.containsPoint(points.at(j), Qt::OddEvenFill))
      {
        containsStroke = true;
        break;
      }
    }
    if (MrWriter::polygonLinesIntersect(points, selectionPolygon))
    {
      containsStroke = true;
    }
  }
  else
  {
    containsStroke = true;
    for (int j = 0; j < points.size(); ++j)
    {
      if (!selectionPolygon.containsPoint(points.at(j), Qt::OddEvenFill))
      {
        containsStroke = false;
        break;
      }
    }
    if (MrWriter::polygonLinesIntersect(points, selectionPolygon))
    {
      containsStroke = false;
    }
  }
  return containsStroke;
}

void Stroke::transform(QTransform _transform)
{
  qreal s = sqrt(_transform.determinant());

  this->points = _transform.map(this->points);
  this->penWidth = this->penWidth * s;
}

QRectF Stroke::boundingRect() const
{
  QRectF bRect = boundingRectSansPenWidth();
  qreal maxPressure = 0.0;
  for (qreal p : pressures)
  {
    if (p > maxPressure)
    {
      maxPressure = p;
    }
  }
  qreal pad = maxPressure * penWidth;
  return bRect.adjusted(-pad, -pad, pad, pad);
}

QRectF Stroke::boundingRectSansPenWidth() const
{
  QPolygonF tmpPoints = points;
  QRectF bRect = tmpPoints.boundingRect();
  if (bRect.isNull())
  {
    qreal ad = 0.0001;
    bRect.adjust(-ad,-ad,ad,ad);
  }
  return bRect;
}

void Stroke::finalize()
{
  if (points.boundingRect().width() * points.boundingRect().width() + points.boundingRect().height() * points.boundingRect().height() < 1.0)
  {
    QPointF point = points.at(0);
    qreal pressure = pressures.at(0);
    points.clear();
    points.append(point);
    pressures.clear();
    pressures.append(pressure);
  }
}

}
