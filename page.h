#ifndef PAGE_H
#define PAGE_H

#include "stroke.h"

namespace MrDoc
{

class Page
{
public:
  Page();

  qreal width() const;
  qreal height() const;

  void setWidth(qreal width);
  void setHeight(qreal height);

  void setBackgroundColor(QColor backgroundColor);
  QColor backgroundColor(void) const;

  const QRectF &dirtyRect() const;
  void clearDirtyRect();

  bool changeStrokeColor(int strokeNum, QColor color);

  const QVector<Stroke> & strokes();

  QVector<QPair<Stroke, int>> getStrokes(QPolygonF &selectionPolygon);
  QVector<QPair<Stroke, int>> removeStrokes(QPolygonF &selectionPolygon);
  void removeStrokeAt(int i);
  void removeLastStroke();

  void insertStrokes(const QVector<QPair<Stroke, int>> &strokesAndPositions);
  void insertStroke(int position, const Stroke &stroke);

  void appendStroke(const Stroke &stroke);
  void appendStrokes(const QVector<Stroke> &strokes);
  void prependStroke(const Stroke &stroke);

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
  QVector<Stroke> m_strokes;

private:
  QColor m_backgroundColor;

  qreal m_width;  // post script units
  qreal m_height; // post script units


  QRectF m_dirtyRect;
};
}

#endif // PAGE_H
