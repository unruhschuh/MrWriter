//#include <QApplication>
#include <QCommandLineParser>
#include <QDebug>
#include <QMessageBox>
#include "mainwindow.h"
#include "tabletapplication.h"

int main(int argc, char *argv[])
{
  TabletApplication a(argc, argv);

  QCommandLineParser parser;

  parser.addHelpOption();
  parser.addVersionOption();
  parser.addPositionalArgument("file", QCoreApplication::translate("main", "File to open."));

  parser.process(a);

  const QStringList args = parser.positionalArguments();

  MainWindow *w = new MainWindow();
  a.m_mainWindows.append(w);

  if (args.size() == 1)
  {
    QString fileName = args.at(0);
    QStringList fileNameSplitted = fileName.split(".");
    bool success = false;
    if (fileNameSplitted.last().compare(QString("xoj"), Qt::CaseInsensitive) == 0)
    {
      success = w->loadXOJ(fileName);
    }
    else if (fileNameSplitted.last().compare(QString("moj"), Qt::CaseInsensitive) == 0)
    {
      success = w->loadMOJ(fileName);
    }
    if (!success)
    {
      QMessageBox errMsgBox;
      errMsgBox.setText("Couldn't open file");
      errMsgBox.exec();
    }
  }
  w->show();
  w->updateGUI();

  return a.exec();
}
