#include <QScrollArea>
#include <QScrollBar>
#include <QAction>
#include <QFileDialog>
#include <QMenuBar>
#include <QToolBar>
#include <QToolButton>
#include <QMessageBox>
#include <QStatusBar>
#include <QInputDialog>
#include <QSysInfo>
#include <qdebug.h>
#include <QPageSize>
#include <QSettings>
#include <QDateTime>
#include <QFontDialog>
//#include <QWebEngineView>
#include <QDesktopServices>
#include <QBoxLayout>

#include <iostream>
#include <memory>

#include "widget.h"
#include "mrdoc.h"
#include "version.h"
#include "mainwindow.h"
#include "pagesettingsdialog.h"
#include "commands.h"
#include "tabletapplication.h"
#include "quickmenu.h"
#include "ui_quickmenu.h"
#include "settingsdialog.h"
#include "ui_settingsdialog.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
  //    this->resize(1024,768);

  qApp->setAttribute(Qt::AA_UseHighDpiPixmaps);

  mainWidget = new Widget(this);
  connect(mainWidget, SIGNAL(select()), this, SLOT(select()));
  connect(mainWidget, SIGNAL(rectSelect()), this, SLOT(rectSelect()));
  connect(mainWidget, SIGNAL(pen()), this, SLOT(pen()));
  connect(mainWidget, SIGNAL(ruler()), this, SLOT(ruler()));
  connect(mainWidget, SIGNAL(circle()), this, SLOT(circle()));
  connect(mainWidget, SIGNAL(rect()), this, SLOT(rect()));
  connect(mainWidget, SIGNAL(eraser()), this, SLOT(eraser()));
  connect(mainWidget, SIGNAL(strokeEraser()), this, SLOT(strokeEraser()));
  connect(mainWidget, SIGNAL(hand()), this, SLOT(hand()));

  connect(mainWidget, SIGNAL(updateGUI()), this, SLOT(updateGUI()));

  connect(mainWidget, SIGNAL(modified()), this, SLOT(modified()));

  connect(mainWidget, SIGNAL(quickmenu()), this, SLOT(quickmenu()));

  scrollArea = new QScrollArea(this);
  scrollArea->setWidget(mainWidget);
  scrollArea->setAlignment(Qt::AlignHCenter);

  loadMyGeometry();
  setCentralWidget(scrollArea);

  mainWidget->m_scrollArea = scrollArea;

  QWidget *sep1 = new QWidget();
  sep1->setFixedWidth(10);
  QWidget *sep2 = new QWidget();
  sep2->setFixedWidth(10);
  QWidget *sep3 = new QWidget();
  sep3->setFixedWidth(10);

  statusBar()->addPermanentWidget(&statusStatus);
  statusBar()->addPermanentWidget(sep1);
  statusBar()->addPermanentWidget(&colorStatus);
  statusBar()->addPermanentWidget(sep2);
  statusBar()->addPermanentWidget(&penWidthStatus);
  statusBar()->addPermanentWidget(sep3);
  statusBar()->addPermanentWidget(&pageStatus);

  createActions();
  createMenus();
  createToolBars();

  updateGUI();
}

MainWindow::~MainWindow()
{
  // delete ui;
}

void MainWindow::setTitle()
{
  QString docName;
  if (mainWidget->m_currentDocument.docName().isEmpty())
  {
    docName = tr("untitled");
  }
  else
  {
    docName = mainWidget->m_currentDocument.docName();
  }
  QString title = PRODUCT_NAME;
  title.append(" - ");
  title.append(docName);
  title.append("[*]");
  setWindowTitle(title);
}

void MainWindow::createActions()
{
  newWindowAct = new QAction(tr("New &Window"), this);
  newWindowAct->setShortcut(QKeySequence(Qt::Modifier::CTRL + Qt::Modifier::SHIFT + Qt::Key_N));
  newWindowAct->setStatusTip(tr("New Window"));
  connect(newWindowAct, SIGNAL(triggered()), this, SLOT(newWindow()));
  this->addAction(newWindowAct);

  cloneWindowAct = new QAction(tr("Clone Window"), this);
  cloneWindowAct->setShortcut(QKeySequence(Qt::Modifier::CTRL + Qt::Modifier::SHIFT + Qt::Key_C));
  cloneWindowAct->setStatusTip(tr("Clone Window"));
  connect(cloneWindowAct, SIGNAL(triggered()), this, SLOT(cloneWindow()));
  this->addAction(cloneWindowAct);

  closeWindowAct = new QAction(tr("Close Window"), this);
  closeWindowAct->setShortcut(QKeySequence(QKeySequence::Close));
  closeWindowAct->setStatusTip(tr("Close Window"));
  connect(closeWindowAct, SIGNAL(triggered()), this, SLOT(close()));
  this->addAction(closeWindowAct);

  newFileAct = new QAction(QIcon(":/images/newIcon.png"), tr("&New file"), this);
  newFileAct->setShortcuts(QKeySequence::New);
  newFileAct->setStatusTip(tr("New File"));
  connect(newFileAct, SIGNAL(triggered()), this, SLOT(newFile()));
  this->addAction(newFileAct); // add to make shortcut work if menubar is hidden

  openFileAct = new QAction(QIcon(":/images/openIcon.png"), tr("&Open file"), this);
  openFileAct->setShortcuts(QKeySequence::Open);
  openFileAct->setStatusTip(tr("Open File"));
  connect(openFileAct, SIGNAL(triggered()), this, SLOT(openFile()));
  this->addAction(openFileAct);

  saveFileAct = new QAction(QIcon(":/images/saveIcon.png"), tr("&Save file"), this);
  saveFileAct->setShortcuts(QKeySequence::Save);
  saveFileAct->setStatusTip(tr("Save File"));
  connect(saveFileAct, SIGNAL(triggered()), this, SLOT(saveFile()));
  this->addAction(saveFileAct);

  saveFileAsAct = new QAction(tr("&Save file as"), this);
  saveFileAsAct->setShortcuts(QKeySequence::SaveAs);
  saveFileAsAct->setStatusTip(tr("Save File as"));
  connect(saveFileAsAct, SIGNAL(triggered()), this, SLOT(saveFileAs()));
  this->addAction(saveFileAsAct);

  exportPDFAct = new QAction(QIcon(":/images/savePDFIcon.png"), tr("Export PDF"), this);
  exportPDFAct->setShortcut(QKeySequence(Qt::Modifier::CTRL + Qt::Key_E));
  exportPDFAct->setStatusTip(tr("Export PDF"));
  connect(exportPDFAct, SIGNAL(triggered()), this, SLOT(exportPDF()));

  importXOJAct = new QAction(tr("Import Xournal File"), this);
  importXOJAct->setStatusTip(tr("Import Xournal File"));
  connect(importXOJAct, SIGNAL(triggered()), this, SLOT(importXOJ()));

  exportXOJAct = new QAction(tr("Export Xournal File"), this);
  exportXOJAct->setStatusTip(tr("Export Xournal File"));
  connect(exportXOJAct, SIGNAL(triggered()), this, SLOT(exportXOJ()));

  exitAct = new QAction(tr("E&xit"), this);
  exitAct->setShortcuts(QKeySequence::Quit);
  exitAct->setStatusTip(tr("Exit MrWriter"));
  connect(exitAct, SIGNAL(triggered()), this, SLOT(exit()));
  this->addAction(exitAct); // add to make shortcut work if menubar is hidden

  undoAct = mainWidget->m_undoStack.createUndoAction(this);
  undoAct->setIcon(QIcon(":/images/undoIcon.png"));
  undoAct->setShortcut(QKeySequence::Undo);
  undoAct->setStatusTip(tr("Undo"));
  undoAct->disconnect(SIGNAL(triggered()));
  connect(undoAct, SIGNAL(triggered()), mainWidget, SLOT(undo()));
  //    disconnect(undoAct, SIGNAL(triggered()), mainWidget->undoStack, SLOT(undo()));
  this->addAction(undoAct); // add to make shortcut work if menubar is hidden

  redoAct = mainWidget->m_undoStack.createRedoAction(this);
  redoAct->setIcon(QIcon(":/images/redoIcon.png"));
  redoAct->setShortcut(QKeySequence::Redo);
  redoAct->setStatusTip(tr("Redo"));
  redoAct->disconnect(SIGNAL(triggered()));
  connect(redoAct, SIGNAL(triggered()), mainWidget, SLOT(redo()));
  //    disconnect(redoAct, SIGNAL(triggered()), mainWidget->undoStack, SLOT(redo()));
  this->addAction(redoAct); // add to make shortcut work if menubar is hidden

  selectAllAct = new QAction(tr("Select &All"), this);
  selectAllAct->setShortcut(QKeySequence(Qt::Modifier::CTRL + Qt::Key_A));
  selectAllAct->setToolTip(tr("Select All"));
  connect(selectAllAct, SIGNAL(triggered()), mainWidget, SLOT(selectAll()));
  this->addAction(selectAllAct);

  copyAct = new QAction(QIcon(":/images/copyIcon.png"), tr("&Copy"), this);
  copyAct->setShortcuts(QKeySequence::Copy);
  copyAct->setStatusTip(tr("Copy"));
  connect(copyAct, SIGNAL(triggered()), mainWidget, SLOT(copy()));
  this->addAction(copyAct); // add to make shortcut work if menubar is hidden

  pasteAct = new QAction(QIcon(":/images/pasteIcon.png"), tr("&Paste"), this);
  pasteAct->setShortcuts(QKeySequence::Paste);
  pasteAct->setStatusTip(tr("Paste"));
  connect(pasteAct, SIGNAL(triggered()), mainWidget, SLOT(paste()));
  this->addAction(pasteAct); // add to make shortcut work if menubar is hidden

  pasteImageAct = new QAction(tr("Paste &Image"), this);
  pasteImageAct->setStatusTip(tr("Paste"));
  connect(pasteImageAct, SIGNAL(triggered()), mainWidget, SLOT(pasteImage()));

  pasteTextAct = new QAction(tr("Paste &Text"), this);
  pasteTextAct->setStatusTip(tr("Paste"));
  connect(pasteTextAct, SIGNAL(triggered()), mainWidget, SLOT(pasteText()));

  cutAct = new QAction(QIcon(":/images/cutIcon.png"), tr("Cut"), this);
  cutAct->setShortcuts(QKeySequence::Cut);
  cutAct->setStatusTip(tr("Cut"));
  connect(cutAct, SIGNAL(triggered()), mainWidget, SLOT(cut()));
  this->addAction(cutAct); // add to make shortcut work if menubar is hidden

  deleteAct = new QAction(QIcon(":/images/deleteIcon_48.png"), tr("Delete"), this);
  deleteAct->setShortcut(QKeySequence::Delete);
  deleteAct->setStatusTip(tr("Delete"));
  connect(deleteAct, SIGNAL(triggered()), mainWidget, SLOT(deleteSlot()));
  this->addAction(deleteAct); // add to make shortdelete work if menubar is hidden

  toTheBackAct = new QAction(tr("To the back"), this);
  toTheBackAct->setStatusTip(tr("To the back"));
  connect(toTheBackAct, SIGNAL(triggered()), mainWidget, SLOT(toTheBack()));

  zoomInAct = new QAction(QIcon(":/images/zoomInIcon.png"), tr("Zoom &In"), this);
  zoomInAct->setShortcut(QKeySequence::ZoomIn);
  zoomInAct->setStatusTip(tr("Zoom In"));
  connect(zoomInAct, SIGNAL(triggered()), this, SLOT(zoomIn()));
  this->addAction(zoomInAct); // add to make shortcut work if menubar is hidden

  zoomOutAct = new QAction(QIcon(":/images/zoomOutIcon.png"), tr("Zoom &Out"), this); // QAction(QIcon(":/images/new.png"), tr("&New"), this);
  zoomOutAct->setShortcut(QKeySequence::ZoomOut);
  zoomOutAct->setStatusTip(tr("Zoom Out"));
  connect(zoomOutAct, SIGNAL(triggered()), this, SLOT(zoomOut()));
  this->addAction(zoomOutAct); // add to make shortcut work if menubar is hidden

  zoomFitWidthAct = new QAction(QIcon(":/images/zoomFitWidthIcon.png"), tr("Zoom to fit width"), this); // QAction(QIcon(":/images/new.png"), tr("&New"), this);
  zoomFitWidthAct->setStatusTip(tr("Zoom to fit width"));
  zoomFitWidthAct->setShortcut(QKeySequence(Qt::Key_Z));
  connect(zoomFitWidthAct, SIGNAL(triggered()), this, SLOT(zoomFitWidth()));
  this->addAction(zoomFitWidthAct); // add to make shortcut work if menubar is hidden

  zoomFitHeightAct =
      new QAction(QIcon(":/images/zoomFitHeightIcon.png"), tr("Zoom to fit Height"), this); // QAction(QIcon(":/images/new.png"), tr("&New"), this);
  zoomFitHeightAct->setStatusTip(tr("Zoom to fit Height"));
  zoomFitHeightAct->setShortcut(QKeySequence(Qt::Key_X));
  connect(zoomFitHeightAct, SIGNAL(triggered()), this, SLOT(zoomFitHeight()));
  this->addAction(zoomFitHeightAct); // add to make shortcut work if menubar is hidden

  pageFirstAct = new QAction(QIcon(":/images/pageFirstIcon.png"), tr("First Page"), this);
  pageFirstAct->setStatusTip(tr("First Page"));
  connect(pageFirstAct, SIGNAL(triggered()), mainWidget, SLOT(pageFirst()));

  pageLastAct = new QAction(QIcon(":/images/pageLastIcon.png"), tr("Last Page"), this);
  pageLastAct->setStatusTip(tr("Last Page"));
  connect(pageLastAct, SIGNAL(triggered()), mainWidget, SLOT(pageLast()));

  pageUpAct = new QAction(QIcon(":/images/pageUpIcon.png"), tr("Page &Up"), this);
  pageUpAct->setStatusTip(tr("Page Up"));
  pageUpAct->setShortcut(QKeySequence(Qt::Key_Up));
  connect(pageUpAct, SIGNAL(triggered()), mainWidget, SLOT(pageUp()));
  this->addAction(pageUpAct);

  pageDownAct = new QAction(QIcon(":/images/pageDownIcon.png"), tr("Page &Down"), this);
  pageDownAct->setStatusTip(tr("Page Down"));
  pageDownAct->setShortcut(QKeySequence(Qt::Key_Down));
  connect(pageDownAct, SIGNAL(triggered()), mainWidget, SLOT(pageDown()));
  this->addAction(pageDownAct);

  pageAddBeforeAct = new QAction(tr("New Page &Before"), this);
  pageAddBeforeAct->setStatusTip(tr("New Page before current Page"));
  connect(pageAddBeforeAct, SIGNAL(triggered()), mainWidget, SLOT(pageAddBefore()));

  pageAddAfterAct = new QAction(tr("New Page &After"), this);
  pageAddAfterAct->setStatusTip(tr("New Page after current Page"));
  connect(pageAddAfterAct, SIGNAL(triggered()), mainWidget, SLOT(pageAddAfter()));

  pageAddEndAct = new QAction(tr("New Page at &End"), this);
  pageAddEndAct->setStatusTip(tr("New Page after last Page"));
  connect(pageAddEndAct, SIGNAL(triggered()), mainWidget, SLOT(pageAddEnd()));

  pageAddBeginningAct = new QAction(tr("New Page at Beginning"), this);
  pageAddBeginningAct->setStatusTip(tr("New Page before first Page"));
  connect(pageAddBeginningAct, SIGNAL(triggered()), mainWidget, SLOT(pageAddBeginning()));

  pageRemoveAct = new QAction(tr("Remove Current Page"), this);
  pageRemoveAct->setStatusTip(tr("Remove Current Page"));
  connect(pageRemoveAct, SIGNAL(triggered()), mainWidget, SLOT(pageRemove()));

  pageSettingsAct = new QAction(tr("Page Settings"), this);
  pageSettingsAct->setStatusTip(tr("Page Settings"));
  connect(pageSettingsAct, SIGNAL(triggered()), this, SLOT(pageSettings()));

  settingsAct = new QAction(tr("Settings"), this);
  settingsAct->setStatusTip(tr("Settings"));
  connect(settingsAct, SIGNAL(triggered()), this, SLOT(settings()));

  saveMyStateAct = new QAction(tr("Save window state"), this);
  saveMyStateAct->setStatusTip(tr("Save window state"));
  connect(saveMyStateAct, SIGNAL(triggered()), this, SLOT(saveMyState()));

  loadMyStateAct = new QAction(tr("Load window state"), this);
  loadMyStateAct->setStatusTip(tr("Load window state"));
  connect(loadMyStateAct, SIGNAL(triggered()), this, SLOT(loadMyState()));

  penAct = new QAction(QIcon(":/images/penIcon.png"), tr("Pen"), this);
  penAct->setStatusTip(tr("Pen Tool"));
  penAct->setShortcut(QKeySequence(Qt::Key_1));
  penAct->setCheckable(true);
  penAct->setChecked(true);
  connect(penAct, SIGNAL(triggered()), this, SLOT(pen()));
  this->addAction(penAct); // add to make shortcut work if menubar is hidden


  rulerAct = new QAction(QIcon(":/images/rulerIcon.png"), tr("Ruler"), this);
  rulerAct->setStatusTip(tr("Ruler Tool"));
  rulerAct->setShortcut(QKeySequence(Qt::Key_2));
  rulerAct->setCheckable(true);
  rulerAct->setChecked(false);
  connect(rulerAct, SIGNAL(triggered()), this, SLOT(ruler()));
  this->addAction(rulerAct); // add to make shortcut work if menubar is hidden

  circleAct = new QAction(QIcon(":/images/circleIcon.png"), tr("Circle"), this);
  circleAct->setStatusTip(tr("Circle Tool"));
  circleAct->setShortcut(QKeySequence(Qt::Key_3));
  circleAct->setCheckable(true);
  circleAct->setChecked(false);
  connect(circleAct, SIGNAL(triggered()), this, SLOT(circle()));
  this->addAction(circleAct); // add to make shortcut work if menubar is hidden

  rectAct = new QAction(QIcon(":/images/rectIcon_48.png"), tr("Rectangle"), this);
  rectAct->setStatusTip(tr("Rectangle Tool"));
  rectAct->setShortcut(QKeySequence(Qt::Key_4));
  rectAct->setCheckable(true);
  rectAct->setChecked(false);
  connect(rectAct, SIGNAL(triggered()), this, SLOT(rect()));
  this->addAction(rectAct); // add to make shortcut work if menubar is hidden

  textAct = new QAction(QIcon(":/images/textIcon_48.png"), tr("Text"), this);
  textAct->setStatusTip(tr("Text Tool"));
  textAct->setCheckable(true);
  textAct->setChecked(false);
  connect(textAct, SIGNAL(triggered()), this, SLOT(text()));

  fontAct = new QAction(tr("Select Font"), this);
  fontAct->setStatusTip(tr("Select Font"));
  connect(fontAct, SIGNAL(triggered()), this, SLOT(font()));

  eraserAct = new QAction(QIcon(":/images/eraserIcon.png"), tr("Eraser"), this);
  eraserAct->setStatusTip(tr("Eraser Tool"));
  eraserAct->setShortcut(QKeySequence(Qt::Key_5));
  eraserAct->setCheckable(true);
  connect(eraserAct, SIGNAL(triggered()), this, SLOT(eraser()));
  this->addAction(eraserAct); // add to make shortcut work if menubar is hidden

  strokeEraserAct = new QAction(QIcon(":/images/strokeEraserIcon.png"), tr("Stroke Eraser"), this);
  strokeEraserAct->setStatusTip(tr("Stroke Eraser Tool"));
  strokeEraserAct->setShortcut(QKeySequence(Qt::Modifier::SHIFT + Qt::Key_5));
  strokeEraserAct->setCheckable(true);
  connect(strokeEraserAct, SIGNAL(triggered()), this, SLOT(strokeEraser()));

  selectAct = new QAction(QIcon(":/images/selectIcon.png"), tr("Select"), this);
  selectAct->setStatusTip(tr("Select Tool"));
  selectAct->setShortcut(QKeySequence(Qt::Key_6));
  selectAct->setCheckable(true);
  connect(selectAct, SIGNAL(triggered()), this, SLOT(select()));
  this->addAction(selectAct); // add to make shortcut work if menubar is hidden

  rectSelectAct = new QAction(QIcon(":/images/rectSelectIcon_48.png"), tr("Rectangular Select"), this);
  rectSelectAct->setStatusTip(tr("Rectangular Select Tool"));
  rectSelectAct->setShortcut(QKeySequence(Qt::Modifier::SHIFT + Qt::Key_6));
  rectSelectAct->setCheckable(true);
  connect(rectSelectAct, SIGNAL(triggered()), this, SLOT(rectSelect()));
  this->addAction(rectSelectAct); // add to make shortcut work if menubar is hidden

  handAct = new QAction(QIcon(":/images/handIcon.png"), tr("Hand"), this);
  handAct->setStatusTip(tr("Hand Tool"));
  handAct->setShortcut(QKeySequence(Qt::Key_7));
  handAct->setCheckable(true);
  connect(handAct, SIGNAL(triggered()), this, SLOT(hand()));
  this->addAction(handAct); // add to make shortcut work if menubar is hidden

  solidPatternAct = new QAction(QIcon(":/images/solidPatternIcon.png"), tr("solid line"), this);
  solidPatternAct->setStatusTip(tr("solid line"));
  solidPatternAct->setCheckable(true);
  connect(solidPatternAct, SIGNAL(triggered()), mainWidget, SLOT(solidPattern()));

  dashPatternAct = new QAction(QIcon(":/images/dashPatternIcon.png"), tr("dash line"), this);
  dashPatternAct->setStatusTip(tr("dash line"));
  dashPatternAct->setCheckable(true);
  connect(dashPatternAct, SIGNAL(triggered()), mainWidget, SLOT(dashPattern()));

  dashDotPatternAct = new QAction(QIcon(":/images/dashDotPatternIcon.png"), tr("dash dot line"), this);
  dashDotPatternAct->setStatusTip(tr("dash dot line"));
  dashDotPatternAct->setCheckable(true);
  connect(dashDotPatternAct, SIGNAL(triggered()), mainWidget, SLOT(dashDotPattern()));

  dotPatternAct = new QAction(QIcon(":/images/dotPatternIcon.png"), tr("dot line"), this);
  dotPatternAct->setStatusTip(tr("dot line"));
  dotPatternAct->setCheckable(true);
  connect(dotPatternAct, SIGNAL(triggered()), mainWidget, SLOT(dotPattern()));

  veryFinePenWidthAct = new QAction(QIcon(":/images/veryFinePenWidthIcon.png"), tr("Very Fine"), this);
  veryFinePenWidthAct->setStatusTip(tr("Very Fine Pen Width"));
  veryFinePenWidthAct->setShortcut(QKeySequence(Qt::Modifier::CTRL + Qt::Key_1));
  veryFinePenWidthAct->setCheckable(true);
  veryFinePenWidthAct->setChecked(false);
  connect(veryFinePenWidthAct, SIGNAL(triggered()), mainWidget, SLOT(veryFine()));

  finePenWidthAct = new QAction(QIcon(":/images/finePenWidthIcon.png"), tr("Fine"), this);
  finePenWidthAct->setStatusTip(tr("Fine Pen Width"));
  finePenWidthAct->setShortcut(QKeySequence(Qt::Modifier::CTRL + Qt::Key_2));
  finePenWidthAct->setCheckable(true);
  finePenWidthAct->setChecked(false);
  connect(finePenWidthAct, SIGNAL(triggered()), mainWidget, SLOT(fine()));

  mediumPenWidthAct = new QAction(QIcon(":/images/mediumPenWidthIcon.png"), tr("Medium"), this);
  mediumPenWidthAct->setStatusTip(tr("Medium Pen Width"));
  mediumPenWidthAct->setShortcut(QKeySequence(Qt::Modifier::CTRL + Qt::Key_3));
  mediumPenWidthAct->setCheckable(true);
  mediumPenWidthAct->setChecked(false);
  connect(mediumPenWidthAct, SIGNAL(triggered()), mainWidget, SLOT(medium()));

  thickPenWidthAct = new QAction(QIcon(":/images/thickPenWidthIcon.png"), tr("Thick"), this);
  thickPenWidthAct->setStatusTip(tr("Thick Pen Width"));
  thickPenWidthAct->setShortcut(QKeySequence(Qt::Modifier::CTRL + Qt::Key_4));
  thickPenWidthAct->setCheckable(true);
  thickPenWidthAct->setChecked(false);
  connect(thickPenWidthAct, SIGNAL(triggered()), mainWidget, SLOT(thick()));

  veryThickPenWidthAct = new QAction(QIcon(":/images/veryThickPenWidthIcon.png"), tr("Very Thick"), this);
  veryThickPenWidthAct->setStatusTip(tr("Very Thick Pen Width"));
  veryThickPenWidthAct->setShortcut(QKeySequence(Qt::Modifier::CTRL + Qt::Key_5));
  veryThickPenWidthAct->setCheckable(true);
  veryThickPenWidthAct->setChecked(false);
  connect(veryThickPenWidthAct, SIGNAL(triggered()), mainWidget, SLOT(veryThick()));

  pencilIconAct = new QAction(QIcon(":/images/penCursor3.png"), tr("Pencil Cursor"), this);
  pencilIconAct->setStatusTip(tr("Pencil Cursor"));
  pencilIconAct->setCheckable(true);
  pencilIconAct->setChecked(false);
  connect(pencilIconAct, SIGNAL(triggered()), mainWidget, SLOT(setPencilCursorIcon()));

  dotIconAct = new QAction(QIcon(":/images/dotCursor.png"), tr("Dot Cursor"), this);
  dotIconAct->setStatusTip(tr("Dot Cursor"));
  dotIconAct->setCheckable(true);
  dotIconAct->setChecked(false);
  connect(dotIconAct, SIGNAL(triggered()), mainWidget, SLOT(setDotCursorIcon()));

  toolbarAct = new QAction(tr("show Toolbar"), this);
  toolbarAct->setShortcut(QKeySequence(Qt::Key_T));
  toolbarAct->setCheckable(true);
  toolbarAct->setChecked(true);
  connect(toolbarAct, SIGNAL(triggered()), this, SLOT(toolbar()));
  this->addAction(toolbarAct); // add to make shortcut work if menubar is hidden

  statusbarAct = new QAction(tr("show Statusbar"), this);
  statusbarAct->setShortcut(QKeySequence(Qt::Key_S));
  statusbarAct->setCheckable(true);
  statusbarAct->setChecked(true);
  connect(statusbarAct, SIGNAL(triggered()), this, SLOT(statusbar()));
  this->addAction(statusbarAct); // add to make shortcut work if menubar is hidden

  fullscreenAct = new QAction(QIcon(":/images/fullscreenIcon.png"), tr("show fullscreen"), this);
  fullscreenAct->setShortcut(QKeySequence(Qt::Key_F));
  fullscreenAct->setCheckable(true);
  fullscreenAct->setChecked(false);
  connect(fullscreenAct, SIGNAL(triggered()), this, SLOT(fullscreen()));
  this->addAction(fullscreenAct); // add to make shortcut work if menubar is hidden

  maximizeAct = new QAction(tr("Maximize"), this);
  maximizeAct->setShortcut(QKeySequence(Qt::Key_M));
  connect(maximizeAct, SIGNAL(triggered()), this, SLOT(maximize()));
  this->addAction(maximizeAct);

  // grid actions
  showGridAct = new QAction(QIcon(":/images/gridIcon_48.png"), tr("show grid"), this);
  showGridAct->setShortcut(QKeySequence(Qt::Key_G));
  showGridAct->setCheckable(true);
  showGridAct->setChecked(false);
  connect(showGridAct, SIGNAL(triggered()), this, SLOT(showGrid()));
  this->addAction(showGridAct); // add to make shortcut work if menubar is hidden

  snapToGridAct = new QAction(QIcon(":/images/snapToGridIcon_48.png"), tr("snap to grid"), this);
  snapToGridAct->setShortcut(QKeySequence(Qt::SHIFT + Qt::Key_G));
  snapToGridAct->setCheckable(true);
  snapToGridAct->setChecked(false);
  connect(snapToGridAct, SIGNAL(triggered()), this, SLOT(snapToGrid()));
  this->addAction(snapToGridAct); // add to make shortcut work if menubar is hidden

  // colorActions
  blackAct = new QAction(QIcon(":/images/blackIcon.png"), tr("black"), this);
  blackAct->setShortcut(QKeySequence(Qt::Key_Q));
  blackAct->setStatusTip(tr("black"));
  blackAct->setCheckable(true);
  blackAct->setChecked(true);
  connect(blackAct, SIGNAL(triggered()), this, SLOT(black()));
  this->addAction(blackAct); // add to make shortcut work if menubar is hidden

  redAct = new QAction(QIcon(":/images/redIcon.png"), tr("red"), this);
  redAct->setShortcut(QKeySequence(Qt::Key_W));
  redAct->setStatusTip(tr("red"));
  redAct->setCheckable(true);
  connect(redAct, SIGNAL(triggered()), this, SLOT(red()));
  this->addAction(redAct); // add to make shortcut work if menubar is hidden

  greenAct = new QAction(QIcon(":/images/greenIcon.png"), tr("green"), this);
  greenAct->setShortcut(QKeySequence(Qt::Key_E));
  greenAct->setStatusTip(tr("green"));
  greenAct->setCheckable(true);
  connect(greenAct, SIGNAL(triggered()), this, SLOT(green()));
  this->addAction(greenAct); // add to make shortcut work if menubar is hidden

  blueAct = new QAction(QIcon(":/images/blueIcon.png"), tr("blue"), this);
  blueAct->setShortcut(QKeySequence(Qt::Key_R));
  blueAct->setStatusTip(tr("blue"));
  blueAct->setCheckable(true);
  connect(blueAct, SIGNAL(triggered()), this, SLOT(blue()));
  this->addAction(blueAct); // add to make shortcut work if menubar is hidden

  grayAct = new QAction(QIcon(":/images/grayIcon.png"), tr("gray"), this);
  grayAct->setStatusTip(tr("gray"));
  grayAct->setCheckable(true);
  connect(grayAct, SIGNAL(triggered()), this, SLOT(gray()));

  lightblueAct = new QAction(QIcon(":/images/lightblueIcon.png"), tr("lightblue"), this);
  lightblueAct->setStatusTip(tr("lightblue"));
  lightblueAct->setCheckable(true);
  connect(lightblueAct, SIGNAL(triggered()), this, SLOT(lightblue()));

  lightgreenAct = new QAction(QIcon(":/images/lightgreenIcon.png"), tr("lightgreen"), this);
  lightgreenAct->setStatusTip(tr("lightgreen"));
  lightgreenAct->setCheckable(true);
  connect(lightgreenAct, SIGNAL(triggered()), this, SLOT(lightgreen()));

  magentaAct = new QAction(QIcon(":/images/magentaIcon.png"), tr("magenta"), this);
  magentaAct->setStatusTip(tr("magenta"));
  magentaAct->setCheckable(true);
  connect(magentaAct, SIGNAL(triggered()), this, SLOT(magenta()));

  orangeAct = new QAction(QIcon(":/images/orangeIcon.png"), tr("orange"), this);
  orangeAct->setStatusTip(tr("orange"));
  orangeAct->setCheckable(true);
  connect(orangeAct, SIGNAL(triggered()), this, SLOT(orange()));

  yellowAct = new QAction(QIcon(":/images/yellowIcon.png"), tr("yellow"), this);
  yellowAct->setStatusTip(tr("yellow"));
  yellowAct->setCheckable(true);
  connect(yellowAct, SIGNAL(triggered()), this, SLOT(yellow()));

  whiteAct = new QAction(QIcon(":/images/whiteIcon.png"), tr("white"), this);
  whiteAct->setStatusTip(tr("white"));
  whiteAct->setCheckable(true);
  connect(whiteAct, SIGNAL(triggered()), this, SLOT(white()));

  rotateAct = new QAction(tr("Rotate"), this);
  rotateAct->setStatusTip("Rotate Selection");
  rotateAct->setShortcut(QKeySequence(Qt::Modifier::CTRL + Qt::Key_R));
  connect(rotateAct, SIGNAL(triggered()), this, SLOT(rotate()));
  this->addAction(rotateAct); // add to make shortcut work if menubar is hidden

  helpAct = new QAction(tr("&Help"), this);
  helpAct->setShortcut(QKeySequence(Qt::Key_F1));
  connect(helpAct, SIGNAL(triggered()), this, SLOT(help()));

  aboutAct = new QAction(tr("&About"), this);
  connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));

  aboutQtAct = new QAction(tr("About &Qt"), this);
  connect(aboutQtAct, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

  QObject::connect(scrollArea->verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(verticalScrolling()));
}

void MainWindow::createMenus()
{
  fileMenu = menuBar()->addMenu(tr("&File"));
  fileMenu->addAction(newWindowAct);
  fileMenu->addAction(cloneWindowAct);
  fileMenu->addAction(closeWindowAct);
  fileMenu->addSeparator();
  fileMenu->addAction(newFileAct);
  fileMenu->addAction(openFileAct);
  fileMenu->addAction(saveFileAct);
  fileMenu->addAction(saveFileAsAct);
  fileMenu->addAction(exportPDFAct);
  fileMenu->addAction(importXOJAct);
  fileMenu->addAction(exportXOJAct);
  fileMenu->addSeparator();
  fileMenu->addAction(exitAct);

  editMenu = menuBar()->addMenu(tr("&Edit"));
  editMenu->addAction(undoAct);
  editMenu->addAction(redoAct);
  editMenu->addSeparator();
  editMenu->addAction(deleteAct);
  editMenu->addAction(cutAct);
  editMenu->addAction(copyAct);
  editMenu->addAction(pasteAct);
  editMenu->addAction(pasteImageAct);
  editMenu->addAction(pasteTextAct);
  editMenu->addSeparator();
  editMenu->addAction(selectAllAct);
  editMenu->addAction(toTheBackAct);
  editMenu->addSeparator();
  editMenu->addAction(rotateAct);
  editMenu->addSeparator();
  editMenu->addAction(settingsAct);

  pageMenu = menuBar()->addMenu(tr("&Page"));
  pageMenu->addAction(pageAddBeforeAct);
  pageMenu->addAction(pageAddAfterAct);
  pageMenu->addAction(pageAddBeginningAct);
  pageMenu->addAction(pageAddEndAct);
  pageMenu->addSeparator();
  pageMenu->addAction(pageRemoveAct);
  pageMenu->addSeparator();
  pageMenu->addAction(pageSettingsAct);

  toolsMenu = menuBar()->addMenu(tr("&Tools"));
  toolsMenu->addAction(penAct);
  toolsMenu->addAction(rulerAct);
  toolsMenu->addAction(circleAct);
  toolsMenu->addAction(rectAct);
  toolsMenu->addAction(textAct);
  toolsMenu->addAction(eraserAct);
  toolsMenu->addAction(strokeEraserAct);
  toolsMenu->addAction(selectAct);
  toolsMenu->addAction(rectSelectAct);
  toolsMenu->addAction(handAct);
  toolsMenu->addSeparator();

  penWidthMenu = toolsMenu->addMenu(tr("Pen Width"));
  penWidthMenu->addAction(veryFinePenWidthAct);
  penWidthMenu->addAction(finePenWidthAct);
  penWidthMenu->addAction(mediumPenWidthAct);
  penWidthMenu->addAction(thickPenWidthAct);
  penWidthMenu->addAction(veryThickPenWidthAct);

  //    toolsMenu->addSeparator();

  patternMenu = toolsMenu->addMenu(tr("Line Pattern"));
  patternMenu->addAction(solidPatternAct);
  patternMenu->addAction(dashPatternAct);
  patternMenu->addAction(dashDotPatternAct);
  patternMenu->addAction(dotPatternAct);

  toolsMenu->addSeparator();
  penIconMenu = toolsMenu->addMenu(tr("Pen Cursor"));
  penIconMenu->addAction(pencilIconAct);
  penIconMenu->addAction(dotIconAct);

  toolsMenu->addAction(fontAct);

  viewMenu = menuBar()->addMenu(tr("&View"));
  viewMenu->addAction(zoomInAct);
  viewMenu->addAction(zoomOutAct);
  viewMenu->addAction(zoomFitWidthAct);
  viewMenu->addAction(zoomFitHeightAct);
  viewMenu->addSeparator();
  viewMenu->addAction(toolbarAct);
  viewMenu->addAction(statusbarAct);
  viewMenu->addAction(fullscreenAct);
  viewMenu->addSeparator();
  viewMenu->addAction(showGridAct);
  viewMenu->addAction(snapToGridAct);
  viewMenu->addSeparator();
  viewMenu->addAction(saveMyStateAct);
  viewMenu->addAction(loadMyStateAct);

  helpMenu = menuBar()->addMenu(tr("&Help"));
  helpMenu->addAction(helpAct);
  helpMenu->addAction(aboutAct);
  helpMenu->addAction(aboutQtAct);
}

void MainWindow::createToolBars()
{
  QSize iconSize;
  if (QSysInfo::productType().compare("android") == 0)
  {
    iconSize = QSize(48, 48);
  }
  else
  {
    iconSize = QSize(24, 24);
  }

  fileToolBar = addToolBar(tr("File"));
  fileToolBar->setObjectName("fileToolBar");
  if (QSysInfo::productType().compare("android") == 0)
  {
    mainMenuButton = new QToolButton();
    mainMenuButton->setText("Menu");
    mainMenuButton->setPopupMode(QToolButton::InstantPopup);
    mainMenu = new QMenu("Menu");
    mainMenu->addMenu(fileMenu);
    mainMenu->addMenu(editMenu);
    mainMenu->addMenu(pageMenu);
    mainMenu->addMenu(toolsMenu);
    mainMenu->addMenu(viewMenu);
    mainMenu->addMenu(helpMenu);
    mainMenuButton->setMenu(mainMenu);
    fileToolBar->addWidget(mainMenuButton);
  }
  fileToolBar->addAction(newFileAct);
  fileToolBar->addAction(openFileAct);
  fileToolBar->addAction(saveFileAct);
  fileToolBar->addAction(exportPDFAct);
  fileToolBar->setIconSize(iconSize);

  editToolBar = addToolBar(tr("Edit"));
  editToolBar->setObjectName("editToolBar");
  editToolBar->addAction(deleteAct);
  editToolBar->addAction(cutAct);
  editToolBar->addAction(copyAct);
  editToolBar->addAction(pasteAct);
  editToolBar->addSeparator();
  editToolBar->addAction(undoAct);
  editToolBar->addAction(redoAct);
  editToolBar->setIconSize(iconSize);

  viewToolBar = addToolBar(tr("View"));
  viewToolBar->setObjectName("viewToolBar");
  viewToolBar->addAction(pageFirstAct);
  viewToolBar->addAction(pageUpAct);
  viewToolBar->addAction(pageDownAct);
  viewToolBar->addAction(pageLastAct);
  viewToolBar->addSeparator();
  viewToolBar->addAction(zoomOutAct);
  viewToolBar->addAction(zoomFitWidthAct);
  viewToolBar->addAction(zoomFitHeightAct);
  viewToolBar->addAction(zoomInAct);
  viewToolBar->addSeparator();
  viewToolBar->addAction(showGridAct);
  viewToolBar->addAction(snapToGridAct);
  viewToolBar->addSeparator();
  viewToolBar->addAction(fullscreenAct);
  viewToolBar->setIconSize(iconSize);

  addToolBarBreak();

  toolsToolBar = addToolBar(tr("Tools"));
  toolsToolBar->setObjectName("toolsToolBar");
  toolsToolBar->addAction(penAct);
  toolsToolBar->addAction(rulerAct);
  toolsToolBar->addAction(circleAct);
  toolsToolBar->addAction(rectAct);
  toolsToolBar->addAction(textAct);

  eraserToolButton = new QToolButton();
  eraserToolButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
  eraserToolButton->setPopupMode(QToolButton::MenuButtonPopup);
  eraserToolButton->addAction(eraserAct);
  eraserToolButton->addAction(strokeEraserAct);
  eraserToolButton->setDefaultAction(eraserAct);
  toolsToolBar->addWidget(eraserToolButton);
  selectToolButton = new QToolButton();
  selectToolButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
  selectToolButton->setPopupMode(QToolButton::MenuButtonPopup);
  selectToolButton->addAction(selectAct);
  selectToolButton->addAction(rectSelectAct);
  selectToolButton->setDefaultAction(selectAct);
  toolsToolBar->addWidget(selectToolButton);
  toolsToolBar->addAction(handAct);

  toolsToolBar->addSeparator();

  toolsToolBar->addAction(finePenWidthAct);
  toolsToolBar->addAction(mediumPenWidthAct);
  toolsToolBar->addAction(thickPenWidthAct);

  toolsToolBar->addSeparator();

  patternToolButton = new QToolButton();
  patternToolButton->setMenu(patternMenu);
  patternToolButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
  patternToolButton->setPopupMode(QToolButton::InstantPopup);
  toolsToolBar->addWidget(patternToolButton);

  toolsToolBar->addSeparator();

  toolsToolBar->addAction(blackAct);
  toolsToolBar->addAction(redAct);
  toolsToolBar->addAction(greenAct);
  toolsToolBar->addAction(blueAct);
  toolsToolBar->addAction(grayAct);
  toolsToolBar->addAction(lightblueAct);
  toolsToolBar->addAction(lightgreenAct);
  toolsToolBar->addAction(magentaAct);
  toolsToolBar->addAction(orangeAct);
  toolsToolBar->addAction(yellowAct);
  toolsToolBar->addAction(whiteAct);
  toolsToolBar->setIconSize(iconSize);
}

// slots
void MainWindow::newFile()
{
  if (maybeSave())
  {
    mainWidget->newFile();
    updateGUI();
  }
  else
  {
    // ignore
  }
}

void MainWindow::openFile()
{
  if (!maybeSave())
  {
    return;
  }

  QString dir;

  if (mainWidget->m_currentDocument.path().isEmpty())
  {
    dir = QDir::homePath();
  }
  else
  {
    dir = mainWidget->m_currentDocument.path();
  }

  QString fileName = QFileDialog::getOpenFileName(this, tr("Open MOJ"), dir, tr("MrWriter Files (*.moj)"));

  if (fileName.isNull())
  {
    return;
  }

  MrDoc::Document openDocument;

  if (openDocument.loadMOJ(fileName))
  {
    mainWidget->letGoSelection();
    mainWidget->setDocument(openDocument);
    setTitle();
    modified();
  }
  else
  {
    QMessageBox errMsgBox;
    errMsgBox.setText("Couldn't open file");
    errMsgBox.exec();
  }
}

QString MainWindow::askForFileName()
{
  QDateTime dateTime = QDateTime::currentDateTime();
  QString dir = QDir::homePath();
  dir.append("/");
  dir.append(dateTime.toString("yyyy-MM-dd"));
  dir.append("-Note-");
  dir.append(dateTime.toString("HH-mm"));
  dir.append(".moj");
  QString fileName = QFileDialog::getSaveFileName(this, tr("Save MOJ"), dir, tr("MrWriter Files (*.moj)"));

  return fileName;
}

bool MainWindow::saveFileAs()
{
  QString fileName = askForFileName();

  if (fileName.isNull())
  {
    return false;
  }

  mainWidget->letGoSelection();

  if (mainWidget->m_currentDocument.saveMOJ(fileName))
  {
    modified();
    setTitle();
    return true;
  }
  else
  {
    return false;
  }
}

bool MainWindow::saveFile()
{
  QString dir;
  QString fileName;
  if (mainWidget->m_currentDocument.docName().isEmpty())
  {
    fileName = askForFileName();
  }
  else
  {
    dir = mainWidget->m_currentDocument.path();
    dir.append('/');
    dir.append(mainWidget->m_currentDocument.docName());
    dir.append(".moj");
    fileName = dir;
  }

  if (fileName.isNull())
  {
    return false;
  }

  mainWidget->letGoSelection();

  if (mainWidget->m_currentDocument.saveMOJ(fileName))
  {
    modified();
    setTitle();
    return true;
  }
  else
  {
    return false;
  }
}

void MainWindow::exportPDF()
{
  QString fileName;
  if (mainWidget->m_currentDocument.docName().isEmpty())
  {
    fileName = QDir::homePath();
  }
  else
  {
    fileName = mainWidget->m_currentDocument.path();
    fileName.append('/');
    fileName.append(mainWidget->m_currentDocument.docName());
    fileName.append(".pdf");
  }
  fileName = QFileDialog::getSaveFileName(this, tr("Export PDF"), fileName, tr("Adobe PDF files (*.pdf)"));

  if (fileName.isNull())
  {
    return;
  }

  mainWidget->m_currentDocument.exportPDF(fileName);
}

void MainWindow::importXOJ()
{
  if (!maybeSave())
  {
    return;
  }

  QString dir;

  if (mainWidget->m_currentDocument.path().isEmpty())
  {
    dir = QDir::homePath();
  }
  else
  {
    dir = mainWidget->m_currentDocument.path();
  }

  QString fileName = QFileDialog::getOpenFileName(this, tr("Import XOJ"), dir, tr("Xournal Files (*.xoj)"));

  if (fileName.isNull())
  {
    return;
  }

  MrDoc::Document openDocument;

  if (openDocument.loadXOJ(fileName))
  {
    mainWidget->letGoSelection();
    mainWidget->setDocument(openDocument);
    setTitle();
    modified();
  }
  else
  {
    QMessageBox errMsgBox;
    errMsgBox.setText("Couldn't open file");
    errMsgBox.exec();
  }
}

bool MainWindow::exportXOJ()
{
  QString dir;
  QString fileName;
  //    mainWidget->currentDocument.docName().isEmpty()
  dir = QDir::homePath();
  fileName = QFileDialog::getSaveFileName(this, tr("Export XOJ"), dir, tr("Xournal Files (*.xoj)"));

  if (fileName.isNull())
  {
    return false;
  }

  if (mainWidget->m_currentDocument.saveXOJ(fileName))
  {
    modified();
    setTitle();
    return true;
  }
  else
  {
    return false;
  }
}

void MainWindow::zoomIn()
{
  mainWidget->zoomIn();
}

void MainWindow::zoomOut()
{
  mainWidget->zoomOut();
}

void MainWindow::zoomFitWidth()
{
  mainWidget->zoomFitWidth();
}

void MainWindow::zoomFitHeight()
{
  mainWidget->zoomFitHeight();
}

void MainWindow::undo()
{
  mainWidget->undo();
}

void MainWindow::redo()
{
  mainWidget->redo();
}

void MainWindow::copy()
{
  mainWidget->copy();
}

void MainWindow::cut()
{
  mainWidget->cut();
  updateGUI();
}

void MainWindow::paste()
{
  mainWidget->paste();
}

void MainWindow::deleteSlot()
{
  mainWidget->deleteSlot();
  updateGUI();
}

void MainWindow::toTheBack()
{
  mainWidget->toTheBack();
  updateGUI();
}

void MainWindow::letGoSelection()
{
  mainWidget->letGoSelection();
}


void MainWindow::pen()
{
  mainWidget->setCurrentTool(Widget::tool::PEN);
  updateGUI();
}

void MainWindow::ruler()
{
  mainWidget->setCurrentTool(Widget::tool::RULER);
  updateGUI();
}

void MainWindow::circle()
{
  mainWidget->setCurrentTool(Widget::tool::CIRCLE);
  updateGUI();
}

void MainWindow::rect()
{
  mainWidget->setCurrentTool(Widget::tool::RECT);
  updateGUI();
}

void MainWindow::text()
{
  mainWidget->setCurrentTool(Widget::tool::TEXT);
  updateGUI();
}

void MainWindow::font()
{
  bool ok;
  QFont font = QFontDialog::getFont(&ok, mainWidget->m_currentFont, this);
  if (ok) {
    mainWidget->setCurrentFont(font);
  }
  updateGUI();
}

void MainWindow::eraser()
{
  mainWidget->setCurrentTool(Widget::tool::ERASER);
  eraserToolButton->setDefaultAction(eraserAct);
  updateGUI();
}

void MainWindow::strokeEraser()
{
  mainWidget->setCurrentTool(Widget::tool::STROKE_ERASER);
  eraserToolButton->setDefaultAction(strokeEraserAct);
  updateGUI();
}

void MainWindow::select()
{
  mainWidget->setCurrentTool(Widget::tool::SELECT);
  selectToolButton->setDefaultAction(selectAct);
  updateGUI();
}

void MainWindow::rectSelect()
{
  mainWidget->setCurrentTool(Widget::tool::RECT_SELECT);
  selectToolButton->setDefaultAction(rectSelectAct);
  updateGUI();
}

void MainWindow::hand()
{
  mainWidget->setCurrentTool(Widget::tool::HAND);
  updateGUI();
}

void MainWindow::modified()
{
  setWindowModified(mainWidget->m_currentDocument.documentChanged());
}

void MainWindow::quickmenu()
{
  mainWidget->disableInput();
  auto quickMenu = new QuickMenu();
  quickMenu->setupSignalsAndSlots(this);
  quickMenu->move(QCursor::pos() - QPoint(quickMenu->width() / 2, quickMenu->height() / 2));
//  quickMenu->setWindowModality(Qt::WindowModal);
  quickMenu->setAttribute(Qt::WA_DeleteOnClose);
  quickMenu->show();
}

void MainWindow::quickmenuClose()
{
  mainWidget->enableInput();
  updateGUI();
}

void MainWindow::black()
{
  mainWidget->setCurrentColor(MrDoc::black);
  updateGUI();
}

void MainWindow::blue()
{
  mainWidget->setCurrentColor(MrDoc::blue);
  updateGUI();
}

void MainWindow::red()
{
  mainWidget->setCurrentColor(MrDoc::red);
  updateGUI();
}

void MainWindow::green()
{
  mainWidget->setCurrentColor(MrDoc::green);
  updateGUI();
}

void MainWindow::gray()
{
  mainWidget->setCurrentColor(MrDoc::gray);
  updateGUI();
}

void MainWindow::lightblue()
{
  mainWidget->setCurrentColor(MrDoc::lightblue);
  updateGUI();
}

void MainWindow::lightgreen()
{
  mainWidget->setCurrentColor(MrDoc::lightgreen);
  updateGUI();
}

void MainWindow::magenta()
{
  mainWidget->setCurrentColor(MrDoc::magenta);
  updateGUI();
}

void MainWindow::orange()
{
  mainWidget->setCurrentColor(MrDoc::orange);
  updateGUI();
}

void MainWindow::yellow()
{
  mainWidget->setCurrentColor(MrDoc::yellow);
  updateGUI();
}

void MainWindow::white()
{
  mainWidget->setCurrentColor(MrDoc::white);
  updateGUI();
}

void MainWindow::help()
{
  QDesktopServices::openUrl(QUrl("http://htmlpreview.github.io/?https://github.com/unruhschuh/MrWriter/blob/master/documentation/MrWriterDoc.html"));
}

/*
void MainWindow::help()
{
  QDialog *helpDialog = new QDialog();
  QWebEngineView *helpView = new QWebEngineView(helpDialog);
  QPushButton *closeButton = new QPushButton(helpDialog);
  closeButton->setText(tr("Close"));
  connect(closeButton, SIGNAL(clicked()), helpDialog, SLOT(close()));
  QBoxLayout *helpLayout = new QBoxLayout(QBoxLayout::TopToBottom);
  helpLayout->addWidget(helpView);
  helpLayout->addWidget(closeButton);
  helpDialog->setLayout(helpLayout);
  helpView->load(QUrl("qrc:/documentation/MrWriterDoc.html"));

  helpDialog->show();
}
*/

void MainWindow::about()
{
  QString version;
  version = version.append(QString::number(MAJOR_VERSION));
  version = version.append(".");
  version = version.append(QString::number(MINOR_VERSION));
  version = version.append(".");
  version = version.append(QString::number(PATCH_VERSION));
  QMessageBox msgBox(this);
  msgBox.setWindowTitle("About");
  msgBox.setTextFormat(Qt::RichText); // this is what makes the links clickable
  QString aboutText;
  aboutText.append("<center>");
  aboutText.append(PRODUCT_NAME);
  aboutText.append(" ");
  aboutText.append(version);
//  aboutText.append(" Build ");
//  aboutText.append(BUILD);
  aboutText.append("<br/><br/>Written by Thomas Leitz<br/><br/><a href='");
  aboutText.append(PRODUCT_URL);
  aboutText.append("'>");
  aboutText.append(PRODUCT_URL);
  aboutText.append("</a>");
  aboutText.append("<br/><br/>Licensed under the GNU GENERAL PUBLIC LICENSE Version 3.0<br/><br/><a "
                   "href='http://www.gnu.org/licenses/'>www.gnu.org/licenses</a></center>");
  msgBox.setText(aboutText);
  msgBox.setStandardButtons(QMessageBox::Ok);
  msgBox.setIconPixmap(QIcon(":/images/Icon1024.png").pixmap(QSize(100, 100)));
  msgBox.exec();
}

void MainWindow::toolbar()
{
  bool vis;
  if (toolbarAct->isChecked())
  {
    vis = true;
  }
  else
  {
    vis = false;
  }
  fileToolBar->setVisible(vis);
  editToolBar->setVisible(vis);
  viewToolBar->setVisible(vis);
  toolsToolBar->setVisible(vis);
  updateGUI();
}

void MainWindow::statusbar()
{
  bool vis;
  if (statusbarAct->isChecked())
  {
    vis = true;
  }
  else
  {
    vis = false;
  }
  statusBar()->setVisible(vis);
  updateGUI();
}

void MainWindow::fullscreen()
{
  //if (fullscreenAct->isChecked())
  if (!isFullScreen())
  {
    showFullScreen();
    menuBar()->hide();
    fullscreenAct->setChecked(true);
  }
  else
  {
    showNormal();
    menuBar()->show();
    fullscreenAct->setChecked(false);
  }
}


void MainWindow::showGrid()
{
  mainWidget->toggleGrid();
  updateGUI();
}

void MainWindow::snapToGrid()
{
  mainWidget->toggleSnapToGrid();
  updateGUI();
}


void MainWindow::verticalScrolling()
{
  size_t pageNum = mainWidget->getCurrentPage();

  if (pageNum == mainWidget->m_currentDocument.pages.size() - 1)
  {
    pageDownAct->setIcon(QIcon(":/images/pageDownPlusIcon.png"));
    pageDownAct->setText(tr("Page Down (add Page)"));
    pageDownAct->setStatusTip(tr("Page Down (add Page)"));
  }
  else
  {
    pageDownAct->setIcon(QIcon(":/images/pageDownIcon.png"));
    pageDownAct->setText(tr("Page Down"));
    pageDownAct->setStatusTip(tr("Page Down"));
  }

  size_t Npages = mainWidget->m_currentDocument.pages.size();

  QString statusMsg = QString("%1 / %2").arg(QString::number(pageNum + 1), QString::number(Npages));

  pageStatus.setText(statusMsg);

  mainWidget->pageVisible(0);
  mainWidget->updateAllPageBuffers();
  mainWidget->update();
}

void MainWindow::showEvent(QShowEvent *event)
{
  qDebug() << "show event";
  QMainWindow::showEvent(event);
  mainWidget->zoomFitWidth();

  QSettings settings;
  Widget::cursor storedValue = static_cast<Widget::cursor>(
              settings.value("cursorIcon").toInt());
  mainWidget->setCurrentPenCursor(storedValue);

}

void MainWindow::closeEvent(QCloseEvent *event)
{
  QSettings settings;
  settings.setValue("cursorIcon",
                    static_cast<int>(mainWidget->getCurrentPenCursor()));
  if (maybeSave())
  {
    event->accept();
    TabletApplication *myApp = static_cast<TabletApplication *>(qApp);
    myApp->mainWindows.removeOne(this);
    saveMyGeometry();
    if (myApp->mainWindows.isEmpty())
    {
      QGuiApplication::exit(0); // fix for calling closeEvent twice, see https://bugreports.qt.io/browse/QTBUG-43344
    }
  }
  else
  {
    event->ignore();
  }
}

bool MainWindow::maybeSave()
{
  if (mainWidget->m_currentDocument.documentChanged())
  {
    QMessageBox::StandardButton ret;
    ret = QMessageBox::warning(this, tr("Application"), tr("The document has been modified.\n"
                                                           "Do you want to save your changes?"),
                               QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    if (ret == QMessageBox::Save)
      return saveFile();
    else if (ret == QMessageBox::Cancel)
      return false;
  }
  return true;
}

void MainWindow::updateGUI()
{
  QColor currentColor = mainWidget->getCurrentColor();

  blackAct->setChecked(currentColor == MrDoc::black);
  blueAct->setChecked(currentColor == MrDoc::blue);
  redAct->setChecked(currentColor == MrDoc::red);
  greenAct->setChecked(currentColor == MrDoc::green);
  grayAct->setChecked(currentColor == MrDoc::gray);
  lightblueAct->setChecked(currentColor == MrDoc::lightblue);
  lightgreenAct->setChecked(currentColor == MrDoc::lightgreen);
  magentaAct->setChecked(currentColor == MrDoc::magenta);
  orangeAct->setChecked(currentColor == MrDoc::orange);
  yellowAct->setChecked(currentColor == MrDoc::yellow);
  whiteAct->setChecked(currentColor == MrDoc::white);

  Widget::tool currentTool = mainWidget->currentTool();

  penAct->setChecked(currentTool == Widget::tool::PEN);
  rulerAct->setChecked(currentTool == Widget::tool::RULER);
  circleAct->setChecked(currentTool == Widget::tool::CIRCLE);
  rectAct->setChecked(currentTool == Widget::tool::RECT);
  textAct->setChecked(currentTool == Widget::tool::TEXT);
  eraserAct->setChecked(currentTool == Widget::tool::ERASER);
  strokeEraserAct->setChecked(currentTool == Widget::tool::STROKE_ERASER);
  selectAct->setChecked(currentTool == Widget::tool::SELECT);
  rectSelectAct->setChecked(currentTool == Widget::tool::RECT_SELECT);
  handAct->setChecked(currentTool == Widget::tool::HAND);

  qreal currentPenWidth = mainWidget->getCurrentPenWidth();

  veryFinePenWidthAct->setChecked(currentPenWidth == Widget::m_veryFinePenWidth);
  finePenWidthAct->setChecked(currentPenWidth == Widget::m_finePenWidth);
  mediumPenWidthAct->setChecked(currentPenWidth == Widget::m_mediumPenWidth);
  thickPenWidthAct->setChecked(currentPenWidth == Widget::m_thickPenWidth);
  veryThickPenWidthAct->setChecked(currentPenWidth == Widget::m_veryThickPenWidth);

  Widget::cursor currentCursorIcon = mainWidget->getCurrentPenCursor();
  pencilIconAct->setChecked(currentCursorIcon == Widget::cursor::PENCIL);
  dotIconAct->setChecked(currentCursorIcon == Widget::cursor::DOT);

  QVector<qreal> currentPattern = mainWidget->getCurrentPattern();

  solidPatternAct->setChecked(currentPattern == MrDoc::solidLinePattern);
  dashPatternAct->setChecked(currentPattern == MrDoc::dashLinePattern);
  dashDotPatternAct->setChecked(currentPattern == MrDoc::dashDotLinePattern);
  dotPatternAct->setChecked(currentPattern == MrDoc::dotLinePattern);

  if (currentPattern == MrDoc::solidLinePattern)
    patternToolButton->setIcon(QIcon(":/images/solidPatternIcon.png"));
  if (currentPattern == MrDoc::dashLinePattern)
    patternToolButton->setIcon(QIcon(":/images/dashPatternIcon.png"));
  if (currentPattern == MrDoc::dashDotLinePattern)
    patternToolButton->setIcon(QIcon(":/images/dashDotPatternIcon.png"));
  if (currentPattern == MrDoc::dotLinePattern)
    patternToolButton->setIcon(QIcon(":/images/dotPatternIcon.png"));

  fullscreenAct->setChecked(isFullScreen());
  statusbarAct->setChecked(statusBar()->isVisible());

  showGridAct->setChecked(mainWidget->showingGrid());
  snapToGridAct->setChecked(mainWidget->snappingToGrid());

  if (mainWidget->getCurrentState() == Widget::state::SELECTED)
  {
    cutAct->setEnabled(true);
    copyAct->setEnabled(true);
    deleteAct->setEnabled(true);
    toTheBackAct->setEnabled(true);
  }
  else
  {
    cutAct->setDisabled(true);
    copyAct->setDisabled(true);
    deleteAct->setDisabled(true);
    toTheBackAct->setDisabled(true);
  }
  if (!mainWidget->m_clipboard.empty())
  {
    pasteAct->setEnabled(true);
  }
  else
  {
    pasteAct->setDisabled(true);
  }
  //    toolbarAct->setChecked()

  QPixmap pixmap(16, 16);
  pixmap.fill(mainWidget->m_currentColor);
  colorStatus.setPixmap(pixmap);

  penWidthStatus.setText(QString::number(mainWidget->m_currentPenWidth));

  statusStatus.setText(mainWidget->m_statusText);

  verticalScrolling();
  setTitle();
}

void MainWindow::rotate()
{
  //    book ok;
  qreal angle = QInputDialog::getDouble(this, tr("Rotate"), tr("Degrees:"), 0, -2147483647, 2147483647, 10);
  mainWidget->rotateSelection(angle);
}

void MainWindow::newWindow()
{
  MainWindow *window = new MainWindow();
  static_cast<TabletApplication *>(qApp)->mainWindows.append(window);
  window->show();
}

void MainWindow::cloneWindow()
{
  MainWindow *window = new MainWindow();
  window->mainWidget->m_currentDocument = mainWidget->m_currentDocument;
  window->mainWidget->m_currentDocument.setDocName("");
  window->mainWidget->m_pageBuffer = mainWidget->m_pageBuffer;
  window->mainWidget->m_currentSelection = mainWidget->m_currentSelection;
  window->mainWidget->setCurrentState(mainWidget->getCurrentState());
  //  window->mainWidget->zoomTo(mainWidget->zoom);
  window->mainWidget->m_zoom = mainWidget->m_zoom;

  window->show();

  window->mainWidget->setGeometry(window->mainWidget->getWidgetGeometry());
  window->scrollArea->updateGeometry();
  window->scrollArea->verticalScrollBar()->setValue(scrollArea->verticalScrollBar()->value());
  window->scrollArea->horizontalScrollBar()->setValue(scrollArea->horizontalScrollBar()->value());

  window->mainWidget->update();
  window->mainWidget->updateGUI();

  static_cast<TabletApplication *>(qApp)->mainWindows.append(window);
}

void MainWindow::maximize()
{
  showMaximized();
}

bool MainWindow::loadXOJ(QString fileName)
{
  return mainWidget->m_currentDocument.loadXOJ(fileName);
}

bool MainWindow::loadMOJ(QString fileName)
{
  return mainWidget->m_currentDocument.loadMOJ(fileName);
}

Widget::tool MainWindow::currentTool()
{
  return mainWidget->currentTool();
}

QColor MainWindow::currentColor()
{
  return mainWidget->getCurrentColor();
}

Widget::state MainWindow::currentState()
{
  return mainWidget->getCurrentState();
}

bool MainWindow::showingGrid()
{
  return mainWidget->showingGrid();
}

bool MainWindow::snappingToGrid()
{
  return mainWidget->snappingToGrid();
}

void MainWindow::pageSettings()
{
  size_t pageNum = mainWidget->getCurrentPage();
  qreal width = mainWidget->m_currentDocument.pages[pageNum].width();
  qreal height = mainWidget->m_currentDocument.pages[pageNum].height();
  PageSettingsDialog *pageDialog = new PageSettingsDialog(QSizeF(width, height), mainWidget->m_currentDocument.pages[pageNum].backgroundColor(), this);
  pageDialog->setWindowModality(Qt::WindowModal);
  if (pageDialog->exec() == QDialog::Accepted)
  {
    if (pageDialog->currentPageSize.isValid())
    {
      qDebug() << "valid";
      ChangePageSettingsCommand *cpsCommand = new ChangePageSettingsCommand(mainWidget, pageNum, pageDialog->currentPageSize, pageDialog->backgroundColor);
      mainWidget->m_undoStack.push(cpsCommand);
    }
  }
  delete pageDialog;
}

void MainWindow::settings()
{
  SettingsDialog * settingsDialog = new SettingsDialog(this);
  settingsDialog->exec();
}

void MainWindow::saveMyState()
{
  QSettings settings;
  settings.beginGroup("MainWindow");
  settings.setValue("windowState", saveState());
  settings.endGroup();
}

void MainWindow::loadMyState()
{
  QSettings settings;
  restoreState(settings.value("MainWindow/windowState").toByteArray());
}

void MainWindow::saveMyGeometry()
{
  QSettings settings;
  settings.beginGroup("MainWindow");
  settings.setValue("geometry", saveGeometry());
  settings.endGroup();
}

void MainWindow::loadMyGeometry()
{
  QSettings settings;
  restoreGeometry(settings.value("MainWindow/geometry").toByteArray());
}

void MainWindow::exit()
{
  static_cast<TabletApplication *>(qApp)->exit();
}

