#include "quickmenu.h"
#include "ui_quickmenu.h"

QuickMenu::QuickMenu(QDialog *parent) :
  QDialog(parent),
  ui(new Ui::QuickMenu)
{
  ui->setupUi(this);
}

QuickMenu::~QuickMenu()
{
  delete ui;
}
