#ifndef COLORBUTTON_H
#define COLORBUTTON_H

#include <QLabel>
#include <QMouseEvent>

class ColorButton : public QLabel
{
  Q_OBJECT
public:
  explicit ColorButton(QWidget *parent = 0);
  ~ColorButton()
  {
  }

  void setColor(QColor newColor);

  void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
  void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;

signals:
  void clicked();

private slots:

private:
  QColor m_color;
  bool m_leftButtonDown;
};

#endif // COLORBUTTON_H
