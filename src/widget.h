#ifndef WIDGET_H
#define WIDGET_H

#define MIN_ZOOM 0.1
#define MAX_ZOOM 10.0

// #define INVISIBLE_BUFFER_FACTOR 10.0

#include <QWidget>
#include <QOpenGLWidget>
#include <QVector>
#include <QPainter>
#include <QTabletEvent>
#include <QUndoStack>
#include <QScrollArea>
#include <QMutex>

#include <QTime>
#include <QTimer>

#include "tabletapplication.h"
#include "mrdoc.h"
#include "document.h"
#include "stroke.h"

class Widget : public QWidget
{
  Q_OBJECT
public:
  explicit Widget(QWidget *parent = nullptr);

  enum class tool
  {
    NONE,
    PEN,
    RULER,
    CIRCLE,
    RECT,
    TEXT,
    ERASER,
    STROKE_ERASER,
    SELECT,
    RECT_SELECT,
    HAND
  };
  enum class state
  {
    IDLE,
    DRAWING,
    RULING,
    CIRCLING,
    RECTING,
    TEXTING,
    SELECTING,
    SELECTED,
    MOVING_SELECTION,
    RESIZING_SELECTION,
    ROTATING_SELECTION
  };
  enum class cursor
  {
    PENCIL,
    DOT
  };

  static constexpr qreal veryFinePenWidth = 0.42;
  static constexpr qreal finePenWidth = 0.85;
  static constexpr qreal mediumPenWidth = 1.41;
  static constexpr qreal thickPenWidth = 2.26;
  static constexpr qreal veryThickPenWidth = 5.67;

  void setCurrentTool(tool toolID);
  void setPenCursor(const QString &resourceName);

  inline tool getCurrentTool()
  {
    return currentTool;
  }

  void setCurrentPenWidth(qreal penWidth);

  qreal getCurrentPenWidth()
  {
    return currentPenWidth;
  }

  void setCurrentPenCursor(Widget::cursor cursorType);

  Widget::cursor getCurrentPenCursor()
  {
    return currentPenCursor;
  }

  void mouseAndTabletEvent(QPointF mousePos, Qt::MouseButton button, Qt::MouseButtons buttons, Qt::KeyboardModifiers keyboardModifiers,
                           QTabletEvent::PointerType pointerType, QEvent::Type eventType, qreal pressure, bool tabletEvent);

  bool pageVisible(size_t buffNum) const;
  size_t firstVisiblePage() const;
  size_t lastVisiblePage() const;
  /**
   * @brief updateAllPageBuffers
   * @todo use std::unique_lock
   */
  void updateAllPageBuffers(bool force = false);
  void updateImageBuffer(size_t buffNum);
  void updateBuffer(size_t i);
  void updateBufferRegion(size_t buffNum, QRectF const &clipRect);
  void drawGrid(QPainter &painter, size_t buffNum);
  void drawOnBuffer(bool last = false);
  size_t getPageFromMousePos(QPointF mousePos) const;
  QPointF getPagePosFromMousePos(QPointF mousePos, size_t pageNum) const;
  QPointF getAbsolutePagePosFromMousePos(QPointF mousePos) const;
  QRect getWidgetGeometry() const;
  size_t getCurrentPage() const;

  void setCurrentState(state newState);
  state getCurrentState();

  void setCurrentColor(QColor newColor);
  QColor getCurrentColor();

  void setDocument(const MrDoc::Document &newDocument);

  void newFile();
  //    void openFile();

  void zoomIn();
  void zoomOut();
  void zoomTo(qreal newZoom);
  void zoomFitWidth();
  void zoomFitHeight();

  void toggleGrid();
  void toggleSnapToGrid();

  bool showingGrid();
  bool snappingToGrid();

  void rotateSelection(qreal angle);

  MrDoc::Document currentDocument;
  std::vector<QPixmap> pageBuffer;
  std::vector<QImage> pageImageBuffer;
  QMutex pageImageBufferMutex;

  QColor currentColor;
  qreal currentPenWidth;

  Widget::cursor currentPenCursor;

  qreal m_currentAngle;

  void setCurrentPattern(QVector<qreal> newPattern);
  QVector<qreal> getCurrentPattern();

  MrDoc::Selection currentSelection;
  MrDoc::Selection &clipboard = static_cast<TabletApplication *>(qApp)->clipboard;

  size_t selectingOnPage;

  QScrollArea *scrollArea;

  QUndoStack undoStack;

  qreal m_zoom;

  QString m_statusText;

private:
  void showEvent(QShowEvent* event) override;

  QTime timer;

  QTimer *updateTimer;
  QTimer *updateDirtyTimer;

  qreal count;

  bool m_inputEnabled = true;

  void scrollDocumentToPageNum(size_t pageNum);

  QVector<qreal> currentPattern = MrDoc::solidLinePattern;

  QCursor penCursor;
  QCursor circleCursor;
  QCursor rulerCursor;
  QCursor eraserCursor;
  QCursor strokeEraserCursor;

  MrDoc::Stroke currentStroke;
  QRect currentUpdateRect;

  state currentState;

  bool penDown = false;
  size_t drawingOnPage;

  tool currentTool;
  tool previousTool;

  bool showGrid;
  bool snapToGrid;
  qreal gridWidth;

  qreal currentDashOffset;

  qreal minWidthMultiplier = 0.0;
  qreal maxWidthMultiplier = 1.25;

  QPointF currentCOSPos;
  QPointF firstMousePos;
  QPointF previousMousePos;
  QPointF previousPagePos;
  MrDoc::Selection::GrabZone m_grabZone = MrDoc::Selection::GrabZone::None;


  QPointF pagePosToGrid(QPointF pagePos);

  void startDrawing(QPointF mousePos, qreal pressure);
  void continueDrawing(QPointF mousePos, qreal pressure);
  void stopDrawing(QPointF mousePos, qreal pressure);

  void startRuling(QPointF mousePos);
  void continueRuling(QPointF mousePos);
  void stopRuling(QPointF mousePos);

  void startCircling(QPointF mousePos);
  void continueCircling(QPointF mousePos);
  void stopCircling(QPointF mousePos);

  void startRecting(QPointF mousePos);
  void continueRecting(QPointF mousePos);
  void stopRecting(QPointF mousePos);

  void startTexting(QPointF mousePos);

  void startSelecting(QPointF mousePos);
  void continueSelecting(QPointF mousePos);
  void stopSelecting(QPointF mousePos);

  void startMovingSelection(QPointF mousePos);
  void continueMovingSelection(QPointF mousePos);

  void startRotatingSelection(QPointF mousePos);
  void continueRotatingSelection(QPointF mousePos);
  void stopRotatingSelection(QPointF mousePos);

  void startResizingSelection(QPointF mousePos, MrDoc::Selection::GrabZone grabZone);
  void continueResizingSelection(QPointF mousePos);
  void stopResizingSelection(QPointF mousePos);

  void setPreviousTool();

  void erase(QPointF mousePos, bool invertEraser = false);

public slots:
  void undo();
  void redo();

  void selectAll();
  void copy();
  void paste();
  void pasteImage();
  void pasteText();
  void cut();
  void deleteSlot();

private slots:
  void updateAllDirtyBuffers();


  void pageFirst();
  void pageLast();
  void pageUp();
  void pageDown();

  void pageAddBefore();
  void pageAddAfter();
  void pageAddBeginning();
  void pageAddEnd();

  void pageRemove();

  void veryFine();
  void fine();
  void medium();
  void thick();
  void veryThick();

  void setPencilCursorIcon();
  void setDotCursorIcon();

  void solidPattern();
  void dashPattern();
  void dashDotPattern();
  void dotPattern();

  void updateWhileDrawing();

signals:
  void pen();
  void ruler();
  void circle();
  void rect();
  void eraser();
  void strokeEraser();
  void select();
  void rectSelect();
  void hand();

  void updateGUI();

  void modified();

  void quickmenu();

protected:
  void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;
  void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
  void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
  void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
  virtual void mouseDoubleClickEvent(QMouseEvent *event) override;

  virtual void dragEnterEvent(QDragEnterEvent* event) override;
  virtual void dragMoveEvent(QDragMoveEvent* event) override;
  virtual void dragLeaveEvent(QDragLeaveEvent* event) override;
  virtual void dropEvent(QDropEvent* event) override;

  void tabletEvent(QTabletEvent *event) Q_DECL_OVERRIDE;

signals:

public slots:
  void letGoSelection();
  void enableInput();
  void disableInput();

public:
  bool inputEnabled();
};

#endif // WIDGET_H
