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

Page::Page(const Page & _page)
{
  size_t numElements = _page.m_elements.size();
  for (size_t i = 0; i < numElements; i++)
  {
    m_elements.push_back(_page.m_elements[i]->clone());
  }
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
  for (const auto & element : m_elements)
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

bool Page::changePenWidth(size_t elementNum, qreal penWidth)
{
  if (elementNum >= m_elements.size() || m_elements.empty())
  {
    return false;
  }
  else
  {
    auto stroke = dynamic_cast<Stroke*>(m_elements[elementNum].get());
    if (nullptr != stroke)
    {
      stroke->penWidth = penWidth;
      m_dirtyRect = m_dirtyRect.united(stroke->boundingRect());
      return true;
    }
  }
  return false;
}

bool Page::changeStrokeColor(size_t elementNum, QColor color)
{
  if (elementNum >= m_elements.size() || m_elements.empty())
  {
    return false;
  }
  else
  {
    auto stroke = dynamic_cast<Stroke*>(m_elements[elementNum].get());
    if (nullptr != stroke)
    {
      stroke->color = color;
      m_dirtyRect = m_dirtyRect.united(stroke->boundingRect());
      return true;
    }
  }
  return false;
}

bool Page::changeStrokePattern(size_t elementNum, QVector<qreal> pattern)
{
  if (elementNum >= m_elements.size() || m_elements.empty())
  {
    return false;
  }
  else
  {
    auto stroke = dynamic_cast<Stroke*>(m_elements[elementNum].get());
    if (nullptr != stroke)
    {
      stroke->pattern = pattern;
      m_dirtyRect = m_dirtyRect.united(stroke->boundingRect());
      return true;
    }
  }
  return false;
}

const std::vector<std::unique_ptr<Element>> & Page::elements()
{
  return m_elements;
}

std::vector<QPair<std::unique_ptr<Element>, size_t>> Page::getElements(QPolygonF selectionPolygon)
{
  std::vector<QPair<std::unique_ptr<Element>, size_t>> elementsAndPositions;

  for (unsigned long i = m_elements.size(); i != 0; --i)
  {
    const auto & element = m_elements.at(i-1);
    if (element->containedInPolygon(selectionPolygon))
    {
      // add selected element and positions to return vector
      // elementsAndPositions.append(QPair<std::unique_ptr<Element>, int>(element, i));
      QPair<std::unique_ptr<Element>, size_t> eAndP;
      eAndP.first = std::unique_ptr<Element>(element->clone());
      eAndP.second = i - 1;
      elementsAndPositions.push_back(std::move(eAndP));
    }
  }

  return elementsAndPositions;
}

std::vector<QPair<std::unique_ptr<Element>, size_t>> Page::removeElements(QPolygonF selectionPolygon)
{
  auto removedElementsAndPositions = getElements(selectionPolygon);

  for (auto & eAndP : removedElementsAndPositions)
  {
    removeElementAt(eAndP.second);
  }

  return removedElementsAndPositions;
}

void Page::removeElementAt(size_t i)
{
  m_dirtyRect = m_dirtyRect.united(m_elements[i]->boundingRect());
  //m_elements.removeAt(i);
  m_elements.erase(m_elements.begin() + static_cast<long>(i));
}

void Page::removeLastElement()
{
  removeElementAt(m_elements.size() - 1);
}

void Page::insertElements(std::vector<QPair<std::unique_ptr<Element>, size_t>> & elementsAndPositions)
{
  for (auto i = elementsAndPositions.size(); i != 0; --i)
  {
    insertElement(elementsAndPositions[i-1].second, elementsAndPositions[i-1].first->clone());
  }
}

void Page::insertElement(size_t position, std::unique_ptr<Element> element)
{
  m_dirtyRect = m_dirtyRect.united(element->boundingRect());
  m_elements.insert(m_elements.begin() + static_cast<long>(position), std::move(element));
}

void Page::appendElement(std::unique_ptr<Element> element)
{
  m_dirtyRect = m_dirtyRect.united(element->boundingRect());
  m_elements.push_back(std::move(element));
}

void Page::prependElement(std::unique_ptr<Element> element)
{
  m_dirtyRect = m_dirtyRect.united(element->boundingRect());
  m_elements.insert(m_elements.begin(), std::move(element));
}

void Page::appendElements(std::vector<std::unique_ptr<Element>> & elements)
{
  for (auto &element : elements)
  {
    appendElement(element->clone());
    //appendElement(std::move(element));
  }
}
}
