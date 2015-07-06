#include "widget.h"
//#include <QtWidgets>
#include <QScrollArea>
#include <QScrollBar>
#include <QAction>
#include <QFileDialog>
#include <QMenuBar>
#include <QToolBar>
#include <QMessageBox>
#include <QStatusBar>
#include <qdebug.h>

#include <iostream>

#include "document.h"
#include "version.h"
#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    this->resize(1024,768);

    mainWidget = new Widget(this);
    connect(mainWidget, SIGNAL(select()), this, SLOT(select()));
    connect(mainWidget, SIGNAL(pen()), this, SLOT(pen()));
    connect(mainWidget, SIGNAL(eraser()), this, SLOT(eraser()));
    connect(mainWidget, SIGNAL(hand()), this, SLOT(hand()));

    connect(mainWidget, SIGNAL(modified()), this, SLOT(modified()));

    scrollArea = new QScrollArea(this);
    scrollArea->setWidget(mainWidget);
    scrollArea->setAlignment(Qt::AlignHCenter);
//    mainWidget->setGeometry(0,0,100,100);
//    scrollArea->setWidgetResizable(true);

    setCentralWidget(scrollArea);

//    setCentralWidget(mainWidget);

    mainWidget->scrollArea = scrollArea;

    statusBar()->addPermanentWidget(&pageStatus);
    pageStatus.setText("1 / 1");

    createActions();
    createToolBars();
    createMenus();

    mainWidget->zoomTo(1.0);
    verticalScrolling();

    setTitle();
}

MainWindow::~MainWindow()
{
    //delete ui;
}

void MainWindow::setTitle()
{
    QString docName;
    if (mainWidget->currentDocument->getDocName().isEmpty())
    {
        docName = tr("untitled");
    } else {
        docName = mainWidget->currentDocument->getDocName();
    }
    QString title = MY_PRODUCT_NAME;
    title.append(" - ");
    title.append(docName);
    setWindowTitle(title);
}

void MainWindow::createActions()
{
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

    exportPDFAct = new QAction(QIcon(":/images/savePDFIcon.png"), tr("Export PDF"), this);
//    exportPDFAct->setShortcuts(QKeySequence(Qt::CTRL + Qt::Key_E));
    exportPDFAct->setStatusTip(tr("Export PDF"));
    connect(exportPDFAct, SIGNAL(triggered()), this, SLOT(exportPDF()));

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
    zoomInAct->setStatusTip(tr("Zoom In"));
    connect(zoomInAct, SIGNAL(triggered()), this, SLOT(zoomIn()));
    this->addAction(zoomInAct); // add to make shortcut work if menubar is hidden

    zoomOutAct = new QAction(QIcon(":/images/zoomOutIcon.png"), tr("Zoom &Out"), this); // QAction(QIcon(":/images/new.png"), tr("&New"), this);
    zoomOutAct->setStatusTip(tr("Zoom Out"));
    connect(zoomOutAct, SIGNAL(triggered()), this, SLOT(zoomOut()));
    this->addAction(zoomOutAct); // add to make shortcut work if menubar is hidden

    zoomFitWidthAct = new QAction(QIcon(":/images/zoomFitWidthIcon.png"), tr("Zoom to fit width"), this); // QAction(QIcon(":/images/new.png"), tr("&New"), this);
    zoomFitWidthAct->setStatusTip(tr("Zoom to fit width"));
    connect(zoomFitWidthAct, SIGNAL(triggered()), this, SLOT(zoomFitWidth()));
    this->addAction(zoomFitWidthAct); // add to make shortcut work if menubar is hidden

    pageFirstAct = new QAction(QIcon(":/images/pageFirstIcon.png"), tr("First Page"), this);
    pageFirstAct->setStatusTip(tr("First Page"));
    connect(pageFirstAct, SIGNAL(triggered()), mainWidget, SLOT(pageFirst()));

    pageLastAct = new QAction(QIcon(":/images/pageLastIcon.png"), tr("Last Page"), this);
    pageLastAct->setStatusTip(tr("Last Page"));
    connect(pageLastAct, SIGNAL(triggered()), mainWidget, SLOT(pageLast()));

    pageUpAct = new QAction(QIcon(":/images/pageUpIcon.png"), tr("Page &Up"), this);
    pageUpAct->setStatusTip(tr("Page Up"));
    connect(pageUpAct, SIGNAL(triggered()), mainWidget, SLOT(pageUp()));

    pageDownAct = new QAction(QIcon(":/images/pageDownIcon.png"), tr("Page &Down"), this);
    pageDownAct->setStatusTip(tr("Page Down"));
    connect(pageDownAct, SIGNAL(triggered()), mainWidget, SLOT(pageDown()));

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

    penAct = new QAction(QIcon(":/images/penIcon.png"), tr("Pen"), this);
    penAct->setStatusTip(tr("Pen Tool"));
    penAct->setCheckable(true);
    penAct->setChecked(true);
    connect(penAct, SIGNAL(triggered()), this, SLOT(pen()));
    this->addAction(penAct); // add to make shortcut work if menubar is hidden

    eraserAct = new QAction(QIcon(":/images/eraserIcon.png"), tr("Eraser"), this);
    eraserAct->setStatusTip(tr("Eraser Tool"));
    eraserAct->setCheckable(true);
    connect(eraserAct, SIGNAL(triggered()), this, SLOT(eraser()));
    this->addAction(eraserAct); // add to make shortcut work if menubar is hidden

    selectAct = new QAction(QIcon(":/images/selectIcon.png"), tr("Select"), this);
    selectAct->setStatusTip(tr("Select Tool"));
    selectAct->setCheckable(true);
    connect(selectAct, SIGNAL(triggered()), this, SLOT(select()));
    this->addAction(selectAct); // add to make shortcut work if menubar is hidden

    handAct = new QAction(QIcon(":/images/handIcon.png"), tr("Hand"), this);
    handAct->setStatusTip(tr("Hand Tool"));
    handAct->setCheckable(true);
    connect(handAct, SIGNAL(triggered()), this, SLOT(hand()));
    this->addAction(handAct); // add to make shortcut work if menubar is hidden

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

    fullscreenAct = new QAction(tr("show fullscreen"), this);
    fullscreenAct->setShortcut(QKeySequence(Qt::Key_F));
    fullscreenAct->setCheckable(true);
    fullscreenAct->setChecked(false);
    connect(fullscreenAct, SIGNAL(triggered()), this, SLOT(fullscreen()));
    this->addAction(fullscreenAct); // add to make shortcut work if menubar is hidden

    // colorActions
    blackAct = new QAction(QIcon(":/images/blackIcon.png"), tr("black"), this);
    blackAct->setStatusTip(tr("black"));
    blackAct->setCheckable(true);
    blackAct->setChecked(true);
    connect(blackAct, SIGNAL(triggered()), this, SLOT(black()));

    redAct = new QAction(QIcon(":/images/redIcon.png"), tr("red"), this);
    redAct->setStatusTip(tr("red"));
    redAct->setCheckable(true);
    connect(redAct, SIGNAL(triggered()), this, SLOT(red()));

    blueAct = new QAction(QIcon(":/images/blueIcon.png"), tr("blue"), this);
    blueAct->setStatusTip(tr("blue"));
    blueAct->setCheckable(true);
    connect(blueAct, SIGNAL(triggered()), this, SLOT(blue()));

    greenAct = new QAction(QIcon(":/images/greenIcon.png"), tr("green"), this);
    greenAct->setStatusTip(tr("green"));
    greenAct->setCheckable(true);
    connect(greenAct, SIGNAL(triggered()), this, SLOT(green()));

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

    aboutAct = new QAction(tr("&About"), this);
    connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));

    QObject::connect(scrollArea->verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(verticalScrolling()));
}

void MainWindow::createMenus()
{
    fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(newFileAct);
    fileMenu->addAction(openFileAct);
    fileMenu->addAction(saveFileAct);
    fileMenu->addAction(exportPDFAct);

    editMenu = menuBar()->addMenu(tr("&Edit"));
    editMenu->addAction(undoAct);
    editMenu->addAction(redoAct);
    editMenu->addSeparator();
    editMenu->addAction(cutAct);
    editMenu->addAction(copyAct);
    editMenu->addAction(pasteAct);

    pageMenu = menuBar()->addMenu(tr("&Page"));
    pageMenu->addAction(pageAddBeforeAct);
    pageMenu->addAction(pageAddAfterAct);
    pageMenu->addAction(pageAddBeginningAct);
    pageMenu->addAction(pageAddEndAct);
    pageMenu->addSeparator();
    pageMenu->addAction(pageRemoveAct);

    viewMenu = menuBar()->addMenu(tr("&View"));
    viewMenu->addAction(zoomInAct);
    viewMenu->addAction(zoomOutAct);
    viewMenu->addAction(zoomFitWidthAct);
    viewMenu->addSeparator();
    viewMenu->addAction(toolbarAct);
    viewMenu->addAction(statusbarAct);
    viewMenu->addAction(fullscreenAct);

    helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(aboutAct);
}

void MainWindow::createToolBars()
{
    QSize iconSize = QSize(24,24);

    fileToolBar = addToolBar(tr("File"));
    fileToolBar->addAction(newFileAct);
    fileToolBar->addAction(openFileAct);
    fileToolBar->addAction(saveFileAct);
    fileToolBar->addAction(exportPDFAct);
    fileToolBar->setIconSize(iconSize);

    editToolBar = addToolBar(tr("Edit"));
    editToolBar->addAction(cutAct);
    editToolBar->addAction(copyAct);
    editToolBar->addAction(pasteAct);
    editToolBar->addSeparator();
    editToolBar->addAction(undoAct);
    editToolBar->addAction(redoAct);
    editToolBar->setIconSize(iconSize);

    viewToolBar = addToolBar(tr("View"));
    viewToolBar->addAction(zoomOutAct);
    viewToolBar->addAction(zoomFitWidthAct);
    viewToolBar->addAction(zoomInAct);
    viewToolBar->setIconSize(iconSize);
    viewToolBar->addSeparator();
    viewToolBar->addAction(pageFirstAct);
    viewToolBar->addAction(pageUpAct);
    viewToolBar->addAction(pageDownAct);
    viewToolBar->addAction(pageLastAct);

    toolsToolBar = addToolBar(tr("Tools"));
    toolsToolBar->addAction(penAct);
    toolsToolBar->addAction(eraserAct);
    toolsToolBar->addAction(selectAct);
    toolsToolBar->addAction(handAct);
    toolsToolBar->addSeparator();
    toolsToolBar->addAction(blackAct);
    toolsToolBar->addAction(redAct);
    toolsToolBar->addAction(blueAct);
    toolsToolBar->addAction(greenAct);
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
    } else {
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

    if (mainWidget->currentDocument->getPath().isEmpty())
    {
        dir = QDir::homePath();
    } else {
        dir = mainWidget->currentDocument->getPath();
    }

    QString fileName = QFileDialog::getOpenFileName(this, tr("Open XOJ"), dir, tr("Xournal Files (*.xoj)"));

    if (fileName.isNull())
    {
        return;
    }

    Document* openDocument = new Document();

    if(openDocument->loadXOJ(fileName))
    {
        mainWidget->setDocument(openDocument);
        setTitle();
        modified();
    } else {
        delete openDocument;
        QMessageBox errMsgBox;
        errMsgBox.setText("Couldn't open file");
        errMsgBox.exec();
    }

}

bool MainWindow::saveFile()
{
    QString dir;
    QString fileName;
    if (mainWidget->currentDocument->getDocName().isEmpty())
    {
        dir = QDir::homePath();
        fileName = QFileDialog::getSaveFileName(this, tr("Save XOJ"), dir, tr("Xournal Files (*.xoj)"));
    } else {
        dir = mainWidget->currentDocument->getPath();
        dir.append('/');
        dir.append(mainWidget->currentDocument->getDocName());
        dir.append(".xoj");
        fileName = dir;
    }

    if (fileName.isNull())
    {
        return false;
    }

    if (mainWidget->currentDocument->saveXOJ(fileName))
    {
        modified();
        setTitle();
        return true;
    } else {
        return false;
    }
}

void MainWindow::exportPDF()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Export PDF"), QDir::homePath(), tr("Adobe PDF files (*.PDF)"));

    if (fileName.isNull())
    {
        return;
    }

    mainWidget->currentDocument->exportPDF(fileName);
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

void MainWindow::pen()
{
    mainWidget->setCurrentTool(Widget::tool::PEN);
    penAct->setChecked(true);
    eraserAct->setChecked(false);
    selectAct->setChecked(false);
    handAct->setChecked(false);
}

void MainWindow::eraser()
{
    mainWidget->setCurrentTool(Widget::tool::ERASER);
    penAct->setChecked(false);
    eraserAct->setChecked(true);
    selectAct->setChecked(false);
    handAct->setChecked(false);
}

void MainWindow::select()
{
    mainWidget->setCurrentTool(Widget::tool::SELECT);
    penAct->setChecked(false);
    eraserAct->setChecked(false);
    selectAct->setChecked(true);
    handAct->setChecked(false);
}

void MainWindow::hand()
{
    mainWidget->setCurrentTool(Widget::tool::HAND);
    penAct->setChecked(false);
    eraserAct->setChecked(false);
    selectAct->setChecked(false);
    handAct->setChecked(true);
}

void MainWindow::modified()
{
    setWindowModified(mainWidget->currentDocument->getDocumentChanged());
}

void MainWindow::black()
{
    blackAct->setChecked(true);
    blueAct->setChecked(false);
    redAct->setChecked(false);
    greenAct->setChecked(false);
    grayAct->setChecked(false);
    lightblueAct->setChecked(false);
    lightgreenAct->setChecked(false);
    magentaAct->setChecked(false);
    orangeAct->setChecked(false);
    yellowAct->setChecked(false);
    whiteAct->setChecked(false);

    mainWidget->setCurrentColor(Document::black);
}

void MainWindow::blue()
{
    blackAct->setChecked(false);
    blueAct->setChecked(true);
    redAct->setChecked(false);
    greenAct->setChecked(false);
    grayAct->setChecked(false);
    lightblueAct->setChecked(false);
    lightgreenAct->setChecked(false);
    magentaAct->setChecked(false);
    orangeAct->setChecked(false);
    yellowAct->setChecked(false);
    whiteAct->setChecked(false);

    mainWidget->setCurrentColor(Document::blue);
}

void MainWindow::red()
{
    blackAct->setChecked(false);
    blueAct->setChecked(false);
    redAct->setChecked(true);
    greenAct->setChecked(false);
    grayAct->setChecked(false);
    lightblueAct->setChecked(false);
    lightgreenAct->setChecked(false);
    magentaAct->setChecked(false);
    orangeAct->setChecked(false);
    yellowAct->setChecked(false);
    whiteAct->setChecked(false);

    mainWidget->setCurrentColor(Document::red);
}

void MainWindow::green()
{
    blackAct->setChecked(false);
    blueAct->setChecked(false);
    redAct->setChecked(false);
    greenAct->setChecked(true);
    grayAct->setChecked(false);
    lightblueAct->setChecked(false);
    lightgreenAct->setChecked(false);
    magentaAct->setChecked(false);
    orangeAct->setChecked(false);
    yellowAct->setChecked(false);
    whiteAct->setChecked(false);

    mainWidget->setCurrentColor(Document::green);
}

void MainWindow::gray()
{
    blackAct->setChecked(false);
    blueAct->setChecked(false);
    redAct->setChecked(false);
    greenAct->setChecked(false);
    grayAct->setChecked(true);
    lightblueAct->setChecked(false);
    lightgreenAct->setChecked(false);
    magentaAct->setChecked(false);
    orangeAct->setChecked(false);
    yellowAct->setChecked(false);
    whiteAct->setChecked(false);

    mainWidget->setCurrentColor(Document::gray);
}

void MainWindow::lightblue()
{
    blackAct->setChecked(false);
    blueAct->setChecked(false);
    redAct->setChecked(false);
    greenAct->setChecked(false);
    grayAct->setChecked(false);
    lightblueAct->setChecked(true);
    lightgreenAct->setChecked(false);
    magentaAct->setChecked(false);
    orangeAct->setChecked(false);
    yellowAct->setChecked(false);
    whiteAct->setChecked(false);

    mainWidget->setCurrentColor(Document::lightblue);
}

void MainWindow::lightgreen()
{
    blackAct->setChecked(false);
    blueAct->setChecked(false);
    redAct->setChecked(false);
    greenAct->setChecked(false);
    grayAct->setChecked(false);
    lightblueAct->setChecked(false);
    lightgreenAct->setChecked(true);
    magentaAct->setChecked(false);
    orangeAct->setChecked(false);
    yellowAct->setChecked(false);
    whiteAct->setChecked(false);

    mainWidget->setCurrentColor(Document::lightgreen);
}

void MainWindow::magenta()
{
    blackAct->setChecked(false);
    blueAct->setChecked(false);
    redAct->setChecked(false);
    greenAct->setChecked(false);
    grayAct->setChecked(false);
    lightblueAct->setChecked(false);
    lightgreenAct->setChecked(false);
    magentaAct->setChecked(true);
    orangeAct->setChecked(false);
    yellowAct->setChecked(false);
    whiteAct->setChecked(false);

    mainWidget->setCurrentColor(Document::magenta);
}

void MainWindow::orange()
{
    blackAct->setChecked(false);
    blueAct->setChecked(false);
    redAct->setChecked(false);
    greenAct->setChecked(false);
    grayAct->setChecked(false);
    lightblueAct->setChecked(false);
    lightgreenAct->setChecked(false);
    magentaAct->setChecked(false);
    orangeAct->setChecked(true);
    yellowAct->setChecked(false);
    whiteAct->setChecked(false);

    mainWidget->setCurrentColor(Document::orange);
}

void MainWindow::yellow()
{
    blackAct->setChecked(false);
    blueAct->setChecked(false);
    redAct->setChecked(false);
    greenAct->setChecked(false);
    grayAct->setChecked(false);
    lightblueAct->setChecked(false);
    lightgreenAct->setChecked(false);
    magentaAct->setChecked(false);
    orangeAct->setChecked(false);
    yellowAct->setChecked(true);
    whiteAct->setChecked(false);

    mainWidget->setCurrentColor(Document::yellow);
}

void MainWindow::white()
{
    blackAct->setChecked(false);
    blueAct->setChecked(false);
    redAct->setChecked(false);
    greenAct->setChecked(false);
    grayAct->setChecked(false);
    lightblueAct->setChecked(false);
    lightgreenAct->setChecked(false);
    magentaAct->setChecked(false);
    orangeAct->setChecked(false);
    yellowAct->setChecked(false);
    whiteAct->setChecked(true);

    mainWidget->setCurrentColor(Document::white);
}

void MainWindow::about()
{
    QString version;
    version = version.append(QString::number(MY_MAJOR_VERSION));
    version = version.append(".");
    version = version.append(QString::number(MY_MINOR_VERSION));
    QMessageBox msgBox(this);
    msgBox.setWindowTitle("About");
    msgBox.setTextFormat(Qt::RichText);   //this is what makes the links clickable
    QString aboutText;
    aboutText = aboutText.append("<center>");
    aboutText = aboutText.append(MY_PRODUCT_NAME);
    aboutText = aboutText.append(" ");
    aboutText = aboutText.append(version);
    aboutText = aboutText.append("<br/><br/>Written by Thomas Leitz<br/><br/><a href='http://www.unruhschuh.com'>unruhschuh.com/MrWriter</a></center>");
    msgBox.setText(aboutText);
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.setIconPixmap(QIcon(":/images/Icon1024.png").pixmap(QSize(100,100)));
    msgBox.exec();
}


void MainWindow::toolbar()
{
    bool vis;
    if (toolbarAct->isChecked())
    {
        vis = true;
    } else {
        vis = false;
    }
    fileToolBar->setVisible(vis);
    editToolBar->setVisible(vis);
    viewToolBar->setVisible(vis);
    toolsToolBar->setVisible(vis);
}

void MainWindow::statusbar()
{
    bool vis;
    if (statusbarAct->isChecked())
    {
        vis = true;
    } else {
        vis = false;
    }
    statusBar()->setVisible(vis);
}

void MainWindow::fullscreen()
{
    if (fullscreenAct->isChecked())
    {
        showFullScreen();
        menuBar()->hide();
    } else {
        showNormal();
        menuBar()->show();
    }
}


void MainWindow::verticalScrolling()
{
    QSize size = scrollArea->size();
    QPoint globalMousePos = QPoint(size.width()/2.0, size.height()/2.0) + scrollArea->pos() + this->pos();
    QPoint pos = mainWidget->mapFromGlobal(globalMousePos);
    int pageNum = mainWidget->getPageFromMousePos(pos);

    pageNum = mainWidget->getCurrentPage();

    if (pageNum == mainWidget->currentDocument->pages.size()-1)
    {
        pageDownAct->setIcon(QIcon(":/images/pageDownPlusIcon.png"));
        pageDownAct->setText(tr("Page Down (add Page)"));
        pageDownAct->setStatusTip(tr("Page Down (add Page)"));
    } else {
        pageDownAct->setIcon(QIcon(":/images/pageDownIcon.png"));
        pageDownAct->setText(tr("Page Down"));
        pageDownAct->setStatusTip(tr("Page Down"));
    }

    int Npages = mainWidget->currentDocument->pages.size();

    QString statusMsg = QString("%1 / %2").arg(QString::number(pageNum+1), QString::number(Npages));

    pageStatus.setText(statusMsg);
//    statusBar()->showMessage(statusMsg);

//    std::cout << "width: " << globalMousePos.x() << ", height: " << globalMousePos.y() << ", page: " << pageNum << std::endl;

}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (maybeSave())
    {
        event->accept();
        QGuiApplication::exit(0); // fix for calling closeEvent twice, see https://bugreports.qt.io/browse/QTBUG-43344
    } else {
        event->ignore();
    }
}

bool MainWindow::maybeSave()
{
    if (mainWidget->currentDocument->getDocumentChanged()) {
        QMessageBox::StandardButton ret;
        ret = QMessageBox::warning(this, tr("Application"),
                     tr("The document has been modified.\n"
                        "Do you want to save your changes?"),
                     QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        if (ret == QMessageBox::Save)
            return saveFile();
        else if (ret == QMessageBox::Cancel)
            return false;
    }
    return true;
}
