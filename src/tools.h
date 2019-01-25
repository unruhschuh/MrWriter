#ifndef TOOLS_H
#define TOOLS_H

#include <QPolygonF>
#include <QTransform>

#define DEBUG_PRINT_EXPR(var) qDebug() << #var << " = " << var << " in " << __FILE__ "/" << __func__ << ":" << __LINE__

namespace MrWriter {

double polygonSignedArea(const QPolygonF &polygon);
bool polygonIsClockwise(const QPolygonF &polygon);

bool polygonLinesIntersect(const QPolygonF &polygonA, const QPolygonF &polygonB);

QTransform reallyScaleTransform(QTransform transform, qreal factor);

QString transformToString(const QTransform & transform);
QTransform stringToTransform(const QString & string);

}

#endif // TOOLS_H
