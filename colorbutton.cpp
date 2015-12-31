/*
#####################################################################
Copyright (C) 2015 Thomas Leitz (thomas.leitz@web.de)
#####################################################################

LICENSE:

This file is part of MrWriter.

MrWriter is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License 2.0 as published
by the Free Software Foundation.

MrWriter is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with MrWriter.  If not, see <http://www.gnu.org/licenses/>.
#####################################################################
*/

#include "colorbutton.h"

ColorButton::ColorButton(QWidget *parent) : QLabel(parent)
{
    leftButtonDown = false;
}

void ColorButton::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        leftButtonDown = true;
    }
}

void ColorButton::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && leftButtonDown == true && rect().contains(event->pos()))
    {
        emit clicked();
    }
    leftButtonDown = false;
}

void ColorButton::setColor(QColor newColor)
{
    color = newColor;
    QPixmap pixmap(64, 16);
    pixmap.fill(color);
    setPixmap(pixmap);
}
