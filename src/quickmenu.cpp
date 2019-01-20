#include "quickmenu.h"
#include "ui_quickmenu.h"
#include <QtDebug>
#include "mainwindow.h"

QuickMenu::QuickMenu(QWidget *parent) : QDialog(parent), ui(new Ui::QuickMenu)
{
  qDebug() << "constr";
  ui->setupUi(this);
  setWindowFlag(Qt::FramelessWindowHint);

  MainWindow *mainWindow = dynamic_cast<MainWindow*>(parent);

  if (mainWindow)
  {
    connect(ui->penButton, SIGNAL(clicked()), mainWindow, SLOT(pen()));
    connect(ui->penButton, SIGNAL(clicked()), this, SLOT(close()));

    connect(ui->rulerButton, SIGNAL(clicked()), mainWindow, SLOT(ruler()));
    connect(ui->rulerButton, SIGNAL(clicked()), this, SLOT(close()));

    connect(ui->circleButton, SIGNAL(clicked()), mainWindow, SLOT(circle()));
    connect(ui->circleButton, SIGNAL(clicked()), this, SLOT(close()));

    connect(ui->eraserButton, SIGNAL(clicked()), mainWindow, SLOT(eraser()));
    connect(ui->eraserButton, SIGNAL(clicked()), this, SLOT(close()));

    connect(ui->strokeEraserButton, SIGNAL(clicked()), mainWindow, SLOT(strokeEraser()));
    connect(ui->strokeEraserButton, SIGNAL(clicked()), this, SLOT(close()));

    connect(ui->rectButton, SIGNAL(clicked()), mainWindow, SLOT(rect()));
    connect(ui->rectButton, SIGNAL(clicked()), this, SLOT(close()));

    connect(ui->selectButton, SIGNAL(clicked()), mainWindow, SLOT(select()));
    connect(ui->selectButton, SIGNAL(clicked()), this, SLOT(close()));

    connect(ui->rectSelectButton, SIGNAL(clicked()), mainWindow, SLOT(rectSelect()));
    connect(ui->rectSelectButton, SIGNAL(clicked()), this, SLOT(close()));

    connect(ui->handButton, SIGNAL(clicked()), mainWindow, SLOT(hand()));
    connect(ui->handButton, SIGNAL(clicked()), this, SLOT(close()));
  }
}

QuickMenu::~QuickMenu()
{
  delete ui;
}


void QuickMenu::enterEvent(QEvent* event)
{
  (void)event;
}

void QuickMenu::leaveEvent(QEvent* event)
{
  (void)event;
  close();
}
