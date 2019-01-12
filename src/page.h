#ifndef PAGE_H
#define PAGE_H

#include "stroke.h"
#include "element.h"

namespace MrDoc
{

class Page
{
public:
  Page();
  virtual ~Page() { }

  qreal width() const;
  qreal height() const;

  int pixelHeight(const qreal zoom, int devicePixelRatio) const;
  int pixelWidth(const qreal zoom, int devicePixelRatio) const;

  void setWidth(qreal width);
  void setHeight(qreal height);

  void setBackgroundColor(QColor backgroundColor);
  QColor backgroundColor(void) const;

  const QRectF &dirtyRect() const;
  void clearDirtyRect();

  bool changePenWidth(int strokeNum, qreal penWidth);
  bool changeStrokeColor(int strokeNum, QColor color);
  bool changeStrokePattern(int strokeNum, QVector<qreal> pattern);

  const QVector<std::shared_ptr<Element>> & elements();

  QVector<QPair<std::shared_ptr<Element>, int> > getElements(QPolygonF selectionPolygon);
  QVector<QPair<std::shared_ptr<Element>, int> > removeElements(QPolygonF selectionPolygon);
  void removeElementAt(int i);
  void removeLastElement();

  void insertElements(QVector<QPair<std::shared_ptr<Element>, int>> & elementsAndPositions);
  void insertElement(int position, std::shared_ptr<Element> & element);

  void appendElement(const std::shared_ptr<Element> & element);
  void appendElements(QVector<std::shared_ptr<Element>> & elements);
  void prependElement(std::shared_ptr<Element> & element);

  //    virtual void paint(QPainter &painter, qreal zoom);
  /**
   * @brief paint
   * @param painter
   * @param zoom
   * @param region
   */
  virtual void paint(QPainter &painter, qreal zoom, QRectF region = QRect(0, 0, 0, 0));

  //    QVector<Stroke> strokes;

protected:
//  QVector<Stroke> m_strokes;
  QVector<std::shared_ptr<Element>> m_elements;

private:
  QColor m_backgroundColor;

  qreal m_width;  // post script units
  qreal m_height; // post script units

  QRectF m_dirtyRect;
};
}

#endif // PAGE_H
