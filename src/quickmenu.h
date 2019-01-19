#ifndef QUICKMENU_H
#define QUICKMENU_H

#include <QtWidgets/QDialog>

namespace Ui {
class QuickMenu;
}

class QuickMenu : public QDialog
{
  Q_OBJECT

public:
  explicit QuickMenu(QWidget *parent = nullptr);
  ~QuickMenu() override;

private:
  Ui::QuickMenu *ui;

protected:
  virtual void enterEvent(QEvent * event) override;
  virtual void leaveEvent(QEvent * event) override;

signals:
  void pen();

};

#endif // QUICKMENU_H
