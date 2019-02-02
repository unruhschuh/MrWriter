#include "quickmenu.h"
#include "ui_quickmenu.h"
#include <QtDebug>
#include "mainwindow.h"
#include "widget.h"

QuickMenu::QuickMenu(QWidget *parent) : QWidget(parent, Qt::FramelessWindowHint | Qt::Window), ui(new Ui::QuickMenu)
{
  qDebug() << "constr";
  ui->setupUi(this);
  setAttribute(Qt::WA_Hover);
}

QuickMenu::~QuickMenu()
{
  delete ui;
}

void QuickMenu::setupSignalsAndSlots(MainWindow* mainWindow)
{
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

    connect(ui->textButton, SIGNAL(clicked()), mainWindow, SLOT(text()));
    connect(ui->textButton, SIGNAL(clicked()), mainWindow, SLOT(letGoSelection()));
    connect(ui->textButton, SIGNAL(clicked()), this, SLOT(close()));

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

    // other buttons
    connect(ui->fullscreenButton, SIGNAL(clicked()), mainWindow, SLOT(fullscreen()));
    connect(ui->fullscreenButton, SIGNAL(clicked()), this, SLOT(close()));

    // close event
    connect(this, SIGNAL(destroyed()), mainWindow, SLOT(quickmenuClose()));

    QColor currentColor = mainWindow->currentColor();

    ui->blackButton->setChecked(currentColor == MrDoc::black);
    ui->blueButton->setChecked(currentColor == MrDoc::blue);
    ui->redButton->setChecked(currentColor == MrDoc::red);
    ui->greenButton->setChecked(currentColor == MrDoc::green);
    ui->grayButton->setChecked(currentColor == MrDoc::gray);
    ui->lightblueButton->setChecked(currentColor == MrDoc::lightblue);

    Widget::tool currentTool = mainWindow->currentTool();

    ui->penButton->setChecked(currentTool == Widget::tool::PEN);
    ui->rulerButton->setChecked(currentTool == Widget::tool::RULER);
    ui->circleButton->setChecked(currentTool == Widget::tool::CIRCLE);
    ui->rectButton->setChecked(currentTool == Widget::tool::RECT);
    ui->eraserButton->setChecked(currentTool == Widget::tool::ERASER);
    ui->strokeEraserButton->setChecked(currentTool == Widget::tool::STROKE_ERASER);
    ui->selectButton->setChecked(currentTool == Widget::tool::SELECT);
    ui->rectSelectButton->setChecked(currentTool == Widget::tool::RECT_SELECT);
    ui->handButton->setChecked(currentTool == Widget::tool::HAND);

    ui->showGridButton->setChecked(mainWindow->showingGrid());
    ui->snapToGridButton->setChecked(mainWindow->snappingToGrid());

    ui->fullscreenButton->setChecked(mainWindow->isFullScreen());
  }
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

void QuickMenu::changeEvent(QEvent* event)
{
  QWidget::changeEvent(event);
  if (event->type() == QEvent::ActivationChange)
  {
    if(this->isActiveWindow())
    {
      // widget is now active
    }
    else
    {
      // widget is now inactive
      close();
    }
  }
}

