#ifndef SELECTION_H
#define SELECTION_H

//#include "mrdoc.h"
#include "page.h"

namespace MrDoc
{

class Selection : public Page
{
public:
  Selection();

  void setPageNum(int pageNum);
  int pageNum() const;

  void setSelectionPolygon(QPolygonF selectionPolygon);
  QPolygonF selectionPolygon() const;

  bool containsPoint(QPointF pagePos);

  void appendToSelectionPolygon(QPointF pagePos);

  QRectF boundingRect() const;

  virtual void paint(QPainter &painter, qreal zoom, QRectF region = QRect(0, 0, 0, 0)) override;

  void transform(QTransform transform, int pageNum);

  void finalize();

  void updateBuffer(qreal zoom);

private:
  QImage m_buffer;
  qreal m_ad = 10;

  QPolygonF m_selectionPolygon;

  int m_pageNum;
};
}
#endif // SELECTION_H
