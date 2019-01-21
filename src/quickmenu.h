#ifndef QUICKMENU_H
#define QUICKMENU_H

#include <QtWidgets/QWidget>
#include <QMainWindow>

namespace Ui {
class QuickMenu;
}

class QuickMenu : public QWidget
{
  Q_OBJECT

public:
  explicit QuickMenu(QWidget *parent = nullptr);
  ~QuickMenu() override;

  void setupSignalsAndSlots(QMainWindow * mainWindow);

private:
  Ui::QuickMenu *ui;

protected:
  virtual void enterEvent(QEvent * event) override;
  virtual void leaveEvent(QEvent * event) override;
  virtual void changeEvent(QEvent * event) override;

signals:
  void pen();

};

#endif // QUICKMENU_H
