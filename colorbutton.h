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

#ifndef COLORBUTTON_H
#define COLORBUTTON_H

#include <QLabel>
#include <QMouseEvent>


class ColorButton : public QLabel
{
    Q_OBJECT
public:
    explicit ColorButton(QWidget* parent = 0);
    ~ColorButton() {}

    void setColor(QColor newColor);

    void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
    void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;

signals:
    void clicked();

private slots:

private:
    QColor color;
    bool leftButtonDown;

};

#endif // COLORBUTTON_H

