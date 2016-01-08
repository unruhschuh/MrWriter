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
  painter.drawPolygon(scaleTrans.map(m_selectionPolygon), Qt::OddEvenFill);

  // draw edges for grabbing to resize
  /*
  pen.setWidthF(0.5);
  painter.setPen(pen);
  QRect brect = scaleTrans.map(m_selectionPolygon).boundingRect().toRect();
  painter.drawLine(brect.topLeft() + QPointF(m_ad/2, 0), brect.bottomLeft() + QPointF(m_ad/2, 0));
  painter.drawLine(brect.topRight() - QPointF(m_ad/2, 0), brect.bottomRight() - QPointF(m_ad/2, 0));
  painter.drawLine(brect.topLeft() + QPointF(0, m_ad/2), brect.topRight() + QPointF(0, m_ad/2));
  painter.drawLine(brect.bottomLeft() - QPointF(0, m_ad/2), brect.bottomRight() - QPointF(0, m_ad/2));
  painter.setRenderHint(QPainter::Antialiasing, false);
  */
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

  boundingRect = boundingRect.adjusted(-m_ad, -m_ad, m_ad, m_ad);
  m_selectionPolygon = QPolygonF(boundingRect);

  setWidth(boundingRect.width());
  setHeight(boundingRect.height());
}
}
