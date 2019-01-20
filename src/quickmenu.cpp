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
    // tool buttons
    connect(ui->penButton, SIGNAL(clicked()), mainWindow, SLOT(pen()));
    connect(ui->penButton, SIGNAL(clicked()), mainWindow, SLOT(letGoSelection()));
    connect(ui->penButton, SIGNAL(clicked()), this, SLOT(close()));

    connect(ui->rulerButton, SIGNAL(clicked()), mainWindow, SLOT(ruler()));
    connect(ui->rulerButton, SIGNAL(clicked()), mainWindow, SLOT(letGoSelection()));
    connect(ui->rulerButton, SIGNAL(clicked()), this, SLOT(close()));

    connect(ui->circleButton, SIGNAL(clicked()), mainWindow, SLOT(circle()));
    connect(ui->circleButton, SIGNAL(clicked()), mainWindow, SLOT(letGoSelection()));
    connect(ui->circleButton, SIGNAL(clicked()), this, SLOT(close()));

    connect(ui->eraserButton, SIGNAL(clicked()), mainWindow, SLOT(eraser()));
    connect(ui->eraserButton, SIGNAL(clicked()), mainWindow, SLOT(letGoSelection()));
    connect(ui->eraserButton, SIGNAL(clicked()), this, SLOT(close()));

    connect(ui->strokeEraserButton, SIGNAL(clicked()), mainWindow, SLOT(strokeEraser()));
    connect(ui->strokeEraserButton, SIGNAL(clicked()), mainWindow, SLOT(letGoSelection()));
    connect(ui->strokeEraserButton, SIGNAL(clicked()), this, SLOT(close()));

    connect(ui->rectButton, SIGNAL(clicked()), mainWindow, SLOT(rect()));
    connect(ui->rectButton, SIGNAL(clicked()), mainWindow, SLOT(letGoSelection()));
    connect(ui->rectButton, SIGNAL(clicked()), this, SLOT(close()));

    connect(ui->selectButton, SIGNAL(clicked()), mainWindow, SLOT(select()));
    connect(ui->selectButton, SIGNAL(clicked()), mainWindow, SLOT(letGoSelection()));
    connect(ui->selectButton, SIGNAL(clicked()), this, SLOT(close()));

    connect(ui->rectSelectButton, SIGNAL(clicked()), mainWindow, SLOT(rectSelect()));
    connect(ui->rectSelectButton, SIGNAL(clicked()), mainWindow, SLOT(letGoSelection()));
    connect(ui->rectSelectButton, SIGNAL(clicked()), this, SLOT(close()));

    connect(ui->handButton, SIGNAL(clicked()), mainWindow, SLOT(hand()));
    connect(ui->handButton, SIGNAL(clicked()), mainWindow, SLOT(letGoSelection()));
    connect(ui->handButton, SIGNAL(clicked()), this, SLOT(close()));

    // color buttons
    connect(ui->blackButton, SIGNAL(clicked()), mainWindow, SLOT(black()));
    connect(ui->blackButton, SIGNAL(clicked()), this, SLOT(close()));

    connect(ui->blueButton, SIGNAL(clicked()), mainWindow, SLOT(blue()));
    connect(ui->blueButton, SIGNAL(clicked()), this, SLOT(close()));

    connect(ui->redButton, SIGNAL(clicked()), mainWindow, SLOT(red()));
    connect(ui->redButton, SIGNAL(clicked()), this, SLOT(close()));

    connect(ui->greenButton, SIGNAL(clicked()), mainWindow, SLOT(green()));
    connect(ui->greenButton, SIGNAL(clicked()), this, SLOT(close()));

    connect(ui->grayButton, SIGNAL(clicked()), mainWindow, SLOT(gray()));
    connect(ui->grayButton, SIGNAL(clicked()), this, SLOT(close()));

    connect(ui->lightblueButton, SIGNAL(clicked()), mainWindow, SLOT(lightblue()));
    connect(ui->lightblueButton, SIGNAL(clicked()), this, SLOT(close()));

    connect(ui->lightgreenButton, SIGNAL(clicked()), mainWindow, SLOT(lightgreen()));
    connect(ui->lightgreenButton, SIGNAL(clicked()), this, SLOT(close()));

    connect(ui->magentaButton, SIGNAL(clicked()), mainWindow, SLOT(magenta()));
    connect(ui->magentaButton, SIGNAL(clicked()), this, SLOT(close()));

    // connect(ui->orangeButton, SIGNAL(clicked()), mainWindow, SLOT(orange()));
    // connect(ui->orangeButton, SIGNAL(clicked()), this, SLOT(close()));

    // connect(ui->yellowButton, SIGNAL(clicked()), mainWindow, SLOT(yellow()));
    // connect(ui->yellowButton, SIGNAL(clicked()), this, SLOT(close()));

    // connect(ui->whiteButton, SIGNAL(clicked()), mainWindow, SLOT(white()));
    // connect(ui->whiteButton, SIGNAL(clicked()), this, SLOT(close()));

    // copy paste cut buttons
    connect(ui->copyButton, SIGNAL(clicked()), mainWindow, SLOT(copy()));
    connect(ui->copyButton, SIGNAL(clicked()), this, SLOT(close()));

    connect(ui->pasteButton, SIGNAL(clicked()), mainWindow, SLOT(paste()));
    connect(ui->pasteButton, SIGNAL(clicked()), this, SLOT(close()));

    connect(ui->cutButton, SIGNAL(clicked()), mainWindow, SLOT(cut()));
    connect(ui->cutButton, SIGNAL(clicked()), this, SLOT(close()));

    connect(ui->deleteButton, SIGNAL(clicked()), mainWindow, SLOT(deleteSlot()));
    connect(ui->deleteButton, SIGNAL(clicked()), this, SLOT(close()));

    // undo redo buttons
    connect(ui->undoButton, SIGNAL(clicked()), mainWindow, SLOT(undo()));
    connect(ui->redoButton, SIGNAL(clicked()), mainWindow, SLOT(redo()));

    // grid buttons
    connect(ui->showGridButton, SIGNAL(clicked()), mainWindow, SLOT(showGrid()));
    connect(ui->snapToGridButton, SIGNAL(clicked()), mainWindow, SLOT(snapToGrid()));
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
