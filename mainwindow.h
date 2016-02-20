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
  void eraser();
  void select();
  void hand();

  void modified();

  void toolbar();
  void statusbar();
  void fullscreen();
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

  void createActions();
  void createToolBars();
  void createMenus();

  QString askForFileName();

  QLabel m_pageStatus;
  QLabel m_penWidthStatus;
  QLabel m_colorStatus;

  // actions
  QAction *m_newWindowAct;
  QAction *m_cloneWindowAct;
  QAction *m_closeWindowAct;
  QAction *m_newFileAct;
  QAction *m_openFileAct;
  QAction *m_saveFileAct;
  QAction *m_saveFileAsAct;
  QAction *m_exportPDFAct;
  QAction *m_exitAct;

  QAction *m_importXOJAct;
  QAction *m_exportXOJAct;

  QAction *m_undoAct;
  QAction *m_redoAct;

  QAction *m_selectAllAct;
  QAction *m_copyAct;
  QAction *m_pasteAct;
  QAction *m_cutAct;

  QAction *m_zoomInAct;
  QAction *m_zoomOutAct;
  QAction *m_zoomFitWidthAct;
  QAction *m_zoomFitHeightAct;

  QAction *m_pageFirstAct;
  QAction *m_pageLastAct;
  QAction *m_pageUpAct;
  QAction *m_pageDownAct;

  QAction *m_pageAddEndAct;
  QAction *m_pageAddBeginningAct;
  QAction *m_pageAddBeforeAct;
  QAction *m_pageAddAfterAct;

  QAction *m_pageRemoveAct;

  QAction *m_pageSettingsAct;

  QAction *m_penAct;
  QAction *m_rulerAct;
  QAction *m_circleAct;
  QAction *m_eraserAct;
  QAction *m_selectAct;
  QAction *m_handAct;

  QAction *m_solidPatternAct;
  QAction *m_dashPatternAct;
  QAction *m_dashDotPatternAct;
  QAction *m_dotPatternAct;

  QAction *m_rotateAct;

  QAction *m_veryFinePenWidthAct;
  QAction *m_finePenWidthAct;
  QAction *m_mediumPenWidthAct;
  QAction *m_thickPenWidthAct;
  QAction *m_veryThickPenWidthAct;

  QAction *m_toolbarAct;
  QAction *m_statusbarAct;
  QAction *m_fullscreenAct;
  QAction *m_maximizeAct;

  // color actions
  QAction *m_blackAct;
  QAction *m_blueAct;
  QAction *m_redAct;
  QAction *m_greenAct;
  QAction *m_grayAct;
  QAction *m_lightblueAct;
  QAction *m_lightgreenAct;
  QAction *m_magentaAct;
  QAction *m_orangeAct;
  QAction *m_yellowAct;
  QAction *m_whiteAct;

  QAction *m_helpAct;
  QAction *m_aboutAct;
  QAction *m_aboutQtAct;

  QAction *m_saveMyStateAct;
  QAction *m_loadMyStateAct;

  QToolBar *m_fileToolBar;
  QToolBar *m_editToolBar;
  QToolBar *m_viewToolBar;
  QToolBar *m_toolsToolBar;

  QToolButton *m_patternToolButton;

  QMenu *m_fileMenu;
  QMenu *m_editMenu;
  QMenu *m_pageMenu;
  QMenu *m_toolsMenu;
  QMenu *m_penWidthMenu;
  QMenu *m_patternMenu;
  QMenu *m_viewMenu;
  QMenu *m_helpMenu;

  // for android
  QToolButton *m_mainMenuButton;
  QMenu *m_mainMenu;
};

#endif // MAINWINDOW_H
