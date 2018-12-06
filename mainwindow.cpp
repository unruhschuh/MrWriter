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
//#include <QWebEngineView>
#include <QDesktopServices>
#include <QBoxLayout>

#include <iostream>

#include "widget.h"
#include "mrdoc.h"
#include "version.h"
#include "mainwindow.h"
#include "pagesettingsdialog.h"
#include "commands.h"
#include "tabletapplication.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
  //    this->resize(1024,768);

  qApp->setAttribute(Qt::AA_UseHighDpiPixmaps);

  mainWidget = new Widget(this);
  connect(mainWidget, SIGNAL(select()), this, SLOT(select()));
  connect(mainWidget, SIGNAL(pen()), this, SLOT(pen()));
  connect(mainWidget, SIGNAL(ruler()), this, SLOT(ruler()));
  connect(mainWidget, SIGNAL(circle()), this, SLOT(circle()));
  connect(mainWidget, SIGNAL(eraser()), this, SLOT(eraser()));
  connect(mainWidget, SIGNAL(hand()), this, SLOT(hand()));

  connect(mainWidget, SIGNAL(updateGUI()), this, SLOT(updateGUI()));

  connect(mainWidget, SIGNAL(modified()), this, SLOT(modified()));

  scrollArea = new ZoomScrollArea(this);
  scrollArea->setWidget(mainWidget);
  scrollArea->setAlignment(Qt::AlignHCenter);
  //    scrollArea->setPalette(QPalette(QColor(130,255,130)));

  //    mainWidget->setGeometry(0,0,100,100);
  //    scrollArea->setWidgetResizable(true);

  loadMyGeometry();
  setCentralWidget(scrollArea);

  //    setCentralWidget(mainWidget);

  mainWidget->scrollArea = scrollArea;

  QWidget *sep1 = new QWidget();
  sep1->setFixedWidth(10);
  QWidget *sep2 = new QWidget();
  sep2->setFixedWidth(10);

  //possible memory leak?
  searchBar = new SearchBar();
  connect(searchBar, &SearchBar::searchNext, this->mainWidget, &Widget::searchPdfNext);
  connect(searchBar, &SearchBar::searchPrev, this->mainWidget, &Widget::searchPdfPrev);
  connect(searchBar, &SearchBar::clearSearch, this->mainWidget, &Widget::clearPdfSearch);

  statusBar()->addPermanentWidget(&colorStatus);
  statusBar()->addPermanentWidget(sep1);
  statusBar()->addPermanentWidget(&penWidthStatus);
  statusBar()->addPermanentWidget(sep2);
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
  if (mainWidget->currentDocument.docName().isEmpty())
  {
    docName = tr("untitled");
  }
  else
  {
    docName = mainWidget->currentDocument.docName();
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

  annotatePdfAct = new QAction(tr("&Annotate PDF"), this);
  annotatePdfAct->setStatusTip(tr("Annotate PDF"));
  connect(annotatePdfAct, &QAction::triggered, this, &MainWindow::openPdf);
  this->addAction(annotatePdfAct);

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
  //    zoomFitWidthAct->setShortcut(QKeySequence(Qt::Key_Z));
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
  //    TabletApplication *qTabApp = static_cast<TabletApplication*>(qApp); // I have no idea what this was for (TOM)
  connect(exitAct, SIGNAL(triggered()), this, SLOT(exit()));
  this->addAction(exitAct); // add to make shortcut work if menubar is hidden

  undoAct = mainWidget->undoStack.createUndoAction(this);
  //    undoAct = new QAction(QIcon(":/images/undoIcon.png"), tr("&Undo"), this);
  undoAct->setIcon(QIcon(":/images/undoIcon.png"));
  undoAct->setShortcut(QKeySequence::Undo);
  undoAct->setStatusTip(tr("Undo"));
  undoAct->disconnect(SIGNAL(triggered()));
  connect(undoAct, SIGNAL(triggered()), mainWidget, SLOT(undo()));
  //    disconnect(undoAct, SIGNAL(triggered()), mainWidget->undoStack, SLOT(undo()));
  this->addAction(undoAct); // add to make shortcut work if menubar is hidden

  redoAct = mainWidget->undoStack.createRedoAction(this);
  //    redoAct = new QAction(QIcon(":/images/redoIcon.png"), tr("&Redo"), this);
  redoAct->setIcon(QIcon(":/images/redoIcon.png"));
  redoAct->setShortcut(QKeySequence::Redo);
  redoAct->setStatusTip(tr("Redo"));
  redoAct->disconnect(SIGNAL(triggered()));
  connect(redoAct, SIGNAL(triggered()), mainWidget, SLOT(redo()));
  //    disconnect(redoAct, SIGNAL(triggered()), mainWidget->undoStack, SLOT(redo()));
  this->addAction(redoAct); // add to make shortcut work if menubar is hidden

  pageHistoryForward = new QAction(this);
  pageHistoryForward->setShortcut(QKeySequence::Forward);
  connect(pageHistoryForward, &QAction::triggered, mainWidget, &Widget::pageHistoryForward);
  this->addAction(pageHistoryForward);

  pageHistoryBackward = new QAction(this);
  pageHistoryBackward->setShortcut(QKeySequence::Back);
  connect(pageHistoryBackward, &QAction::triggered, mainWidget, &Widget::pageHistoryBackward);
  this->addAction(pageHistoryBackward);

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

  cutAct = new QAction(QIcon(":/images/cutIcon.png"), tr("Cut"), this);
  cutAct->setShortcuts(QKeySequence::Cut);
  cutAct->setStatusTip(tr("Cut"));
  connect(cutAct, SIGNAL(triggered()), mainWidget, SLOT(cut()));
  this->addAction(cutAct); // add to make shortcut work if menubar is hidden

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

  highlighterAct = new QAction(tr("Highlighter"), this);
  highlighterAct->setStatusTip(tr("Highlighting Tool"));
  highlighterAct->setShortcut(QKeySequence(Qt::Key_2));
  highlighterAct->setCheckable(true);
  highlighterAct->setChecked(false);
  connect(highlighterAct, &QAction::triggered, this, &MainWindow::highlighter);
  this->addAction(highlighterAct);

  rulerAct = new QAction(QIcon(":/images/rulerIcon.png"), tr("Ruler"), this);
  rulerAct->setStatusTip(tr("Ruler Tool"));
  rulerAct->setShortcut(QKeySequence(Qt::Key_3));
  rulerAct->setCheckable(true);
  rulerAct->setChecked(false);
  connect(rulerAct, SIGNAL(triggered()), this, SLOT(ruler()));
  this->addAction(rulerAct); // add to make shortcut work if menubar is hidden

  circleAct = new QAction(QIcon(":/images/circleIcon.png"), tr("circle"), this);
  circleAct->setStatusTip(tr("circle Tool"));
  circleAct->setShortcut(QKeySequence(Qt::Key_4));
  circleAct->setCheckable(true);
  circleAct->setChecked(false);
  connect(circleAct, SIGNAL(triggered()), this, SLOT(circle()));
  this->addAction(circleAct); // add to make shortcut work if menubar is hidden

  eraserAct = new QAction(QIcon(":/images/eraserIcon.png"), tr("Eraser"), this);
  eraserAct->setStatusTip(tr("Eraser Tool"));
  eraserAct->setShortcut(QKeySequence(Qt::Key_5));
  eraserAct->setCheckable(true);
  connect(eraserAct, SIGNAL(triggered()), this, SLOT(eraser()));
  this->addAction(eraserAct); // add to make shortcut work if menubar is hidden

  selectAct = new QAction(QIcon(":/images/selectIcon.png"), tr("Select"), this);
  selectAct->setStatusTip(tr("Select Tool"));
  selectAct->setShortcut(QKeySequence(Qt::Key_6));
  selectAct->setCheckable(true);
  connect(selectAct, SIGNAL(triggered()), this, SLOT(select()));
  this->addAction(selectAct); // add to make shortcut work if menubar is hidden

  handAct = new QAction(QIcon(":/images/handIcon.png"), tr("Hand"), this);
  handAct->setStatusTip(tr("Hand Tool"));
  handAct->setShortcut(QKeySequence(Qt::Key_7));
  handAct->setCheckable(true);
  connect(handAct, SIGNAL(triggered()), this, SLOT(hand()));
  this->addAction(handAct); // add to make shortcut work if menubar is hidden

  textAct = new QAction(tr("Text"), this);
  textAct->setStatusTip(tr("Insert text"));
  textAct->setShortcut(QKeySequence(Qt::Key_8));
  textAct->setCheckable(true);
  textAct->setChecked(false);
  connect(textAct, &QAction::triggered, this, &MainWindow::text);
  this->addAction(textAct);

  markdownAct = new QAction(tr("Markdown"), this);
  markdownAct->setStatusTip(tr("Insert Markdown text"));
  markdownAct->setShortcut(QKeySequence(Qt::Key_9));
  markdownAct->setCheckable(true);
  markdownAct->setChecked(false);
  connect(markdownAct, &QAction::triggered, this, &MainWindow::markdown);
  this->addAction(markdownAct);

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

  horizontalViewAct = new QAction(tr("HView"), this);
  horizontalViewAct->setStatusTip(tr("Horizontal view"));
  horizontalViewAct->setCheckable(true);
  horizontalViewAct->setChecked(false);
  connect(horizontalViewAct, &QAction::triggered, this, &MainWindow::horizontalView);
  this->addAction(horizontalViewAct);

  verticalViewAct = new QAction(tr("VView"), this);
  verticalViewAct->setStatusTip(tr("Vertical view"));
  verticalViewAct->setCheckable(true);
  verticalViewAct->setChecked(true);
  connect(verticalViewAct, &QAction::triggered, this, &MainWindow::verticalView);
  this->addAction(verticalViewAct);

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

  fontAct = new QAction(tr("Sans 12"), this);
  fontAct->setText(tr("Select font"));
  fontAct->setIconText(mainWidget->getCurrentFont().family() + ", " + QString::number(mainWidget->getCurrentFont().pointSize()));
  fontAct->setStatusTip(tr("Set font"));
  fontAct->setShortcut(QKeySequence(Qt::Modifier::CTRL + Qt::Modifier::SHIFT + Qt::Key_F));
  connect(fontAct, &QAction::triggered, this, &MainWindow::selectFont);
  this->addAction(fontAct);

  helpAct = new QAction(tr("&Help"), this);
  helpAct->setShortcut(QKeySequence(Qt::Key_F1));
  connect(helpAct, SIGNAL(triggered()), this, SLOT(help()));

  aboutAct = new QAction(tr("&About"), this);
  connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));

  aboutQtAct = new QAction(tr("About &Qt"), this);
  connect(aboutQtAct, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

  QObject::connect(scrollArea->verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(scrolling()));
  QObject::connect(scrollArea->horizontalScrollBar(), &QScrollBar::valueChanged, this, &MainWindow::scrolling);
}

void MainWindow::createMenus()
{
  fileMenu = menuBar()->addMenu(tr("&File"));
  fileMenu->addAction(newWindowAct);
  fileMenu->addAction(cloneWindowAct);
  fileMenu->addAction(closeWindowAct);
  fileMenu->addSeparator();
  fileMenu->addAction(newFileAct);
  fileMenu->addAction(annotatePdfAct);
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
  editMenu->addAction(cutAct);
  editMenu->addAction(copyAct);
  editMenu->addAction(pasteAct);
  editMenu->addSeparator();
  editMenu->addAction(selectAllAct);
  editMenu->addSeparator();
  editMenu->addAction(rotateAct);
  editMenu->addSeparator();
  editMenu->addAction(fontAct);

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
  toolsMenu->addAction(highlighterAct);
  toolsMenu->addAction(rulerAct);
  toolsMenu->addAction(circleAct);
  toolsMenu->addAction(eraserAct);
  toolsMenu->addAction(selectAct);
  toolsMenu->addAction(handAct);
  toolsMenu->addAction(textAct);
  toolsMenu->addAction(markdownAct);
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

  viewOrientationMenu = new QMenu(tr("View orientation"), this);
  viewOrientationMenu->addAction(verticalViewAct);
  viewOrientationMenu->addAction(horizontalViewAct);

  viewMenu = menuBar()->addMenu(tr("&View"));
  viewMenu->addAction(zoomInAct);
  viewMenu->addAction(zoomOutAct);
  viewMenu->addAction(zoomFitWidthAct);
  viewMenu->addAction(zoomFitHeightAct);
  viewMenu->addSeparator();
  viewMenu->addAction(toolbarAct);
  viewMenu->addAction(statusbarAct);
  viewMenu->addAction(fullscreenAct);
  viewMenu->addMenu(viewOrientationMenu);
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
  editToolBar->addAction(cutAct);
  editToolBar->addAction(copyAct);
  editToolBar->addAction(pasteAct);
  editToolBar->addSeparator();
  editToolBar->addAction(fontAct);
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
  viewToolBar->addAction(verticalViewAct);
  viewToolBar->addAction(horizontalViewAct);
  viewToolBar->addSeparator();
  viewToolBar->addAction(fullscreenAct);
  viewToolBar->setIconSize(iconSize);

  viewToolBar->addWidget(searchBar);

  addToolBarBreak();

  toolsToolBar = addToolBar(tr("Tools"));
  toolsToolBar->setObjectName("toolsToolBar");
  toolsToolBar->addAction(penAct);
  toolsToolBar->addAction(highlighterAct);
  toolsToolBar->addAction(rulerAct);
  toolsToolBar->addAction(circleAct);
  toolsToolBar->addAction(eraserAct);
  toolsToolBar->addAction(selectAct);
  toolsToolBar->addAction(handAct);
  toolsToolBar->addAction(textAct);
  toolsToolBar->addAction(markdownAct);

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

void MainWindow::openPdf(){
    if(!maybeSave()){
        return;
    }

    QString dir;
    if(mainWidget->currentDocument.path().isEmpty()){
        dir = QDir::homePath();
    }
    else{
        dir = mainWidget->currentDocument.path();
    }
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open PDF"), dir, tr("PDF Files (*.pdf)"));
    if(fileName.isNull()){
        return;
    }

    MrDoc::Document openDocument;

    if (openDocument.loadPDF(fileName))
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

void MainWindow::openFile()
{
  if (!maybeSave())
  {
    return;
  }

  QString dir;

  if (mainWidget->currentDocument.path().isEmpty())
  {
    dir = QDir::homePath();
  }
  else
  {
    dir = mainWidget->currentDocument.path();
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

  if (mainWidget->currentDocument.saveMOJ(fileName))
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
  if (mainWidget->currentDocument.docName().isEmpty())
  {
    fileName = askForFileName();
  }
  else
  {
    dir = mainWidget->currentDocument.path();
    dir.append('/');
    dir.append(mainWidget->currentDocument.docName());
    dir.append(".moj");
    fileName = dir;
  }

  if (fileName.isNull())
  {
    return false;
  }

  if (mainWidget->currentDocument.saveMOJ(fileName))
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
  if (mainWidget->currentDocument.docName().isEmpty())
  {
    fileName = QDir::homePath();
  }
  else
  {
    fileName = mainWidget->currentDocument.path();
    fileName.append('/');
    fileName.append(mainWidget->currentDocument.docName());
    fileName.append(".pdf");
  }
  fileName = QFileDialog::getSaveFileName(this, tr("Export PDF"), fileName, tr("Adobe PDF files (*.pdf)"));

  if (fileName.isNull())
  {
    return;
  }

  mainWidget->currentDocument.exportPDF(fileName);
}

void MainWindow::importXOJ()
{
  if (!maybeSave())
  {
    return;
  }

  QString dir;

  if (mainWidget->currentDocument.path().isEmpty())
  {
    dir = QDir::homePath();
  }
  else
  {
    dir = mainWidget->currentDocument.path();
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

  if (mainWidget->currentDocument.saveXOJ(fileName))
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

void MainWindow::pen()
{
  mainWidget->setCurrentTool(Widget::tool::PEN);
  updateGUI();
}

void MainWindow::highlighter(){
    mainWidget->setCurrentTool(Widget::tool::HIGHLIGHTER);
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

void MainWindow::eraser()
{
  mainWidget->setCurrentTool(Widget::tool::ERASER);
  updateGUI();
}

void MainWindow::select()
{
  mainWidget->setCurrentTool(Widget::tool::SELECT);
  updateGUI();
}

void MainWindow::hand()
{
  mainWidget->setCurrentTool(Widget::tool::HAND);
  updateGUI();
}

void MainWindow::text(){
    mainWidget->setCurrentTool(Widget::tool::TEXT);
    updateGUI();
}

void MainWindow::markdown(){
    mainWidget->setCurrentTool(Widget::tool::MARKDOWN);
    updateGUI();
}

void MainWindow::selectFont(){
    bool ok;
    QFont font = QFontDialog::getFont(
                &ok, QFont("Sans", 12), this);
    if(ok){
        mainWidget->setCurrentFont(font);
        fontAct->setIconText(font.family() + ", " + QString::number(font.pointSize()));
    }
    else{
        mainWidget->setCurrentFont(font);
        fontAct->setIconText(font.family() + ", " + QString::number(font.pointSize()));
    }
}

void MainWindow::modified()
{
  setWindowModified(mainWidget->currentDocument.documentChanged());
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
  aboutText.append(" Build ");
  aboutText.append(BUILD);
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
  if (fullscreenAct->isChecked())
  {
    showFullScreen();
    menuBar()->hide();
  }
  else
  {
    showNormal();
    menuBar()->show();
  }
}

void MainWindow::scrolling()
{
  QSize size = scrollArea->size();
  QPoint globalMousePos = QPoint(size.width() / 2.0, size.height() / 2.0) + scrollArea->pos() + this->pos();
  QPoint pos = mainWidget->mapFromGlobal(globalMousePos);
  int pageNum = mainWidget->getPageFromMousePos(pos);

  pageNum = mainWidget->getCurrentPage();

  if (pageNum == mainWidget->currentDocument.pages.size() - 1)
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

  int Npages = mainWidget->currentDocument.pages.size();

  QString statusMsg = QString("%1 / %2").arg(QString::number(pageNum + 1), QString::number(Npages));

  pageStatus.setText(statusMsg);
}

void MainWindow::showEvent(QShowEvent *event)
{
  QMainWindow::showEvent(event);
  mainWidget->zoomFitWidth();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
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

void MainWindow::keyReleaseEvent(QKeyEvent *event){
    if(event->key() == Qt::Key_Control){
        qDebug() << "release";
        mainWidget->ctrlZoom = false;
        if(mainWidget->dismissedCleanZoom){
            mainWidget->updateAllPageBuffers();
            mainWidget->update();
            mainWidget->updateAllPageBuffers();
        }
    }
}

bool MainWindow::maybeSave()
{
  if (mainWidget->currentDocument.documentChanged())
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
    QFont currentFont = mainWidget->getCurrentFont();
    fontAct->setIconText(currentFont.family() + ", " + QString::number(currentFont.pointSize()));
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

  Widget::tool currentTool = mainWidget->getCurrentTool();

  penAct->setChecked(currentTool == Widget::tool::PEN);
  highlighterAct->setChecked(currentTool == Widget::tool::HIGHLIGHTER);
  rulerAct->setChecked(currentTool == Widget::tool::RULER);
  circleAct->setChecked(currentTool == Widget::tool::CIRCLE);
  eraserAct->setChecked(currentTool == Widget::tool::ERASER);
  selectAct->setChecked(currentTool == Widget::tool::SELECT);
  handAct->setChecked(currentTool == Widget::tool::HAND);
  textAct->setChecked(currentTool == Widget::tool::TEXT);
  markdownAct->setChecked(currentTool == Widget::tool::MARKDOWN);

  qreal currentPenWidth = mainWidget->getCurrentPenWidth();

  veryFinePenWidthAct->setChecked(currentPenWidth == Widget::veryFinePenWidth);
  finePenWidthAct->setChecked(currentPenWidth == Widget::finePenWidth);
  mediumPenWidthAct->setChecked(currentPenWidth == Widget::mediumPenWidth);
  thickPenWidthAct->setChecked(currentPenWidth == Widget::thickPenWidth);
  veryThickPenWidthAct->setChecked(currentPenWidth == Widget::veryThickPenWidth);

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

  verticalViewAct->setChecked(mainWidget->getCurrentView() == Widget::view::VERTICAL);
  horizontalViewAct->setChecked(mainWidget->getCurrentView() == Widget::view::HORIZONTAL);

  if (mainWidget->getCurrentState() == Widget::state::SELECTED)
  {
    cutAct->setEnabled(true);
    copyAct->setEnabled(true);
  }
  else
  {
    cutAct->setDisabled(true);
    copyAct->setDisabled(true);
  }
  //    toolbarAct->setChecked()

  QPixmap pixmap(16, 16);
  pixmap.fill(mainWidget->currentColor);
  colorStatus.setPixmap(pixmap);

  penWidthStatus.setText(QString::number(mainWidget->currentPenWidth));

  scrolling();
  setTitle();
  if(currentTool != Widget::tool::TEXT){
      mainWidget->closeTextBox();
  }
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
  window->mainWidget->currentDocument = mainWidget->currentDocument;
  window->mainWidget->currentDocument.setDocName("");
  window->mainWidget->pageBufferPtr = mainWidget->pageBufferPtr;
  window->mainWidget->currentSelection = mainWidget->currentSelection;
  window->mainWidget->setCurrentState(mainWidget->getCurrentState());
  //  window->mainWidget->zoomTo(mainWidget->zoom);
  window->mainWidget->zoom = mainWidget->zoom;

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

void MainWindow::horizontalView(){
    mainWidget->setCurrentView(Widget::view::HORIZONTAL);
    updateGUI();
}

void MainWindow::verticalView(){
    mainWidget->setCurrentView(Widget::view::VERTICAL);
    updateGUI();
}

bool MainWindow::loadXOJ(QString fileName)
{
  return mainWidget->currentDocument.loadXOJ(fileName);
  updateGUI();
}

bool MainWindow::loadMOJ(QString fileName)
{
  return mainWidget->currentDocument.loadMOJ(fileName);
  updateGUI();
}

bool MainWindow::loadPDF(QString fileName){

    MrDoc::Document openDocument;

    if (openDocument.loadPDF(fileName))
    {
      mainWidget->letGoSelection();
      mainWidget->setDocument(openDocument);
      setTitle();
      modified();
      return true;
    }
    else
    {
      return false;
    }
}

void MainWindow::pageSettings()
{
  int pageNum = mainWidget->getCurrentPage();
  qreal width = mainWidget->currentDocument.pages[pageNum].width();
  qreal height = mainWidget->currentDocument.pages[pageNum].height();
  PageSettingsDialog *pageDialog = new PageSettingsDialog(QSizeF(width, height), mainWidget->currentDocument.pages[pageNum].backgroundColor(), mainWidget->currentDocument.pages[pageNum].getBackgroundType(), this);
  pageDialog->setWindowModality(Qt::WindowModal);
  if (pageDialog->exec() == QDialog::Accepted)
  {
    if (pageDialog->currentPageSize.isValid())
    {
      qDebug() << "valid";
      ChangePageSettingsCommand *cpsCommand = new ChangePageSettingsCommand(mainWidget, pageNum, pageDialog->currentPageSize, pageDialog->backgroundColor, pageDialog->m_backgroundType);
      mainWidget->undoStack.push(cpsCommand);
    }
  }
  delete pageDialog;
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

ZoomScrollArea::ZoomScrollArea(QWidget *parent)
    : QScrollArea(parent) {}

void ZoomScrollArea::wheelEvent(QWheelEvent *event){
    if(event->modifiers().testFlag(Qt::ControlModifier)){
        event->ignore();
    }
    else{
        QScrollArea::wheelEvent(event);
    }
}
