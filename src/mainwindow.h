#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QToolButton>
#include <QScrollArea>

#include "widget.h"

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
  void ruler();
  void circle();
  void rect();
  void eraser();
  void strokeEraser();
  void select();
  void rectSelect();
  void hand();

  void modified();

  void toolbar();
  void statusbar();
  void fullscreen();
  void showGrid();
  void snapToGrid();
  void maximize();

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

  void verticalScrolling();

  bool maybeSave();

private:
  // widgets
  Widget *mainWidget;
  QScrollArea *scrollArea;

  QDialog *quickmenu;

  void createActions();
  void createToolBars();
  void createMenus();

  QString askForFileName();

  QLabel statusStatus;
  QLabel pageStatus;
  QLabel penWidthStatus;
  QLabel colorStatus;

  // actions
  QAction *newWindowAct;
  QAction *cloneWindowAct;
  QAction *closeWindowAct;
  QAction *newFileAct;
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
  QAction *deleteAct;

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
  QAction *rulerAct;
  QAction *circleAct;
  QAction *rectAct;
  QAction *eraserAct;
  QAction *strokeEraserAct;
  QAction *selectAct;
  QAction *rectSelectAct;
  QAction *handAct;

  QAction *solidPatternAct;
  QAction *dashPatternAct;
  QAction *dashDotPatternAct;
  QAction *dotPatternAct;

  QAction *rotateAct;

  QAction *veryFinePenWidthAct;
  QAction *finePenWidthAct;
  QAction *mediumPenWidthAct;
  QAction *thickPenWidthAct;
  QAction *veryThickPenWidthAct;

  QAction *pencilIconAct;
  QAction *dotIconAct;

  QAction *toolbarAct;
  QAction *statusbarAct;
  QAction *fullscreenAct;
  QAction *maximizeAct;

  // grid actions
  QAction *showGridAct;
  QAction *snapToGridAct;

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
  QToolButton *eraserToolButton;
  QToolButton *selectToolButton;

  QMenu *fileMenu;
  QMenu *editMenu;
  QMenu *pageMenu;
  QMenu *toolsMenu;
  QMenu *penWidthMenu;
  QMenu *patternMenu;
  QMenu *viewMenu;
  QMenu *helpMenu;
  QMenu *penIconMenu;

  // for android
  QToolButton *mainMenuButton;
  QMenu *mainMenu;
};

#endif // MAINWINDOW_H
