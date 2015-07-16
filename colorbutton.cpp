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
