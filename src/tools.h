#ifndef TOOLS_H
#define TOOLS_H

#include <QPolygonF>

namespace MrWriter {

double polygonSignedArea(const QPolygonF &polygon);
bool polygonIsClockwise(const QPolygonF &polygon);

bool polygonLinesIntersect(const QPolygonF &polygonA, const QPolygonF &polygonB);

}

#endif // TOOLS_H
