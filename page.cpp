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

qreal Page::height() const
{
  return m_height;
}

qreal Page::width() const
{
  return m_width;
}

void Page::setHeight(qreal height)
{
  if (height > 0)
  {
    m_height = height;
  }
}

void Page::setWidth(qreal width)
{
  if (width > 0)
  {
    m_width = width;
  }
}

void Page::paint(QPainter &painter, qreal zoom, QRectF region)
{
    painter.drawImage(0,0, m_pdf.scaled(m_width*zoom, m_height*zoom, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    for (Stroke &stroke : m_strokes)
    {
        if (region.isNull() || stroke.boundingRect().intersects(region))
        {
            stroke.paint(painter, zoom);
        }
    }
}

void Page::setBackgroundColor(QColor backgroundColor)
{
  m_backgroundColor = backgroundColor;
}

QColor Page::backgroundColor() const
{
  return m_backgroundColor;
}

const QRectF &Page::dirtyRect() const
{
  return m_dirtyRect;
}

void Page::clearDirtyRect()
{
  m_dirtyRect = QRectF(0.0, 0.0, 0.0, 0.0);
}

bool Page::changePenWidth(int strokeNum, qreal penWidth)
{
  if (strokeNum < 0 || strokeNum >= m_strokes.size() || m_strokes.isEmpty())
  {
    return false;
  }
  else
  {
    m_strokes[strokeNum].penWidth = penWidth;
    m_dirtyRect = m_dirtyRect.united(m_strokes[strokeNum].boundingRect());
    return true;
  }
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
    m_dirtyRect = m_dirtyRect.united(m_strokes[strokeNum].boundingRect());
    return true;
  }
}

bool Page::changeStrokePattern(int strokeNum, QVector<qreal> pattern)
{
  if (strokeNum < 0 || strokeNum >= m_strokes.size() || m_strokes.isEmpty())
  {
    return false;
  }
  else
  {
    m_strokes[strokeNum].pattern = pattern;
    m_dirtyRect = m_dirtyRect.united(m_strokes[strokeNum].boundingRect());
    return true;
  }
}

const QVector<Stroke> &Page::strokes()
{
  return m_strokes;
}

QVector<QPair<Stroke, int>> Page::getStrokes(QPolygonF selectionPolygon)
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

QVector<QPair<Stroke, int>> Page::removeStrokes(QPolygonF selectionPolygon)
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
  m_dirtyRect = m_dirtyRect.united(m_strokes[i].boundingRect());
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
  m_dirtyRect = m_dirtyRect.united(stroke.boundingRect());
  m_strokes.insert(position, stroke);
}

void Page::appendStroke(const Stroke &stroke)
{
  m_dirtyRect = m_dirtyRect.united(stroke.boundingRect());
  m_strokes.append(stroke);
}

void Page::prependStroke(const Stroke &stroke)
{
  m_dirtyRect = m_dirtyRect.united(stroke.boundingRect());
  m_strokes.prepend(stroke);
}

void Page::appendStrokes(const QVector<Stroke> &strokes)
{
  for (auto &stroke : strokes)
  {
    appendStroke(stroke);
  }
}

void Page::setPdf(const QString& path, int pageNum){
    Poppler::Document* doc = Poppler::Document::load(path);
    if (!doc || doc->isLocked()){
        qDebug() << "Couldn't load PDF";
    }
    else{
        Poppler::Page* page = doc->page(pageNum);
        if(page == 0){
            qDebug() << "Couldn't load PDF page";
        }
        else{
            m_pdf = page->renderToImage(72.0*10, 72.0*10, 0,0,int(m_width*10), int(m_height*10));
            pageno = pageNum;
            delete page;
        }
    }
    delete doc;
}
}
