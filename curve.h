#ifndef CURVE_H
#define CURVE_H

#include <QObject>
#include <QPainter>
#include <QVector>
#include <QVector2D>

class Curve
{
public:
    enum class dashPattern { SolidLine, DashLine, DashDotLine, DotLine };

    const static QVector<qreal> dashLinePattern;
    const static QVector<qreal> dashDotLinePattern;
    const static QVector<qreal> dotLinePattern;

    Curve();
    QPolygonF points;
    QVector<qreal> pressures;
    dashPattern pattern;
    qreal penWidth;
    QColor color;
};


#endif // CURVE_H
