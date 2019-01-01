#ifndef MRDOC_H
#define MRDOC_H

#include <QColor>
#include <QVector>

namespace MrDoc
{

const QColor black = QColor(0, 0, 0);
const QColor blue = QColor(51, 51, 204);
const QColor red = QColor(255, 0, 0);
const QColor green = QColor(0, 128, 0);
const QColor gray = QColor(128, 128, 128);
const QColor lightblue = QColor(0, 192, 255);
const QColor lightgreen = QColor(0, 255, 0);
const QColor magenta = QColor(255, 0, 255);
const QColor orange = QColor(255, 128, 0);
const QColor yellow = QColor(255, 255, 0);
const QColor white = QColor(255, 255, 255);

const QColor gridColor = QColor(50, 50, 50);

/*
const QColor solarized_base03  = QColor(  0,  43,  54);
const QColor solarized_base02  = QColor(  7,  54,  66);
const QColor solarized_base01  = QColor( 88, 110, 117);
const QColor solarized_base00  = QColor(101, 123, 131);
const QColor solarized_base0   = QColor(131, 148, 150);
const QColor solarized_base1   = QColor(147, 161, 161);
const QColor solarized_base2   = QColor(238, 232, 213);
const QColor solarized_base3   = QColor(253, 246, 227);
const QColor solarized_yellow  = QColor(181, 137,   0);
const QColor solarized_orange  = QColor(203,  75,  22);
const QColor solarized_red     = QColor(220,  50,  47);
const QColor solarized_magenta = QColor(211,  54, 130);
const QColor solarized_violet  = QColor(108, 113, 196);
const QColor solarized_blue    = QColor( 38, 139, 210);
const QColor solarized_cyan    = QColor( 42, 161, 152);
const QColor solarized_green   = QColor(133, 153,   0);

const QColor black = solarized_base01;
const QColor blue = solarized_blue;
const QColor red = solarized_red;
const QColor green = solarized_green;
const QColor gray = solarized_base1;
const QColor lightblue = solarized_violet;
const QColor lightgreen = solarized_cyan;
const QColor magenta = solarized_magenta;
const QColor orange = solarized_orange;
const QColor yellow = solarized_yellow;
const QColor white = QColor(255, 255, 255);
*/


const QVector<QString> standardColorNames = QVector<QString>() << "black"
                                                               << "blue"
                                                               << "red"
                                                               << "green"
                                                               << "gray"
                                                               << "lightblue"
                                                               << "lightgreen"
                                                               << "magenta"
                                                               << "orange"
                                                               << "yellow"
                                                               << "white";

const QVector<QColor> standardColors = QVector<QColor>() << MrDoc::black << MrDoc::blue << MrDoc::red << MrDoc::green << MrDoc::gray << MrDoc::lightblue
                                                         << MrDoc::lightgreen << MrDoc::magenta << MrDoc::orange << MrDoc::yellow << MrDoc::white;

const QVector<qreal> solidLinePattern = {1, 0};
const QVector<qreal> dashLinePattern = {6, 3};
const QVector<qreal> dashDotLinePattern = {6, 3, 0.5, 3};
const QVector<qreal> dotLinePattern = {0.5, 3};
}

#endif // MRDOC_H
