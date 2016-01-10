#include "selection.h"

#include <iostream>
#include <QDebug>

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

Selection::GrabZone Selection::grabZone(QPointF pagePos)
{
  GrabZone grabZone = GrabZone::None;
  QRectF bRect = m_selectionPolygon.boundingRect();
  QRectF moveRect = bRect;

  QRectF topRect = bRect;
  topRect.setTop(topRect.top() - m_ad);
  topRect.setHeight(m_ad);

  QRectF bottomRect = bRect;
  bottomRect.setTop(bottomRect.bottom());
  bottomRect.setHeight(m_ad);

  QRectF leftRect = bRect;
  leftRect.setLeft(leftRect.left() - m_ad);
  leftRect.setWidth(m_ad);

  QRectF rightRect = bRect;
  rightRect.setLeft(rightRect.right());
  rightRect.setWidth(m_ad);

  QRectF topLeftRect = bRect;
  topLeftRect.setLeft(topLeftRect.left() - m_ad);
  topLeftRect.setTop(topLeftRect.top() - m_ad);
  topLeftRect.setWidth(m_ad);
  topLeftRect.setHeight(m_ad);

  QRectF topRightRect = bRect;
  topRightRect.setLeft(topRightRect.right());
  topRightRect.setTop(topRightRect.top() - m_ad);
  topRightRect.setWidth(m_ad);
  topRightRect.setHeight(m_ad);

  QRectF bottomLeftRect = bRect;
  bottomLeftRect.setLeft(bottomLeftRect.left() - m_ad);
  bottomLeftRect.setTop(bottomLeftRect.bottom());
  bottomLeftRect.setWidth(m_ad);
  bottomLeftRect.setHeight(m_ad);

  QRectF bottomRightRect = bRect;
  bottomRightRect.setLeft(bottomRightRect.right());
  bottomRightRect.setTop(bottomRightRect.bottom());
  bottomRightRect.setWidth(m_ad);
  bottomRightRect.setHeight(m_ad);

  if (moveRect.contains(pagePos))
  {
    grabZone = GrabZone::Move;
  }
  else if (topRect.contains(pagePos))
  {
    qInfo() << "top";
    grabZone = GrabZone::Move;
  }
  else if (bottomRect.contains(pagePos))
  {
    qInfo() << "bottom";
    grabZone = GrabZone::Move;
  }
  else if (leftRect.contains(pagePos))
  {
    qInfo() << "left";
    grabZone = GrabZone::Move;
  }
  else if (rightRect.contains(pagePos))
  {
    qInfo() << "right";
    grabZone = GrabZone::Move;
  }
  else if (topLeftRect.contains(pagePos))
  {
    qInfo() << "topLeft";
    grabZone = GrabZone::Move;
  }
  else if (topRightRect.contains(pagePos))
  {
    qInfo() << "topRight";
    grabZone = GrabZone::Move;
  }
  else if (bottomLeftRect.contains(pagePos))
  {
    qInfo() << "bottomLeft";
    grabZone = GrabZone::Move;
  }
  else if (bottomRightRect.contains(pagePos))
  {
    qInfo() << "bottomRight";
    grabZone = GrabZone::Move;
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
  m_buffer = QImage(zoom * width(), zoom * height(), QImage::Format_ARGB32_Premultiplied);
  m_buffer.fill(qRgba(0, 0, 0, 0));
  m_buffer.setAlphaChannel(m_buffer);
  imgPainter.begin(&m_buffer);
  imgPainter.setRenderHint(QPainter::Antialiasing, true);

  imgPainter.translate(-zoom * m_selectionPolygon.boundingRect().topLeft());
  Page::paint(imgPainter, zoom);
}

void Selection::paint(QPainter &painter, qreal zoom, QRectF region __attribute__((unused)))
{
  painter.drawImage(zoom * m_selectionPolygon.boundingRect().topLeft(), m_buffer);

  painter.setRenderHint(QPainter::Antialiasing, true);
  QPen pen;
  pen.setStyle(Qt::DashLine);
  pen.setWidth(2);
  pen.setCapStyle(Qt::RoundCap);
  pen.setColor(QColor(0, 180, 0, 255));
  //  painter.setBrush(QBrush(QColor(127, 127, 127, 50), Qt::SolidPattern));
  //  painter.setBrush(QBrush(QColor(255, 165, 0, 50), Qt::SolidPattern));
  painter.setBrush(QBrush(QColor(0, 255, 0, 50), Qt::SolidPattern));
  painter.setPen(pen);
  QTransform scaleTrans;
  scaleTrans = scaleTrans.scale(zoom, zoom);
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
  }
  setPageNum(pageNum);
}

void Selection::finalize()
{
  QRectF boundingRect;
  for (int i = 0; i < m_strokes.size(); ++i)
  {
    boundingRect = boundingRect.united(m_strokes[i].boundingRect());
  }

  boundingRect.adjust(-m_ad, -m_ad, m_ad, m_ad);
  m_selectionPolygon = QPolygonF(boundingRect);

  setWidth(boundingRect.width());
  setHeight(boundingRect.height());

  m_finalized = true;
}
}
