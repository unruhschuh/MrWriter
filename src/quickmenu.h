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
  explicit QuickMenu(QDialog *parent = nullptr);
  ~QuickMenu();

private:
  Ui::QuickMenu *ui;
};

#endif // QUICKMENU_H
