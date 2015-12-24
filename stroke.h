#ifndef STROKE_H
#define STROKE_H

#include <QObject>
#include <QPainter>
#include <QVector>
#include <QVector2D>

#include "mrdoc.h"

namespace MrDoc {

    struct Stroke
    {
    public:
    //    enum class dashPattern { SolidLine, DashLine, DashDotLine, DotLine };

        const static QVector<qreal> solidLinePattern;
        const static QVector<qreal> dashLinePattern;
        const static QVector<qreal> dashDotLinePattern;
        const static QVector<qreal> dotLinePattern;

        Stroke();
        QPolygonF points;
        QVector<qreal> pressures;
        QVector<qreal> pattern;
        qreal penWidth;
        QColor color;
    };

}

#endif // STROKE_H
