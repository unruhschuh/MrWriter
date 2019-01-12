#include "page.h"
#include "mrdoc.h"
#include "tools.h"
#include <QDebug>

#include <memory>

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

int Page::pixelHeight(const qreal zoom, int devicePixelRatio) const
{
  return static_cast<int>(m_height * zoom * devicePixelRatio);
}

int Page::pixelWidth(const qreal zoom, int devicePixelRatio) const
{
  return static_cast<int>(m_width * zoom * devicePixelRatio);
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
  for (std::shared_ptr<Element> element : m_elements)
  {
    if (region.isNull() || element->boundingRect().intersects(region))
    {
      element->paint(painter, zoom);
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

bool Page::changePenWidth(int elementNum, qreal penWidth)
{
  if (elementNum < 0 || elementNum >= m_elements.size() || m_elements.isEmpty())
  {
    return false;
  }
  else
  {
    auto stroke = std::dynamic_pointer_cast<Stroke>(m_elements[elementNum]);
    if (nullptr != stroke)
    {
      stroke->penWidth = penWidth;
      m_dirtyRect = m_dirtyRect.united(stroke->boundingRect());
      return true;
    }
  }
  return false;
}

bool Page::changeStrokeColor(int elementNum, QColor color)
{
  if (elementNum < 0 || elementNum >= m_elements.size() || m_elements.isEmpty())
  {
    return false;
  }
  else
  {
    auto stroke = std::dynamic_pointer_cast<Stroke>(m_elements[elementNum]);
    if (nullptr != stroke)
    {
      stroke->color = color;
      m_dirtyRect = m_dirtyRect.united(stroke->boundingRect());
      return true;
    }
  }
  return false;
}

bool Page::changeStrokePattern(int elementNum, QVector<qreal> pattern)
{
  if (elementNum < 0 || elementNum >= m_elements.size() || m_elements.isEmpty())
  {
    return false;
  }
  else
  {
    auto stroke = std::dynamic_pointer_cast<Stroke>(m_elements[elementNum]);
    if (nullptr != stroke)
    {
      stroke->pattern = pattern;
      m_dirtyRect = m_dirtyRect.united(stroke->boundingRect());
      return true;
    }
  }
  return false;
}

const QVector<std::shared_ptr<Element>> & Page::elements()
{
  return m_elements;
}

QVector<QPair<std::shared_ptr<Element>, int>> Page::getElements(QPolygonF selectionPolygon)
{
  QVector<QPair<std::shared_ptr<Element>, int>> elementsAndPositions;

  for (int i = m_elements.size() - 1; i >= 0; --i)
  {
    const auto & element = m_elements.at(i);
    if (element->containedInPolygon(selectionPolygon))
    {
      // add selected element and positions to return vector
      elementsAndPositions.append(QPair<std::shared_ptr<Element>, int>(element, i));
      // elementsAndPositions.append(QPair<std::shared_ptr<Element>, int>(std::make_shared<Element>(*element), i)); // creates a copy of element!
    }
  }

  return elementsAndPositions;
}

QVector<QPair<std::shared_ptr<Element>, int>> Page::removeElements(QPolygonF selectionPolygon)
{
  auto removedElementsAndPositions = getElements(selectionPolygon);

  for (auto eAndP : removedElementsAndPositions)
  {
    removeElementAt(eAndP.second);
  }

  return removedElementsAndPositions;
}

void Page::removeElementAt(int i)
{
  m_dirtyRect = m_dirtyRect.united(m_elements[i]->boundingRect());
  m_elements.removeAt(i);
}

void Page::removeLastElement()
{
  removeElementAt(m_elements.size() - 1);
}

void Page::insertElements(QVector<QPair<std::shared_ptr<Element>, int>> & elementsAndPositions)
{
  for (int i = elementsAndPositions.size() - 1; i >= 0; --i)
  {
    insertElement(elementsAndPositions[i].second, elementsAndPositions[i].first);
  }
}

void Page::insertElement(int position, std::shared_ptr<Element> & element)
{
  m_dirtyRect = m_dirtyRect.united(element->boundingRect());
  m_elements.insert(position, element);
}

void Page::appendElement(const std::shared_ptr<Element> & element)
{
  m_dirtyRect = m_dirtyRect.united(element->boundingRect());
  m_elements.append(element);
}

void Page::prependElement(std::shared_ptr<Element> & element)
{
  m_dirtyRect = m_dirtyRect.united(element->boundingRect());
  m_elements.prepend(element);
}

void Page::appendElements(QVector<std::shared_ptr<Element>> & elements)
{
  for (auto &element : elements)
  {
    appendElement(element);
  }
}
}
