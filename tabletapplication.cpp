#include <QFileOpenEvent>
#include <QMessageBox>
#include <QDebug>
#include "tabletapplication.h"
#include "mainwindow.h"
//#include "mainwindow.h"

TabletApplication::TabletApplication(int &argc, char **argv) : QApplication(argc, argv)
{
  QCoreApplication::setOrganizationName("unruhschuh");
  QCoreApplication::setOrganizationDomain("unruhschuh.com");
  QCoreApplication::setApplicationName("MrWriter");
  QCoreApplication::setApplicationVersion("0.1");
}

bool TabletApplication::event(QEvent *event)
{
  if (event->type() == QEvent::TabletEnterProximity)
  {
    usingTablet = true;
    qInfo() << "Enter Proximity";
  }
  if (event->type() == QEvent::TabletLeaveProximity)
  {
    usingTablet = false;
    qInfo() << "Leave Proximity";
  }

  if (event->type() == QEvent::FileOpen)
  {
    QString fileName = static_cast<QFileOpenEvent *>(event)->file();
    MainWindow *newWindow = new MainWindow();
    QStringList fileNameSplitted = fileName.split(".");
    bool success = false;
    if (fileNameSplitted.last().compare(QString("xoj"), Qt::CaseInsensitive) == 0)
    {
      success = newWindow->loadXOJ(fileName);
    }
    else if (fileNameSplitted.last().compare(QString("moj"), Qt::CaseInsensitive) == 0)
    {
      success = newWindow->loadMOJ(fileName);
    }

    if (success)
    {
      mainWindows.append(newWindow);
      newWindow->show();
    }
    else
    {
      delete newWindow;
      QMessageBox errMsgBox;
      errMsgBox.setText("Couldn't open file");
      errMsgBox.exec();
    }
  }

  return QApplication::event(event);
}

bool TabletApplication::isUsingTablet()
{
  return usingTablet;
}

void TabletApplication::exit()
{
  qInfo() << "Exit";

  qInfo() << mainWindows.length();

  while (mainWindows.length() > 0)
  {
    if (mainWindows.last()->close() == false)
    {
      return;
    }
  }
}
