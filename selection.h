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

  virtual void paint(QPainter &painter, qreal zoom, QRectF region = QRect(0, 0, 0, 0)) override;

  void transform(QTransform transform, int newPageNum);

  void finalize();

  void updateBuffer(qreal zoom);

  QPolygonF selectionPolygon;

  int pageNum;

private:
  QImage buffer;
  qreal ad = 10;
  qreal lastZoom = 0.0;
};
}
#endif // SELECTION_H
