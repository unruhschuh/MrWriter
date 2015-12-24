#include "stroke.h"

namespace MrDoc {
    const QVector<qreal> Stroke::solidLinePattern = {1, 0};
    const QVector<qreal> Stroke::dashLinePattern = {6, 3};
    const QVector<qreal> Stroke::dashDotLinePattern = {6, 3, 0.5, 3};
    const QVector<qreal> Stroke::dotLinePattern = {0.5, 3};

    Stroke::Stroke()
    {

    }
}

