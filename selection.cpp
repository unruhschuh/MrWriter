#include "selection.h"

#include <iostream>
#include <QDebug>
#include <QtMath>

namespace MrDoc
{

Selection::Selection()
{
  setWidth(10.0);
  setHeight(10.0);
  setBackgroundColor(QColor(255, 255, 255, 0)); // transparent
}

void Selection::setPageNum(int pageNum)
{
  m_pageNum = pageNum;
}

int Selection::pageNum() const
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
  return grabZone;
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
  m_buffer = QImage(upscale * zoom * width(), upscale * zoom * height(), QImage::Format_ARGB32_Premultiplied);
  m_buffer.fill(qRgba(0, 0, 0, 0));
  m_buffer.setAlphaChannel(m_buffer);
  imgPainter.begin(&m_buffer);
  imgPainter.setRenderHint(QPainter::Antialiasing, true);

  imgPainter.translate(-upscale * zoom * m_selectionPolygon.boundingRect().topLeft());
  Page::paint(imgPainter, upscale * zoom);
}

void Selection::paint(QPainter &painter, qreal zoom, QRectF region __attribute__((unused)))
{
  QTransform scaleTrans;
  scaleTrans = scaleTrans.scale(zoom, zoom);

  painter.setRenderHint(QPainter::Antialiasing, true);
  painter.drawImage(scaleTrans.map(m_selectionPolygon).boundingRect(), m_buffer, QRectF(m_buffer.rect()));

  QPen pen;
  pen.setStyle(Qt::DashLine);
  pen.setWidth(2);
  pen.setCapStyle(Qt::RoundCap);
//  pen.setColor(QColor(0, 180, 0, 255));
    painter.setBrush(QBrush(QColor(127, 127, 127, 50), Qt::SolidPattern));
  //  painter.setBrush(QBrush(QColor(255, 165, 0, 50), Qt::SolidPattern));
//  painter.setBrush(QBrush(QColor(0, 255, 0, 50), Qt::SolidPattern));
  painter.setPen(pen);
  if (!m_finalized)
  {
    painter.drawPolygon(scaleTrans.map(m_selectionPolygon), Qt::OddEvenFill);
  }
  else
  {
    painter.drawRect(scaleTrans.map(m_selectionPolygon).boundingRect().adjusted(-m_ad, -m_ad, m_ad, m_ad));

    painter.setPen(pen);
    QRect brect = scaleTrans.map(m_selectionPolygon).boundingRect().toRect();
    qreal ad = m_ad;
    painter.drawLine(brect.topLeft() - QPointF(0, ad), brect.bottomLeft() + QPointF(0, ad));
    painter.drawLine(brect.topRight() - QPointF(0, ad), brect.bottomRight() + QPointF(0, ad));
    painter.drawLine(brect.topLeft() - QPointF(ad, 0), brect.topRight() + QPointF(ad, 0));
    painter.drawLine(brect.bottomLeft() - QPointF(ad, 0), brect.bottomRight() + QPointF(ad, 0));
    painter.setRenderHint(QPainter::Antialiasing, false);
  }
}

void Selection::transform(QTransform transform, int pageNum)
{
  m_selectionPolygon = transform.map(m_selectionPolygon);
  for (int i = 0; i < m_strokes.size(); ++i)
  {
    m_strokes[i].points = transform.map(m_strokes[i].points);
    qreal s = (transform.m11() + transform.m22()) / 2.0;
    m_strokes[i].penWidth = m_strokes[i].penWidth * s;
    qInfo() << s;
  }
  setPageNum(pageNum);
}

void Selection::finalize()
{
  QRectF boundingRect;
  for (int i = 0; i < m_strokes.size(); ++i)
  {
    boundingRect = boundingRect.united(m_strokes[i].points.boundingRect());
  }

//  boundingRect.adjust(-m_ad, -m_ad, m_ad, m_ad);
  m_selectionPolygon = QPolygonF(boundingRect);

  setWidth(boundingRect.width());
  setHeight(boundingRect.height());

  m_finalized = true;
}
}
