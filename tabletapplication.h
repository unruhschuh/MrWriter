#ifndef TABLETAPPLICATION_H
#define TABLETAPPLICATION_H

#include <QApplication>

#include "selection.h"
//#include "mainwindow.h"
#include <QMainWindow>
#include <QVector>

class TabletApplication : public QApplication
{
public:
  TabletApplication(int &argc, char **argv);

  bool isUsingTablet();

  QVector<QMainWindow *> m_mainWindows;

  MrDoc::Selection m_clipboard;

public slots:
  void exit();

protected:
  bool event(QEvent *event) Q_DECL_OVERRIDE;

private:
  //    bool event(QEvent *event) Q_DECL_OVERRIDE;

  bool m_usingTablet = false;
};

#endif // TABLETAPPLICATION_H
