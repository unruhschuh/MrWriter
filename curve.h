#ifndef CURVE_H
#define CURVE_H

#include <QObject>
#include <QPainter>
#include <QVector>
#include <QVector2D>

class Curve
{
public:
    Curve();
    //QVector<QVector2D> points;
    QPolygonF points;
    QVector<qreal> pressures;
    qreal penWidth;
    QColor color;
};

#endif // CURVE_H
