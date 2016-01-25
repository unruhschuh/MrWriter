#ifndef SELECTION_H
#define SELECTION_H

//#include "mrdoc.h"
#include "page.h"

namespace MrDoc
{

class Selection : public Page
{
public:
  enum class GrabZone
  {
    None,
    Move,
    Top,
    Bottom,
    Left,
    Right,
    TopLeft,
    TopRight,
    BottomLeft,
    BottomRight,
    Rotate
  };

  Selection();

  void setPageNum(int pageNum);
  int pageNum() const;

  void setSelectionPolygon(QPolygonF selectionPolygon);
  QPolygonF selectionPolygon() const;

  bool containsPoint(QPointF pagePos);

  GrabZone grabZone(QPointF pagePos, qreal zoom);

  void setAngle(qreal angle);

  void appendToSelectionPolygon(QPointF pagePos);

  QRectF boundingRect() const;

  virtual void paint(QPainter &painter, qreal zoom, QRectF region = QRect(0, 0, 0, 0)) override;

  void transform(QTransform transform, int pageNum);

  void finalize();

  void updateBuffer(qreal zoom);

private:
  QImage m_buffer;

  qreal m_ad = 10;

  qreal static constexpr m_padding = 10.0;
  qreal m_y_padding = m_padding;
  qreal m_x_padding = m_padding;

  qreal m_angle = 0.0;

  QPolygonF m_selectionPolygon;

  bool m_finalized = false;

  int m_pageNum;

  qreal constexpr static m_rotateRectRadius = 8.0;
  qreal constexpr static m_rotateRectCenter = 20;
};
}
#endif // SELECTION_H
