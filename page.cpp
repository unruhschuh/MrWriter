#include "page.h"
#include "mrdoc.h"
#include <QDebug>

namespace MrDoc
{

Page::Page()
{
  // set up standard page (Letter, white background)
  setWidth(595.0);
  setHeight(842.0);
  //    setWidth(600.0);
  //    setHeight(800.0);
  setBackgroundColor(QColor(255, 255, 255));
}

qreal Page::getHeight() const
{
  return height;
}

qreal Page::getWidth() const
{
  return width;
}

void Page::setHeight(qreal newHeight)
{
  if (newHeight > 0)
  {
    height = newHeight;
  }
}

void Page::setWidth(qreal newWidth)
{
  if (newWidth > 0)
  {
    width = newWidth;
  }
}

void Page::paint(QPainter &painter, qreal zoom, QRectF region)
{
  for (Stroke &stroke : m_strokes)
  {
    if (region.isNull() || stroke.boundingRect().intersects(region))
    {
      stroke.paint(painter, zoom);
    }
  }
}

void Page::setBackgroundColor(QColor newBackgroundColor)
{
  backgroundColor = newBackgroundColor;
}

QColor Page::getBackgroundColor()
{
  return backgroundColor;
}

const QRectF &Page::getDirtyRect() const
{
  return dirtyRect;
}

void Page::clearDirtyRect()
{
  dirtyRect = QRectF();
}

bool Page::changeStrokeColor(int strokeNum, QColor color)
{
  if (strokeNum < 0 || strokeNum >= m_strokes.size() || m_strokes.isEmpty())
  {
    return false;
  }
  else
  {
    m_strokes[strokeNum].color = color;
    dirtyRect = dirtyRect.united(m_strokes[strokeNum].boundingRect());
    return true;
  }
}

const QVector<Stroke> &Page::strokes()
{
  return m_strokes;
}

QVector<QPair<Stroke, int>> Page::getStrokes(QPolygonF &selectionPolygon)
{
  QVector<QPair<Stroke, int>> strokesAndPositions;

  for (int i = m_strokes.size() - 1; i >= 0; --i)
  {
    const MrDoc::Stroke &stroke = m_strokes.at(i);
    bool containsStroke = true;
    for (int j = 0; j < stroke.points.size(); ++j)
    {
      if (!selectionPolygon.containsPoint(stroke.points.at(j), Qt::OddEvenFill))
      {
        containsStroke = false;
      }
    }
    if (containsStroke)
    {
      // add selected strokes and positions to return vector
      strokesAndPositions.append(QPair<Stroke, int>(stroke, i));
    }
  }

  return strokesAndPositions;
}

QVector<QPair<Stroke, int>> Page::removeStrokes(QPolygonF &selectionPolygon)
{
  auto removedStrokesAndPositions = getStrokes(selectionPolygon);

  for (auto sAndP : removedStrokesAndPositions)
  {
    removeStrokeAt(sAndP.second);
  }

  return removedStrokesAndPositions;
}

void Page::removeStrokeAt(int i)
{
  dirtyRect = dirtyRect.united(m_strokes[i].boundingRect());
  m_strokes.removeAt(i);
}

void Page::removeLastStroke()
{
  removeStrokeAt(m_strokes.size() - 1);
}

void Page::insertStrokes(const QVector<QPair<Stroke, int>> &strokesAndPositions)
{
  for (int i = strokesAndPositions.size() - 1; i >= 0; --i)
  {
    insertStroke(strokesAndPositions[i].second, strokesAndPositions[i].first);
  }
}

void Page::insertStroke(int position, const Stroke &stroke)
{
  dirtyRect = dirtyRect.united(stroke.boundingRect());
  m_strokes.insert(position, stroke);
}

void Page::appendStroke(const Stroke &stroke)
{
  dirtyRect = dirtyRect.united(stroke.boundingRect());
  m_strokes.append(stroke);
}

void Page::appendStrokes(const QVector<Stroke> &strokes)
{
  for (auto &stroke : strokes)
  {
    m_strokes.append(stroke);
  }
}

void Page::prependStroke(const Stroke &stroke)
{
  dirtyRect = dirtyRect.united(stroke.boundingRect());
  m_strokes.append(stroke);
}
}
