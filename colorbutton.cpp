#include "colorbutton.h"

ColorButton::ColorButton(QWidget *parent) : QLabel(parent)
{
  m_leftButtonDown = false;
}

void ColorButton::mousePressEvent(QMouseEvent *event)
{
  if (event->button() == Qt::LeftButton)
  {
    m_leftButtonDown = true;
  }
}

void ColorButton::mouseReleaseEvent(QMouseEvent *event)
{
  if (event->button() == Qt::LeftButton && m_leftButtonDown == true && rect().contains(event->pos()))
  {
    emit clicked();
  }
  m_leftButtonDown = false;
}

void ColorButton::setColor(QColor newColor)
{
  m_color = newColor;
  QPixmap pixmap(64, 16);
  pixmap.fill(m_color);
  setPixmap(pixmap);
}
