#ifndef TOOLS_H
#define TOOLS_H

#include <QPolygonF>

namespace MrWriter {

double polygonSignedArea(const QPolygonF &polygon);
bool polygonIsClockwise(const QPolygonF &polygon);

}

#endif // TOOLS_H
