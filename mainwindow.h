#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QToolButton>
#include <QScrollArea>
#include <QFontDialog>

#include "widget.h"
#include "searchbar.h"

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = 0);
  //    MainWindow();
  ~MainWindow();

  void setTitle();
  bool loadXOJ(QString fileName);
  bool loadMOJ(QString fileName);

protected:
  void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;
  void showEvent(QShowEvent *event) Q_DECL_OVERRIDE;

public slots:
  void updateGUI();

private slots:
  void newWindow();
  void cloneWindow();
  void newFile();
  void openPdf();
  void openFile();

  bool saveFileAs();
  bool saveFile();
  void exportPDF();

  void importXOJ();
  bool exportXOJ();

  void exit();

  void zoomIn();
  void zoomOut();
  void zoomFitWidth();
  void zoomFitHeight();

  void pen();
  void highlighter();
  void ruler();
  void circle();
  void eraser();
  void select();
  void hand();
  void text();

  void selectFont();

  void modified();

  void toolbar();
  void statusbar();
  void fullscreen();
  void maximize();
  void horizontalView();
  void verticalView();

  void pageSettings();

  void black();
  void blue();
  void red();
  void green();
  void gray();
  void lightblue();
  void lightgreen();
  void magenta();
  void orange();
  void yellow();
  void white();

  void rotate();

  void help();
  void about();

  void saveMyState();
  void loadMyState();

  void saveMyGeometry();
  void loadMyGeometry();

  void scrolling();

  bool maybeSave();

private:
  // widgets
  Widget *mainWidget;
  QScrollArea *scrollArea;

  void createActions();
  void createToolBars();
  void createMenus();

  QString askForFileName();

  QLabel pageStatus;
  QLabel penWidthStatus;
  QLabel colorStatus;

  SearchBar* searchBar;

  // actions
  QAction *newWindowAct;
  QAction *cloneWindowAct;
  QAction *closeWindowAct;
  QAction *newFileAct;
  QAction *annotatePdfAct;
  QAction *openFileAct;
  QAction *saveFileAct;
  QAction *saveFileAsAct;
  QAction *exportPDFAct;
  QAction *exitAct;

  QAction *importXOJAct;
  QAction *exportXOJAct;

  QAction *undoAct;
  QAction *redoAct;

  QAction *selectAllAct;
  QAction *copyAct;
  QAction *pasteAct;
  QAction *cutAct;

  QAction *zoomInAct;
  QAction *zoomOutAct;
  QAction *zoomFitWidthAct;
  QAction *zoomFitHeightAct;

  QAction *pageFirstAct;
  QAction *pageLastAct;
  QAction *pageUpAct;
  QAction *pageDownAct;

  QAction *pageAddEndAct;
  QAction *pageAddBeginningAct;
  QAction *pageAddBeforeAct;
  QAction *pageAddAfterAct;

  QAction *pageRemoveAct;

  QAction *pageSettingsAct;

  QAction *penAct;
  QAction *highlighterAct;
  QAction *rulerAct;
  QAction *circleAct;
  QAction *eraserAct;
  QAction *selectAct;
  QAction *handAct;

  QAction *solidPatternAct;
  QAction *dashPatternAct;
  QAction *dashDotPatternAct;
  QAction *dotPatternAct;

  QAction *rotateAct;

  QAction *textAct;
  QAction *fontAct;

  QAction *veryFinePenWidthAct;
  QAction *finePenWidthAct;
  QAction *mediumPenWidthAct;
  QAction *thickPenWidthAct;
  QAction *veryThickPenWidthAct;

  QAction *toolbarAct;
  QAction *statusbarAct;
  QAction *fullscreenAct;
  QAction *maximizeAct;
  QAction *horizontalViewAct;
  QAction *verticalViewAct;
  QMenu *viewOrientationMenu;

  // color actions
  QAction *blackAct;
  QAction *blueAct;
  QAction *redAct;
  QAction *greenAct;
  QAction *grayAct;
  QAction *lightblueAct;
  QAction *lightgreenAct;
  QAction *magentaAct;
  QAction *orangeAct;
  QAction *yellowAct;
  QAction *whiteAct;

  QAction *helpAct;
  QAction *aboutAct;
  QAction *aboutQtAct;

  QAction *saveMyStateAct;
  QAction *loadMyStateAct;

  QToolBar *fileToolBar;
  QToolBar *editToolBar;
  QToolBar *viewToolBar;
  QToolBar *toolsToolBar;

  QToolButton *patternToolButton;

  QMenu *fileMenu;
  QMenu *editMenu;
  QMenu *pageMenu;
  QMenu *toolsMenu;
  QMenu *penWidthMenu;
  QMenu *patternMenu;
  QMenu *viewMenu;
  QMenu *helpMenu;

  // for android
  QToolButton *mainMenuButton;
  QMenu *mainMenu;
};

#endif // MAINWINDOW_H
