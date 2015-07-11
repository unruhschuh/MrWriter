#include "curve.h"

const QVector<qreal> Curve::solidLinePattern = {1, 0};
const QVector<qreal> Curve::dashLinePattern = {6, 3};
const QVector<qreal> Curve::dashDotLinePattern = {6, 3, 0.5, 3};
const QVector<qreal> Curve::dotLinePattern = {0.5, 3};

Curve::Curve()
{

}

