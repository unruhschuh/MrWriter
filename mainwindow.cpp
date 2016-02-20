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

  scrollArea = new QScrollArea(this);
  scrollArea->setWidget(mainWidget);
  scrollArea->setAlignment(Qt::AlignHCenter);
  //    scrollArea->setPalette(QPalette(QColor(130,255,130)));

  //    mainWidget->setGeometry(0,0,100,100);
  //    scrollArea->setWidgetResizable(true);

  loadMyGeometry();
  setCentralWidget(scrollArea);

  //    setCentralWidget(mainWidget);

  mainWidget->m_scrollArea = scrollArea;

  QWidget *sep1 = new QWidget();
  sep1->setFixedWidth(10);
  QWidget *sep2 = new QWidget();
  sep2->setFixedWidth(10);

  statusBar()->addPermanentWidget(&m_colorStatus);
  statusBar()->addPermanentWidget(sep1);
  statusBar()->addPermanentWidget(&m_penWidthStatus);
  statusBar()->addPermanentWidget(sep2);
  statusBar()->addPermanentWidget(&m_pageStatus);

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
  m_newWindowAct = new QAction(tr("New &Window"), this);
  m_newWindowAct->setShortcut(QKeySequence(Qt::Modifier::CTRL + Qt::Modifier::SHIFT + Qt::Key_N));
  m_newWindowAct->setStatusTip(tr("New Window"));
  connect(m_newWindowAct, SIGNAL(triggered()), this, SLOT(newWindow()));
  this->addAction(m_newWindowAct);

  m_cloneWindowAct = new QAction(tr("Clone Window"), this);
  m_cloneWindowAct->setShortcut(QKeySequence(Qt::Modifier::CTRL + Qt::Modifier::SHIFT + Qt::Key_C));
  m_cloneWindowAct->setStatusTip(tr("Clone Window"));
  connect(m_cloneWindowAct, SIGNAL(triggered()), this, SLOT(cloneWindow()));
  this->addAction(m_cloneWindowAct);

  m_closeWindowAct = new QAction(tr("Close Window"), this);
  m_closeWindowAct->setShortcut(QKeySequence(QKeySequence::Close));
  m_closeWindowAct->setStatusTip(tr("Close Window"));
  connect(m_closeWindowAct, SIGNAL(triggered()), this, SLOT(close()));
  this->addAction(m_closeWindowAct);

  m_newFileAct = new QAction(QIcon(":/images/newIcon.png"), tr("&New file"), this);
  m_newFileAct->setShortcuts(QKeySequence::New);
  m_newFileAct->setStatusTip(tr("New File"));
  connect(m_newFileAct, SIGNAL(triggered()), this, SLOT(newFile()));
  this->addAction(m_newFileAct); // add to make shortcut work if menubar is hidden

  m_openFileAct = new QAction(QIcon(":/images/openIcon.png"), tr("&Open file"), this);
  m_openFileAct->setShortcuts(QKeySequence::Open);
  m_openFileAct->setStatusTip(tr("Open File"));
  connect(m_openFileAct, SIGNAL(triggered()), this, SLOT(openFile()));
  this->addAction(m_openFileAct);

  m_saveFileAct = new QAction(QIcon(":/images/saveIcon.png"), tr("&Save file"), this);
  m_saveFileAct->setShortcuts(QKeySequence::Save);
  m_saveFileAct->setStatusTip(tr("Save File"));
  connect(m_saveFileAct, SIGNAL(triggered()), this, SLOT(saveFile()));
  this->addAction(m_saveFileAct);

  m_saveFileAsAct = new QAction(tr("&Save file as"), this);
  m_saveFileAsAct->setShortcuts(QKeySequence::SaveAs);
  m_saveFileAsAct->setStatusTip(tr("Save File as"));
  connect(m_saveFileAsAct, SIGNAL(triggered()), this, SLOT(saveFileAs()));
  this->addAction(m_saveFileAsAct);

  m_exportPDFAct = new QAction(QIcon(":/images/savePDFIcon.png"), tr("Export PDF"), this);
  m_exportPDFAct->setShortcut(QKeySequence(Qt::Modifier::CTRL + Qt::Key_E));
  //    zoomFitWidthAct->setShortcut(QKeySequence(Qt::Key_Z));
  m_exportPDFAct->setStatusTip(tr("Export PDF"));
  connect(m_exportPDFAct, SIGNAL(triggered()), this, SLOT(exportPDF()));

  m_importXOJAct = new QAction(tr("Import Xournal File"), this);
  m_importXOJAct->setStatusTip(tr("Import Xournal File"));
  connect(m_importXOJAct, SIGNAL(triggered()), this, SLOT(importXOJ()));

  m_exportXOJAct = new QAction(tr("Export Xournal File"), this);
  m_exportXOJAct->setStatusTip(tr("Export Xournal File"));
  connect(m_exportXOJAct, SIGNAL(triggered()), this, SLOT(exportXOJ()));

  m_exitAct = new QAction(tr("E&xit"), this);
  m_exitAct->setShortcuts(QKeySequence::Quit);
  m_exitAct->setStatusTip(tr("Exit MrWriter"));
  //    TabletApplication *qTabApp = static_cast<TabletApplication*>(qApp); // I have no idea what this was for (TOM)
  connect(m_exitAct, SIGNAL(triggered()), this, SLOT(exit()));
  this->addAction(m_exitAct); // add to make shortcut work if menubar is hidden

  m_undoAct = mainWidget->m_undoStack.createUndoAction(this);
  //    undoAct = new QAction(QIcon(":/images/undoIcon.png"), tr("&Undo"), this);
  m_undoAct->setIcon(QIcon(":/images/undoIcon.png"));
  m_undoAct->setShortcut(QKeySequence::Undo);
  m_undoAct->setStatusTip(tr("Undo"));
  m_undoAct->disconnect(SIGNAL(triggered()));
  connect(m_undoAct, SIGNAL(triggered()), mainWidget, SLOT(undo()));
  //    disconnect(undoAct, SIGNAL(triggered()), mainWidget->undoStack, SLOT(undo()));
  this->addAction(m_undoAct); // add to make shortcut work if menubar is hidden

  m_redoAct = mainWidget->m_undoStack.createRedoAction(this);
  //    redoAct = new QAction(QIcon(":/images/redoIcon.png"), tr("&Redo"), this);
  m_redoAct->setIcon(QIcon(":/images/redoIcon.png"));
  m_redoAct->setShortcut(QKeySequence::Redo);
  m_redoAct->setStatusTip(tr("Redo"));
  m_redoAct->disconnect(SIGNAL(triggered()));
  connect(m_redoAct, SIGNAL(triggered()), mainWidget, SLOT(redo()));
  //    disconnect(redoAct, SIGNAL(triggered()), mainWidget->undoStack, SLOT(redo()));
  this->addAction(m_redoAct); // add to make shortcut work if menubar is hidden

  m_selectAllAct = new QAction(tr("Select &All"), this);
  m_selectAllAct->setShortcut(QKeySequence(Qt::Modifier::CTRL + Qt::Key_A));
  m_selectAllAct->setToolTip(tr("Select All"));
  connect(m_selectAllAct, SIGNAL(triggered()), mainWidget, SLOT(selectAll()));
  this->addAction(m_selectAllAct);

  m_copyAct = new QAction(QIcon(":/images/copyIcon.png"), tr("&Copy"), this);
  m_copyAct->setShortcuts(QKeySequence::Copy);
  m_copyAct->setStatusTip(tr("Copy"));
  connect(m_copyAct, SIGNAL(triggered()), mainWidget, SLOT(copy()));
  this->addAction(m_copyAct); // add to make shortcut work if menubar is hidden

  m_pasteAct = new QAction(QIcon(":/images/pasteIcon.png"), tr("&Paste"), this);
  m_pasteAct->setShortcuts(QKeySequence::Paste);
  m_pasteAct->setStatusTip(tr("Paste"));
  connect(m_pasteAct, SIGNAL(triggered()), mainWidget, SLOT(paste()));
  this->addAction(m_pasteAct); // add to make shortcut work if menubar is hidden

  m_cutAct = new QAction(QIcon(":/images/cutIcon.png"), tr("Cut"), this);
  m_cutAct->setShortcuts(QKeySequence::Cut);
  m_cutAct->setStatusTip(tr("Cut"));
  connect(m_cutAct, SIGNAL(triggered()), mainWidget, SLOT(cut()));
  this->addAction(m_cutAct); // add to make shortcut work if menubar is hidden

  m_zoomInAct = new QAction(QIcon(":/images/zoomInIcon.png"), tr("Zoom &In"), this);
  m_zoomInAct->setShortcut(QKeySequence::ZoomIn);
  m_zoomInAct->setStatusTip(tr("Zoom In"));
  connect(m_zoomInAct, SIGNAL(triggered()), this, SLOT(zoomIn()));
  this->addAction(m_zoomInAct); // add to make shortcut work if menubar is hidden

  m_zoomOutAct = new QAction(QIcon(":/images/zoomOutIcon.png"), tr("Zoom &Out"), this); // QAction(QIcon(":/images/new.png"), tr("&New"), this);
  m_zoomOutAct->setShortcut(QKeySequence::ZoomOut);
  m_zoomOutAct->setStatusTip(tr("Zoom Out"));
  connect(m_zoomOutAct, SIGNAL(triggered()), this, SLOT(zoomOut()));
  this->addAction(m_zoomOutAct); // add to make shortcut work if menubar is hidden

  m_zoomFitWidthAct = new QAction(QIcon(":/images/zoomFitWidthIcon.png"), tr("Zoom to fit width"), this); // QAction(QIcon(":/images/new.png"), tr("&New"), this);
  m_zoomFitWidthAct->setStatusTip(tr("Zoom to fit width"));
  m_zoomFitWidthAct->setShortcut(QKeySequence(Qt::Key_Z));
  connect(m_zoomFitWidthAct, SIGNAL(triggered()), this, SLOT(zoomFitWidth()));
  this->addAction(m_zoomFitWidthAct); // add to make shortcut work if menubar is hidden

  m_zoomFitHeightAct =
      new QAction(QIcon(":/images/zoomFitHeightIcon.png"), tr("Zoom to fit Height"), this); // QAction(QIcon(":/images/new.png"), tr("&New"), this);
  m_zoomFitHeightAct->setStatusTip(tr("Zoom to fit Height"));
  m_zoomFitHeightAct->setShortcut(QKeySequence(Qt::Key_X));
  connect(m_zoomFitHeightAct, SIGNAL(triggered()), this, SLOT(zoomFitHeight()));
  this->addAction(m_zoomFitHeightAct); // add to make shortcut work if menubar is hidden

  m_pageFirstAct = new QAction(QIcon(":/images/pageFirstIcon.png"), tr("First Page"), this);
  m_pageFirstAct->setStatusTip(tr("First Page"));
  connect(m_pageFirstAct, SIGNAL(triggered()), mainWidget, SLOT(pageFirst()));

  m_pageLastAct = new QAction(QIcon(":/images/pageLastIcon.png"), tr("Last Page"), this);
  m_pageLastAct->setStatusTip(tr("Last Page"));
  connect(m_pageLastAct, SIGNAL(triggered()), mainWidget, SLOT(pageLast()));

  m_pageUpAct = new QAction(QIcon(":/images/pageUpIcon.png"), tr("Page &Up"), this);
  m_pageUpAct->setStatusTip(tr("Page Up"));
  m_pageUpAct->setShortcut(QKeySequence(Qt::Key_Up));
  connect(m_pageUpAct, SIGNAL(triggered()), mainWidget, SLOT(pageUp()));
  this->addAction(m_pageUpAct);

  m_pageDownAct = new QAction(QIcon(":/images/pageDownIcon.png"), tr("Page &Down"), this);
  m_pageDownAct->setStatusTip(tr("Page Down"));
  m_pageDownAct->setShortcut(QKeySequence(Qt::Key_Down));
  connect(m_pageDownAct, SIGNAL(triggered()), mainWidget, SLOT(pageDown()));
  this->addAction(m_pageDownAct);

  m_pageAddBeforeAct = new QAction(tr("New Page &Before"), this);
  m_pageAddBeforeAct->setStatusTip(tr("New Page before current Page"));
  connect(m_pageAddBeforeAct, SIGNAL(triggered()), mainWidget, SLOT(pageAddBefore()));

  m_pageAddAfterAct = new QAction(tr("New Page &After"), this);
  m_pageAddAfterAct->setStatusTip(tr("New Page after current Page"));
  connect(m_pageAddAfterAct, SIGNAL(triggered()), mainWidget, SLOT(pageAddAfter()));

  m_pageAddEndAct = new QAction(tr("New Page at &End"), this);
  m_pageAddEndAct->setStatusTip(tr("New Page after last Page"));
  connect(m_pageAddEndAct, SIGNAL(triggered()), mainWidget, SLOT(pageAddEnd()));

  m_pageAddBeginningAct = new QAction(tr("New Page at Beginning"), this);
  m_pageAddBeginningAct->setStatusTip(tr("New Page before first Page"));
  connect(m_pageAddBeginningAct, SIGNAL(triggered()), mainWidget, SLOT(pageAddBeginning()));

  m_pageRemoveAct = new QAction(tr("Remove Current Page"), this);
  m_pageRemoveAct->setStatusTip(tr("Remove Current Page"));
  connect(m_pageRemoveAct, SIGNAL(triggered()), mainWidget, SLOT(pageRemove()));

  m_pageSettingsAct = new QAction(tr("Page Settings"), this);
  m_pageSettingsAct->setStatusTip(tr("Page Settings"));
  connect(m_pageSettingsAct, SIGNAL(triggered()), this, SLOT(pageSettings()));

  m_saveMyStateAct = new QAction(tr("Save window state"), this);
  m_saveMyStateAct->setStatusTip(tr("Save window state"));
  connect(m_saveMyStateAct, SIGNAL(triggered()), this, SLOT(saveMyState()));

  m_loadMyStateAct = new QAction(tr("Load window state"), this);
  m_loadMyStateAct->setStatusTip(tr("Load window state"));
  connect(m_loadMyStateAct, SIGNAL(triggered()), this, SLOT(loadMyState()));

  m_penAct = new QAction(QIcon(":/images/penIcon.png"), tr("Pen"), this);
  m_penAct->setStatusTip(tr("Pen Tool"));
  m_penAct->setShortcut(QKeySequence(Qt::Key_1));
  m_penAct->setCheckable(true);
  m_penAct->setChecked(true);
  connect(m_penAct, SIGNAL(triggered()), this, SLOT(pen()));
  this->addAction(m_penAct); // add to make shortcut work if menubar is hidden

  m_rulerAct = new QAction(QIcon(":/images/rulerIcon.png"), tr("Ruler"), this);
  m_rulerAct->setStatusTip(tr("Ruler Tool"));
  m_rulerAct->setShortcut(QKeySequence(Qt::Key_2));
  m_rulerAct->setCheckable(true);
  m_rulerAct->setChecked(false);
  connect(m_rulerAct, SIGNAL(triggered()), this, SLOT(ruler()));
  this->addAction(m_rulerAct); // add to make shortcut work if menubar is hidden

  m_circleAct = new QAction(QIcon(":/images/circleIcon.png"), tr("circle"), this);
  m_circleAct->setStatusTip(tr("circle Tool"));
  m_circleAct->setShortcut(QKeySequence(Qt::Key_3));
  m_circleAct->setCheckable(true);
  m_circleAct->setChecked(false);
  connect(m_circleAct, SIGNAL(triggered()), this, SLOT(circle()));
  this->addAction(m_circleAct); // add to make shortcut work if menubar is hidden

  m_eraserAct = new QAction(QIcon(":/images/eraserIcon.png"), tr("Eraser"), this);
  m_eraserAct->setStatusTip(tr("Eraser Tool"));
  m_eraserAct->setShortcut(QKeySequence(Qt::Key_4));
  m_eraserAct->setCheckable(true);
  connect(m_eraserAct, SIGNAL(triggered()), this, SLOT(eraser()));
  this->addAction(m_eraserAct); // add to make shortcut work if menubar is hidden

  m_selectAct = new QAction(QIcon(":/images/selectIcon.png"), tr("Select"), this);
  m_selectAct->setStatusTip(tr("Select Tool"));
  m_selectAct->setShortcut(QKeySequence(Qt::Key_5));
  m_selectAct->setCheckable(true);
  connect(m_selectAct, SIGNAL(triggered()), this, SLOT(select()));
  this->addAction(m_selectAct); // add to make shortcut work if menubar is hidden

  m_handAct = new QAction(QIcon(":/images/handIcon.png"), tr("Hand"), this);
  m_handAct->setStatusTip(tr("Hand Tool"));
  m_handAct->setShortcut(QKeySequence(Qt::Key_6));
  m_handAct->setCheckable(true);
  connect(m_handAct, SIGNAL(triggered()), this, SLOT(hand()));
  this->addAction(m_handAct); // add to make shortcut work if menubar is hidden

  m_solidPatternAct = new QAction(QIcon(":/images/solidPatternIcon.png"), tr("solid line"), this);
  m_solidPatternAct->setStatusTip(tr("solid line"));
  m_solidPatternAct->setCheckable(true);
  connect(m_solidPatternAct, SIGNAL(triggered()), mainWidget, SLOT(solidPattern()));

  m_dashPatternAct = new QAction(QIcon(":/images/dashPatternIcon.png"), tr("dash line"), this);
  m_dashPatternAct->setStatusTip(tr("dash line"));
  m_dashPatternAct->setCheckable(true);
  connect(m_dashPatternAct, SIGNAL(triggered()), mainWidget, SLOT(dashPattern()));

  m_dashDotPatternAct = new QAction(QIcon(":/images/dashDotPatternIcon.png"), tr("dash dot line"), this);
  m_dashDotPatternAct->setStatusTip(tr("dash dot line"));
  m_dashDotPatternAct->setCheckable(true);
  connect(m_dashDotPatternAct, SIGNAL(triggered()), mainWidget, SLOT(dashDotPattern()));

  m_dotPatternAct = new QAction(QIcon(":/images/dotPatternIcon.png"), tr("dot line"), this);
  m_dotPatternAct->setStatusTip(tr("dot line"));
  m_dotPatternAct->setCheckable(true);
  connect(m_dotPatternAct, SIGNAL(triggered()), mainWidget, SLOT(dotPattern()));

  m_veryFinePenWidthAct = new QAction(QIcon(":/images/veryFinePenWidthIcon.png"), tr("Very Fine"), this);
  m_veryFinePenWidthAct->setStatusTip(tr("Very Fine Pen Width"));
  m_veryFinePenWidthAct->setShortcut(QKeySequence(Qt::Modifier::CTRL + Qt::Key_1));
  m_veryFinePenWidthAct->setCheckable(true);
  m_veryFinePenWidthAct->setChecked(false);
  connect(m_veryFinePenWidthAct, SIGNAL(triggered()), mainWidget, SLOT(veryFine()));

  m_finePenWidthAct = new QAction(QIcon(":/images/finePenWidthIcon.png"), tr("Fine"), this);
  m_finePenWidthAct->setStatusTip(tr("Fine Pen Width"));
  m_finePenWidthAct->setShortcut(QKeySequence(Qt::Modifier::CTRL + Qt::Key_2));
  m_finePenWidthAct->setCheckable(true);
  m_finePenWidthAct->setChecked(false);
  connect(m_finePenWidthAct, SIGNAL(triggered()), mainWidget, SLOT(fine()));

  m_mediumPenWidthAct = new QAction(QIcon(":/images/mediumPenWidthIcon.png"), tr("Medium"), this);
  m_mediumPenWidthAct->setStatusTip(tr("Medium Pen Width"));
  m_mediumPenWidthAct->setShortcut(QKeySequence(Qt::Modifier::CTRL + Qt::Key_3));
  m_mediumPenWidthAct->setCheckable(true);
  m_mediumPenWidthAct->setChecked(false);
  connect(m_mediumPenWidthAct, SIGNAL(triggered()), mainWidget, SLOT(medium()));

  m_thickPenWidthAct = new QAction(QIcon(":/images/thickPenWidthIcon.png"), tr("Thick"), this);
  m_thickPenWidthAct->setStatusTip(tr("Thick Pen Width"));
  m_thickPenWidthAct->setShortcut(QKeySequence(Qt::Modifier::CTRL + Qt::Key_4));
  m_thickPenWidthAct->setCheckable(true);
  m_thickPenWidthAct->setChecked(false);
  connect(m_thickPenWidthAct, SIGNAL(triggered()), mainWidget, SLOT(thick()));

  m_veryThickPenWidthAct = new QAction(QIcon(":/images/veryThickPenWidthIcon.png"), tr("Very Thick"), this);
  m_veryThickPenWidthAct->setStatusTip(tr("Very Thick Pen Width"));
  m_veryThickPenWidthAct->setShortcut(QKeySequence(Qt::Modifier::CTRL + Qt::Key_5));
  m_veryThickPenWidthAct->setCheckable(true);
  m_veryThickPenWidthAct->setChecked(false);
  connect(m_veryThickPenWidthAct, SIGNAL(triggered()), mainWidget, SLOT(veryThick()));

  m_toolbarAct = new QAction(tr("show Toolbar"), this);
  m_toolbarAct->setShortcut(QKeySequence(Qt::Key_T));
  m_toolbarAct->setCheckable(true);
  m_toolbarAct->setChecked(true);
  connect(m_toolbarAct, SIGNAL(triggered()), this, SLOT(toolbar()));
  this->addAction(m_toolbarAct); // add to make shortcut work if menubar is hidden

  m_statusbarAct = new QAction(tr("show Statusbar"), this);
  m_statusbarAct->setShortcut(QKeySequence(Qt::Key_S));
  m_statusbarAct->setCheckable(true);
  m_statusbarAct->setChecked(true);
  connect(m_statusbarAct, SIGNAL(triggered()), this, SLOT(statusbar()));
  this->addAction(m_statusbarAct); // add to make shortcut work if menubar is hidden

  m_fullscreenAct = new QAction(QIcon(":/images/fullscreenIcon.png"), tr("show fullscreen"), this);
  m_fullscreenAct->setShortcut(QKeySequence(Qt::Key_F));
  m_fullscreenAct->setCheckable(true);
  m_fullscreenAct->setChecked(false);
  connect(m_fullscreenAct, SIGNAL(triggered()), this, SLOT(fullscreen()));
  this->addAction(m_fullscreenAct); // add to make shortcut work if menubar is hidden

  m_maximizeAct = new QAction(tr("Maximize"), this);
  m_maximizeAct->setShortcut(QKeySequence(Qt::Key_M));
  connect(m_maximizeAct, SIGNAL(triggered()), this, SLOT(maximize()));
  this->addAction(m_maximizeAct);

  // colorActions
  m_blackAct = new QAction(QIcon(":/images/blackIcon.png"), tr("black"), this);
  m_blackAct->setShortcut(QKeySequence(Qt::Key_Q));
  m_blackAct->setStatusTip(tr("black"));
  m_blackAct->setCheckable(true);
  m_blackAct->setChecked(true);
  connect(m_blackAct, SIGNAL(triggered()), this, SLOT(black()));
  this->addAction(m_blackAct); // add to make shortcut work if menubar is hidden

  m_redAct = new QAction(QIcon(":/images/redIcon.png"), tr("red"), this);
  m_redAct->setShortcut(QKeySequence(Qt::Key_W));
  m_redAct->setStatusTip(tr("red"));
  m_redAct->setCheckable(true);
  connect(m_redAct, SIGNAL(triggered()), this, SLOT(red()));
  this->addAction(m_redAct); // add to make shortcut work if menubar is hidden

  m_greenAct = new QAction(QIcon(":/images/greenIcon.png"), tr("green"), this);
  m_greenAct->setShortcut(QKeySequence(Qt::Key_E));
  m_greenAct->setStatusTip(tr("green"));
  m_greenAct->setCheckable(true);
  connect(m_greenAct, SIGNAL(triggered()), this, SLOT(green()));
  this->addAction(m_greenAct); // add to make shortcut work if menubar is hidden

  m_blueAct = new QAction(QIcon(":/images/blueIcon.png"), tr("blue"), this);
  m_blueAct->setShortcut(QKeySequence(Qt::Key_R));
  m_blueAct->setStatusTip(tr("blue"));
  m_blueAct->setCheckable(true);
  connect(m_blueAct, SIGNAL(triggered()), this, SLOT(blue()));
  this->addAction(m_blueAct); // add to make shortcut work if menubar is hidden

  m_grayAct = new QAction(QIcon(":/images/grayIcon.png"), tr("gray"), this);
  m_grayAct->setStatusTip(tr("gray"));
  m_grayAct->setCheckable(true);
  connect(m_grayAct, SIGNAL(triggered()), this, SLOT(gray()));

  m_lightblueAct = new QAction(QIcon(":/images/lightblueIcon.png"), tr("lightblue"), this);
  m_lightblueAct->setStatusTip(tr("lightblue"));
  m_lightblueAct->setCheckable(true);
  connect(m_lightblueAct, SIGNAL(triggered()), this, SLOT(lightblue()));

  m_lightgreenAct = new QAction(QIcon(":/images/lightgreenIcon.png"), tr("lightgreen"), this);
  m_lightgreenAct->setStatusTip(tr("lightgreen"));
  m_lightgreenAct->setCheckable(true);
  connect(m_lightgreenAct, SIGNAL(triggered()), this, SLOT(lightgreen()));

  m_magentaAct = new QAction(QIcon(":/images/magentaIcon.png"), tr("magenta"), this);
  m_magentaAct->setStatusTip(tr("magenta"));
  m_magentaAct->setCheckable(true);
  connect(m_magentaAct, SIGNAL(triggered()), this, SLOT(magenta()));

  m_orangeAct = new QAction(QIcon(":/images/orangeIcon.png"), tr("orange"), this);
  m_orangeAct->setStatusTip(tr("orange"));
  m_orangeAct->setCheckable(true);
  connect(m_orangeAct, SIGNAL(triggered()), this, SLOT(orange()));

  m_yellowAct = new QAction(QIcon(":/images/yellowIcon.png"), tr("yellow"), this);
  m_yellowAct->setStatusTip(tr("yellow"));
  m_yellowAct->setCheckable(true);
  connect(m_yellowAct, SIGNAL(triggered()), this, SLOT(yellow()));

  m_whiteAct = new QAction(QIcon(":/images/whiteIcon.png"), tr("white"), this);
  m_whiteAct->setStatusTip(tr("white"));
  m_whiteAct->setCheckable(true);
  connect(m_whiteAct, SIGNAL(triggered()), this, SLOT(white()));

  m_rotateAct = new QAction(tr("Rotate"), this);
  m_rotateAct->setStatusTip("Rotate Selection");
  m_rotateAct->setShortcut(QKeySequence(Qt::Modifier::CTRL + Qt::Key_R));
  connect(m_rotateAct, SIGNAL(triggered()), this, SLOT(rotate()));
  this->addAction(m_rotateAct); // add to make shortcut work if menubar is hidden

  m_helpAct = new QAction(tr("&Help"), this);
  m_helpAct->setShortcut(QKeySequence(Qt::Key_F1));
  connect(m_helpAct, SIGNAL(triggered()), this, SLOT(help()));

  m_aboutAct = new QAction(tr("&About"), this);
  connect(m_aboutAct, SIGNAL(triggered()), this, SLOT(about()));

  m_aboutQtAct = new QAction(tr("About &Qt"), this);
  connect(m_aboutQtAct, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

  QObject::connect(scrollArea->verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(verticalScrolling()));
}

void MainWindow::createMenus()
{
  m_fileMenu = menuBar()->addMenu(tr("&File"));
  m_fileMenu->addAction(m_newWindowAct);
  m_fileMenu->addAction(m_cloneWindowAct);
  m_fileMenu->addAction(m_closeWindowAct);
  m_fileMenu->addSeparator();
  m_fileMenu->addAction(m_newFileAct);
  m_fileMenu->addAction(m_openFileAct);
  m_fileMenu->addAction(m_saveFileAct);
  m_fileMenu->addAction(m_saveFileAsAct);
  m_fileMenu->addAction(m_exportPDFAct);
  m_fileMenu->addAction(m_importXOJAct);
  m_fileMenu->addAction(m_exportXOJAct);
  m_fileMenu->addSeparator();
  m_fileMenu->addAction(m_exitAct);

  m_editMenu = menuBar()->addMenu(tr("&Edit"));
  m_editMenu->addAction(m_undoAct);
  m_editMenu->addAction(m_redoAct);
  m_editMenu->addSeparator();
  m_editMenu->addAction(m_cutAct);
  m_editMenu->addAction(m_copyAct);
  m_editMenu->addAction(m_pasteAct);
  m_editMenu->addSeparator();
  m_editMenu->addAction(m_selectAllAct);
  m_editMenu->addSeparator();
  m_editMenu->addAction(m_rotateAct);

  m_pageMenu = menuBar()->addMenu(tr("&Page"));
  m_pageMenu->addAction(m_pageAddBeforeAct);
  m_pageMenu->addAction(m_pageAddAfterAct);
  m_pageMenu->addAction(m_pageAddBeginningAct);
  m_pageMenu->addAction(m_pageAddEndAct);
  m_pageMenu->addSeparator();
  m_pageMenu->addAction(m_pageRemoveAct);
  m_pageMenu->addSeparator();
  m_pageMenu->addAction(m_pageSettingsAct);

  m_toolsMenu = menuBar()->addMenu(tr("&Tools"));
  m_toolsMenu->addAction(m_penAct);
  m_toolsMenu->addAction(m_rulerAct);
  m_toolsMenu->addAction(m_circleAct);
  m_toolsMenu->addAction(m_eraserAct);
  m_toolsMenu->addAction(m_selectAct);
  m_toolsMenu->addAction(m_handAct);
  m_toolsMenu->addSeparator();

  m_penWidthMenu = m_toolsMenu->addMenu(tr("Pen Width"));
  m_penWidthMenu->addAction(m_veryFinePenWidthAct);
  m_penWidthMenu->addAction(m_finePenWidthAct);
  m_penWidthMenu->addAction(m_mediumPenWidthAct);
  m_penWidthMenu->addAction(m_thickPenWidthAct);
  m_penWidthMenu->addAction(m_veryThickPenWidthAct);

  //    toolsMenu->addSeparator();

  m_patternMenu = m_toolsMenu->addMenu(tr("Line Pattern"));
  m_patternMenu->addAction(m_solidPatternAct);
  m_patternMenu->addAction(m_dashPatternAct);
  m_patternMenu->addAction(m_dashDotPatternAct);
  m_patternMenu->addAction(m_dotPatternAct);

  m_viewMenu = menuBar()->addMenu(tr("&View"));
  m_viewMenu->addAction(m_zoomInAct);
  m_viewMenu->addAction(m_zoomOutAct);
  m_viewMenu->addAction(m_zoomFitWidthAct);
  m_viewMenu->addAction(m_zoomFitHeightAct);
  m_viewMenu->addSeparator();
  m_viewMenu->addAction(m_toolbarAct);
  m_viewMenu->addAction(m_statusbarAct);
  m_viewMenu->addAction(m_fullscreenAct);
  m_viewMenu->addSeparator();
  m_viewMenu->addAction(m_saveMyStateAct);
  m_viewMenu->addAction(m_loadMyStateAct);

  m_helpMenu = menuBar()->addMenu(tr("&Help"));
  m_helpMenu->addAction(m_helpAct);
  m_helpMenu->addAction(m_aboutAct);
  m_helpMenu->addAction(m_aboutQtAct);
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

  m_fileToolBar = addToolBar(tr("File"));
  m_fileToolBar->setObjectName("fileToolBar");
  if (QSysInfo::productType().compare("android") == 0)
  {
    m_mainMenuButton = new QToolButton();
    m_mainMenuButton->setText("Menu");
    m_mainMenuButton->setPopupMode(QToolButton::InstantPopup);
    m_mainMenu = new QMenu("Menu");
    m_mainMenu->addMenu(m_fileMenu);
    m_mainMenu->addMenu(m_editMenu);
    m_mainMenu->addMenu(m_pageMenu);
    m_mainMenu->addMenu(m_toolsMenu);
    m_mainMenu->addMenu(m_viewMenu);
    m_mainMenu->addMenu(m_helpMenu);
    m_mainMenuButton->setMenu(m_mainMenu);
    m_fileToolBar->addWidget(m_mainMenuButton);
  }
  m_fileToolBar->addAction(m_newFileAct);
  m_fileToolBar->addAction(m_openFileAct);
  m_fileToolBar->addAction(m_saveFileAct);
  m_fileToolBar->addAction(m_exportPDFAct);
  m_fileToolBar->setIconSize(iconSize);

  m_editToolBar = addToolBar(tr("Edit"));
  m_editToolBar->setObjectName("editToolBar");
  m_editToolBar->addAction(m_cutAct);
  m_editToolBar->addAction(m_copyAct);
  m_editToolBar->addAction(m_pasteAct);
  m_editToolBar->addSeparator();
  m_editToolBar->addAction(m_undoAct);
  m_editToolBar->addAction(m_redoAct);
  m_editToolBar->setIconSize(iconSize);

  m_viewToolBar = addToolBar(tr("View"));
  m_viewToolBar->setObjectName("viewToolBar");
  m_viewToolBar->addAction(m_pageFirstAct);
  m_viewToolBar->addAction(m_pageUpAct);
  m_viewToolBar->addAction(m_pageDownAct);
  m_viewToolBar->addAction(m_pageLastAct);
  m_viewToolBar->addSeparator();
  m_viewToolBar->addAction(m_zoomOutAct);
  m_viewToolBar->addAction(m_zoomFitWidthAct);
  m_viewToolBar->addAction(m_zoomFitHeightAct);
  m_viewToolBar->addAction(m_zoomInAct);
  m_viewToolBar->addSeparator();
  m_viewToolBar->addAction(m_fullscreenAct);
  m_viewToolBar->setIconSize(iconSize);

  addToolBarBreak();

  m_toolsToolBar = addToolBar(tr("Tools"));
  m_toolsToolBar->setObjectName("toolsToolBar");
  m_toolsToolBar->addAction(m_penAct);
  m_toolsToolBar->addAction(m_rulerAct);
  m_toolsToolBar->addAction(m_circleAct);
  m_toolsToolBar->addAction(m_eraserAct);
  m_toolsToolBar->addAction(m_selectAct);
  m_toolsToolBar->addAction(m_handAct);

  m_toolsToolBar->addSeparator();

  m_toolsToolBar->addAction(m_finePenWidthAct);
  m_toolsToolBar->addAction(m_mediumPenWidthAct);
  m_toolsToolBar->addAction(m_thickPenWidthAct);

  m_toolsToolBar->addSeparator();

  m_patternToolButton = new QToolButton();
  m_patternToolButton->setMenu(m_patternMenu);
  m_patternToolButton->setToolButtonStyle(Qt::ToolButtonIconOnly);
  m_patternToolButton->setPopupMode(QToolButton::InstantPopup);
  m_toolsToolBar->addWidget(m_patternToolButton);

  m_toolsToolBar->addSeparator();

  m_toolsToolBar->addAction(m_blackAct);
  m_toolsToolBar->addAction(m_redAct);
  m_toolsToolBar->addAction(m_greenAct);
  m_toolsToolBar->addAction(m_blueAct);
  m_toolsToolBar->addAction(m_grayAct);
  m_toolsToolBar->addAction(m_lightblueAct);
  m_toolsToolBar->addAction(m_lightgreenAct);
  m_toolsToolBar->addAction(m_magentaAct);
  m_toolsToolBar->addAction(m_orangeAct);
  m_toolsToolBar->addAction(m_yellowAct);
  m_toolsToolBar->addAction(m_whiteAct);
  m_toolsToolBar->setIconSize(iconSize);
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

void MainWindow::modified()
{
  setWindowModified(mainWidget->m_currentDocument.documentChanged());
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
  if (m_toolbarAct->isChecked())
  {
    vis = true;
  }
  else
  {
    vis = false;
  }
  m_fileToolBar->setVisible(vis);
  m_editToolBar->setVisible(vis);
  m_viewToolBar->setVisible(vis);
  m_toolsToolBar->setVisible(vis);
  updateGUI();
}

void MainWindow::statusbar()
{
  bool vis;
  if (m_statusbarAct->isChecked())
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
  if (m_fullscreenAct->isChecked())
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

void MainWindow::verticalScrolling()
{
  QSize size = scrollArea->size();
  QPoint globalMousePos = QPoint(size.width() / 2.0, size.height() / 2.0) + scrollArea->pos() + this->pos();
  QPoint pos = mainWidget->mapFromGlobal(globalMousePos);
  int pageNum = mainWidget->getPageFromMousePos(pos);

  pageNum = mainWidget->getCurrentPage();

  if (pageNum == mainWidget->m_currentDocument.pages.size() - 1)
  {
    m_pageDownAct->setIcon(QIcon(":/images/pageDownPlusIcon.png"));
    m_pageDownAct->setText(tr("Page Down (add Page)"));
    m_pageDownAct->setStatusTip(tr("Page Down (add Page)"));
  }
  else
  {
    m_pageDownAct->setIcon(QIcon(":/images/pageDownIcon.png"));
    m_pageDownAct->setText(tr("Page Down"));
    m_pageDownAct->setStatusTip(tr("Page Down"));
  }

  int Npages = mainWidget->m_currentDocument.pages.size();

  QString statusMsg = QString("%1 / %2").arg(QString::number(pageNum + 1), QString::number(Npages));

  m_pageStatus.setText(statusMsg);
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
    myApp->m_mainWindows.removeOne(this);
    saveMyGeometry();
    if (myApp->m_mainWindows.isEmpty())
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

  m_blackAct->setChecked(currentColor == MrDoc::black);
  m_blueAct->setChecked(currentColor == MrDoc::blue);
  m_redAct->setChecked(currentColor == MrDoc::red);
  m_greenAct->setChecked(currentColor == MrDoc::green);
  m_grayAct->setChecked(currentColor == MrDoc::gray);
  m_lightblueAct->setChecked(currentColor == MrDoc::lightblue);
  m_lightgreenAct->setChecked(currentColor == MrDoc::lightgreen);
  m_magentaAct->setChecked(currentColor == MrDoc::magenta);
  m_orangeAct->setChecked(currentColor == MrDoc::orange);
  m_yellowAct->setChecked(currentColor == MrDoc::yellow);
  m_whiteAct->setChecked(currentColor == MrDoc::white);

  Widget::tool currentTool = mainWidget->getCurrentTool();

  m_penAct->setChecked(currentTool == Widget::tool::PEN);
  m_rulerAct->setChecked(currentTool == Widget::tool::RULER);
  m_circleAct->setChecked(currentTool == Widget::tool::CIRCLE);
  m_eraserAct->setChecked(currentTool == Widget::tool::ERASER);
  m_selectAct->setChecked(currentTool == Widget::tool::SELECT);
  m_handAct->setChecked(currentTool == Widget::tool::HAND);

  qreal currentPenWidth = mainWidget->getCurrentPenWidth();

  m_veryFinePenWidthAct->setChecked(currentPenWidth == Widget::m_veryFinePenWidth);
  m_finePenWidthAct->setChecked(currentPenWidth == Widget::m_finePenWidth);
  m_mediumPenWidthAct->setChecked(currentPenWidth == Widget::m_mediumPenWidth);
  m_thickPenWidthAct->setChecked(currentPenWidth == Widget::m_thickPenWidth);
  m_veryThickPenWidthAct->setChecked(currentPenWidth == Widget::m_veryThickPenWidth);

  QVector<qreal> currentPattern = mainWidget->getCurrentPattern();

  m_solidPatternAct->setChecked(currentPattern == MrDoc::solidLinePattern);
  m_dashPatternAct->setChecked(currentPattern == MrDoc::dashLinePattern);
  m_dashDotPatternAct->setChecked(currentPattern == MrDoc::dashDotLinePattern);
  m_dotPatternAct->setChecked(currentPattern == MrDoc::dotLinePattern);

  if (currentPattern == MrDoc::solidLinePattern)
    m_patternToolButton->setIcon(QIcon(":/images/solidPatternIcon.png"));
  if (currentPattern == MrDoc::dashLinePattern)
    m_patternToolButton->setIcon(QIcon(":/images/dashPatternIcon.png"));
  if (currentPattern == MrDoc::dashDotLinePattern)
    m_patternToolButton->setIcon(QIcon(":/images/dashDotPatternIcon.png"));
  if (currentPattern == MrDoc::dotLinePattern)
    m_patternToolButton->setIcon(QIcon(":/images/dotPatternIcon.png"));

  m_fullscreenAct->setChecked(isFullScreen());
  m_statusbarAct->setChecked(statusBar()->isVisible());

  if (mainWidget->getCurrentState() == Widget::state::SELECTED)
  {
    m_cutAct->setEnabled(true);
    m_copyAct->setEnabled(true);
  }
  else
  {
    m_cutAct->setDisabled(true);
    m_copyAct->setDisabled(true);
  }
  //    toolbarAct->setChecked()

  QPixmap pixmap(16, 16);
  pixmap.fill(mainWidget->m_currentColor);
  m_colorStatus.setPixmap(pixmap);

  m_penWidthStatus.setText(QString::number(mainWidget->m_currentPenWidth));

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
  static_cast<TabletApplication *>(qApp)->m_mainWindows.append(window);
  window->show();
}

void MainWindow::cloneWindow()
{
  MainWindow *window = new MainWindow();
  window->mainWidget->m_currentDocument = mainWidget->m_currentDocument;
  window->mainWidget->m_currentDocument.setDocName("");
  window->mainWidget->m_pageBuffer = mainWidget->m_pageBuffer;
  window->mainWidget->currentSelection = mainWidget->currentSelection;
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

  static_cast<TabletApplication *>(qApp)->m_mainWindows.append(window);
}

void MainWindow::maximize()
{
  showMaximized();
}

bool MainWindow::loadXOJ(QString fileName)
{
  return mainWidget->m_currentDocument.loadXOJ(fileName);
  updateGUI();
}

bool MainWindow::loadMOJ(QString fileName)
{
  return mainWidget->m_currentDocument.loadMOJ(fileName);
  updateGUI();
}

void MainWindow::pageSettings()
{
  int pageNum = mainWidget->getCurrentPage();
  qreal width = mainWidget->m_currentDocument.pages[pageNum].width();
  qreal height = mainWidget->m_currentDocument.pages[pageNum].height();
  PageSettingsDialog *pageDialog = new PageSettingsDialog(QSizeF(width, height), mainWidget->m_currentDocument.pages[pageNum].backgroundColor(), this);
  pageDialog->setWindowModality(Qt::WindowModal);
  if (pageDialog->exec() == QDialog::Accepted)
  {
    if (pageDialog->m_currentPageSize.isValid())
    {
      qDebug() << "valid";
      ChangePageSettingsCommand *cpsCommand = new ChangePageSettingsCommand(mainWidget, pageNum, pageDialog->m_currentPageSize, pageDialog->m_backgroundColor);
      mainWidget->m_undoStack.push(cpsCommand);
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
