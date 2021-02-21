#include "selection.h"

#include "tools.h"

#include <iostream>
#include <QDebug>
#include <QtMath>

namespace MrDoc
{

Selection::Selection()
{
  m_pageNum = 0;
  setWidth(10.0);
  setHeight(10.0);
  setBackgroundColor(QColor(255, 255, 255, 0)); // transparent
}

void Selection::setPageNum(size_t pageNum)
{
  m_pageNum = pageNum;
}

size_t Selection::pageNum() const
{
  return m_pageNum;
}

void Selection::setSelectionPolygon(QPolygonF selectionPolygon)
{
  m_selectionPolygon = selectionPolygon;
}

QPolygonF Selection::selectionPolygon() const
{
  return m_selectionPolygon;
}

bool Selection::containsPoint(QPointF pagePos)
{
  return m_selectionPolygon.containsPoint(pagePos, Qt::OddEvenFill);
}

Selection::GrabZone Selection::grabZone(QPointF pagePos, qreal zoom)
{
  GrabZone grabZone = GrabZone::None;
  QRectF bRect = m_selectionPolygon.boundingRect();
  QRectF moveRect = bRect;

  qreal scaled_ad = m_ad / zoom;

  QRectF topRect = bRect;
  topRect.setTop(topRect.top() - scaled_ad);
  topRect.setHeight(scaled_ad);

  QRectF bottomRect = bRect;
  bottomRect.setTop(bottomRect.bottom());
  bottomRect.setHeight(scaled_ad);

  QRectF leftRect = bRect;
  leftRect.setLeft(leftRect.left() - scaled_ad);
  leftRect.setWidth(scaled_ad);

  QRectF rightRect = bRect;
  rightRect.setLeft(rightRect.right());
  rightRect.setWidth(scaled_ad);

  QRectF topLeftRect = bRect;
  topLeftRect.setLeft(topLeftRect.left() - scaled_ad);
  topLeftRect.setTop(topLeftRect.top() - scaled_ad);
  topLeftRect.setWidth(scaled_ad);
  topLeftRect.setHeight(scaled_ad);

  QRectF topRightRect = bRect;
  topRightRect.setLeft(topRightRect.right());
  topRightRect.setTop(topRightRect.top() - scaled_ad);
  topRightRect.setWidth(scaled_ad);
  topRightRect.setHeight(scaled_ad);

  QRectF bottomLeftRect = bRect;
  bottomLeftRect.setLeft(bottomLeftRect.left() - scaled_ad);
  bottomLeftRect.setTop(bottomLeftRect.bottom());
  bottomLeftRect.setWidth(scaled_ad);
  bottomLeftRect.setHeight(scaled_ad);

  QRectF bottomRightRect = bRect;
  bottomRightRect.setLeft(bottomRightRect.right());
  bottomRightRect.setTop(bottomRightRect.bottom());
  bottomRightRect.setWidth(scaled_ad);
  bottomRightRect.setHeight(scaled_ad);

  QRectF rotateRect = bRect;
  rotateRect.setTop(rotateRect.top() - 1.0 * scaled_ad - (m_rotateRectCenter + m_rotateRectRadius) / zoom);
  rotateRect.setLeft(rotateRect.center().x() - m_rotateRectRadius / zoom);
  rotateRect.setWidth(2.0 * m_rotateRectRadius / zoom);
  rotateRect.setHeight(2.0 * m_rotateRectRadius / zoom);

  if (moveRect.contains(pagePos))
  {
    grabZone = GrabZone::Move;
  }
  else if (topRect.contains(pagePos))
  {
    grabZone = GrabZone::Top;
  }
  else if (bottomRect.contains(pagePos))
  {
    grabZone = GrabZone::Bottom;
  }
  else if (leftRect.contains(pagePos))
  {
    grabZone = GrabZone::Left;
  }
  else if (rightRect.contains(pagePos))
  {
    grabZone = GrabZone::Right;
  }
  else if (topLeftRect.contains(pagePos))
  {
    grabZone = GrabZone::TopLeft;
  }
  else if (topRightRect.contains(pagePos))
  {
    grabZone = GrabZone::TopRight;
  }
  else if (bottomLeftRect.contains(pagePos))
  {
    grabZone = GrabZone::BottomLeft;
  }
  else if (bottomRightRect.contains(pagePos))
  {
    grabZone = GrabZone::BottomRight;
  }
  else if (rotateRect.contains(pagePos))
  {
    grabZone = GrabZone::Rotate;
  }
  qDebug() << bRect;
  qDebug() << rotateRect;
  return grabZone;
}

void Selection::setAngle(qreal angle)
{
  m_angle = angle;
}

void Selection::appendToSelectionPolygon(QPointF pagePos)
{
  m_selectionPolygon.append(pagePos);
}

QRectF Selection::boundingRect() const
{
  return m_selectionPolygon.boundingRect();
}

void Selection::updateBuffer(qreal zoom)
{
  QPainter imgPainter;
  qreal upscale = 2.0;
  m_buffer = QImage(static_cast<int>(upscale * zoom * width()), static_cast<int>(upscale * zoom * height()), QImage::Format_ARGB32_Premultiplied);
  m_buffer.fill(qRgba(0, 0, 0, 0));
  m_buffer.setAlphaChannel(m_buffer);
  imgPainter.begin(&m_buffer);
  imgPainter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

  imgPainter.translate(-upscale * zoom * m_selectionPolygon.boundingRect().topLeft());
  Page::paint(imgPainter, upscale * zoom);
}

const QImage&Selection::buffer()
{
  return m_buffer;
}

bool Selection::empty()
{
  if (m_elements.size() == 0)
  {
    return true;
  } else {
    return false;
  }
}

void Selection::paint(QPainter &painter, qreal zoom, QRectF region)
{
  QTransform scaleTrans;
  scaleTrans = scaleTrans.scale(zoom, zoom);

  QTransform paintTrans;
  paintTrans.translate(m_selectionPolygon.boundingRect().center().x() * zoom, m_selectionPolygon.boundingRect().center().y() * zoom);
  paintTrans.rotate(m_angle);
  paintTrans.translate(-m_selectionPolygon.boundingRect().center().x() * zoom, -m_selectionPolygon.boundingRect().center().y() * zoom);

  painter.setTransform(paintTrans, true);

  painter.setRenderHint(QPainter::Antialiasing, true);
  painter.drawImage(scaleTrans.map(m_selectionPolygon).boundingRect(), m_buffer, QRectF(m_buffer.rect()));

  QPen pen;
  pen.setStyle(Qt::DashLine);
  pen.setWidth(2);
  pen.setCapStyle(Qt::RoundCap);
  if (MrWriter::polygonIsClockwise(selectionPolygon()) && !m_finalized)
  {
    painter.setBrush(QBrush(QColor(255, 127, 127, 50), Qt::SolidPattern));
  }
  else if (!m_finalized)
  {
    painter.setBrush(QBrush(QColor(127, 255, 127, 50), Qt::SolidPattern));
  }
  else
  {
    painter.setBrush(QBrush(QColor(127, 127, 127, 50), Qt::SolidPattern));
  }
  painter.setPen(pen);
  if (!m_finalized)
  {
    painter.drawPolygon(scaleTrans.map(m_selectionPolygon), Qt::OddEvenFill);
  }
  else
  {
    // draw GrabZones for resizing
    pen.setColor(QColor(127, 127, 127, 255));
    //    pen.setColor(QColor(255,255,255,255));
    pen.setWidthF(0.5);
    painter.setPen(pen);
    QRect brect = scaleTrans.map(m_selectionPolygon).boundingRect().toRect();
    qreal ad = m_ad;
    painter.drawLine(brect.topLeft() - QPointF(0, ad), brect.bottomLeft() + QPointF(0, ad));
    painter.drawLine(brect.topRight() - QPointF(0, ad), brect.bottomRight() + QPointF(0, ad));
    painter.drawLine(brect.topLeft() - QPointF(ad, 0), brect.topRight() + QPointF(ad, 0));
    painter.drawLine(brect.bottomLeft() - QPointF(ad, 0), brect.bottomRight() + QPointF(ad, 0));

    pen.setWidth(2);
    //    pen.setColor(QColor(0,0,0,127));
    painter.setPen(pen);
    QRectF outerRect = scaleTrans.map(m_selectionPolygon).boundingRect().adjusted(-m_ad, -m_ad, m_ad, m_ad);
    // draw bounding rect
    painter.drawRect(outerRect);

    // draw GrabZone for rotating
    pen.setColor(QColor(0, 0, 0, 127));
    pen.setStyle(Qt::SolidLine);
    painter.setPen(pen);
    painter.setBrush(QBrush(QColor(127, 127, 127, 127), Qt::SolidPattern));
    QPointF rotateLineFrom = (outerRect.topRight() + outerRect.topLeft()) / 2.0;
    QPointF rotateLineTo = (outerRect.topRight() + outerRect.topLeft()) / 2.0 - QPointF(0, m_rotateRectCenter - m_rotateRectRadius);
    painter.drawLine(rotateLineFrom, rotateLineTo);
    painter.drawEllipse(rotateLineTo - QPointF(0.0, m_rotateRectRadius), m_rotateRectRadius, m_rotateRectRadius);
  }
  painter.setRenderHint(QPainter::Antialiasing, false);

  painter.setTransform(paintTrans.inverted(), true);
}

void Selection::transform(QTransform transform, size_t pageNum)
{
  m_selectionPolygon = transform.map(m_selectionPolygon);

  qreal sx = transform.m11();
  qreal sy = transform.m22();
  // qreal s = (sx + sy) / 2.0;

  for (size_t i = 0; i < m_elements.size(); ++i)
  {
    m_elements[i]->transform(transform);
  }
  if (transform.determinant() != 1)
  {
    m_x_padding *= sx;
    m_y_padding *= sy;
  }
  if (transform.isRotating())
  {
    m_x_padding = m_padding;
    m_y_padding = m_padding;
  }

  m_angle = 0.0;

  setPageNum(pageNum);
}

void Selection::finalize()
{
  QRectF boundingRect;
  for (size_t i = 0; i < m_elements.size(); ++i)
  {
    boundingRect = boundingRect.united(m_elements[i]->boundingRect());
  }

  if (boundingRect.width() < 10.0 || boundingRect.height() < 10.0)
  {
    boundingRect.adjust(-m_x_padding, -m_y_padding, m_x_padding, m_y_padding);
  }
  m_selectionPolygon = QPolygonF(boundingRect);

  setWidth(boundingRect.width());
  setHeight(boundingRect.height());

  m_angle = 0.0;

  m_finalized = true;
}
}
