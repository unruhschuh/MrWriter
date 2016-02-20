#ifndef WIDGET_H
#define WIDGET_H

#define MIN_ZOOM 0.1
#define MAX_ZOOM 10.0

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

class Widget : public QWidget
// class Widget : public QOpenGLWidget
{
  Q_OBJECT
public:
  explicit Widget(QWidget *parent = 0);

  enum class tool
  {
    NONE,
    PEN,
    RULER,
    CIRCLE,
    ERASER,
    SELECT,
    HAND
  };
  enum class state
  {
    IDLE,
    DRAWING,
    RULING,
    CIRCLING,
    SELECTING,
    SELECTED,
    MOVING_SELECTION,
    RESIZING_SELECTION,
    ROTATING_SELECTION
  };

  static constexpr qreal m_veryFinePenWidth = 0.42;
  static constexpr qreal m_finePenWidth = 0.85;
  static constexpr qreal m_mediumPenWidth = 1.41;
  static constexpr qreal m_thickPenWidth = 2.26;
  static constexpr qreal m_veryThickPenWidth = 5.67;

  void setCurrentTool(tool toolID);
  tool getCurrentTool()
  {
    return m_currentTool;
  }

  void setCurrentPenWidth(qreal penWidth);

  qreal getCurrentPenWidth()
  {
    return m_currentPenWidth;
  }

  void mouseAndTabletEvent(QPointF mousePos, Qt::MouseButton button, Qt::MouseButtons buttons, Qt::KeyboardModifiers keyboardModifiers,
                           QTabletEvent::PointerType pointerType, QEvent::Type eventType, qreal pressure, bool tabletEvent);

  /**
   * @brief updateAllPageBuffers
   * @todo use std::unique_lock
   */
  void updateAllPageBuffers();
  void updateImageBuffer(int buffNum);
  void updateBuffer(int i);
  void updateBufferRegion(int buffNum, QRectF const &clipRect);
  void drawOnBuffer(bool last = false);
  int getPageFromMousePos(QPointF mousePos);
  QPointF getPagePosFromMousePos(QPointF mousePos, int pageNum);
  QPointF getAbsolutePagePosFromMousePos(QPointF mousePos);
  QRect getWidgetGeometry();
  int getCurrentPage();

  void setCurrentState(state newState);
  state getCurrentState();

  void setCurrentColor(QColor newColor);
  QColor getCurrentColor();

  void setDocument(const MrDoc::Document &newDocument);
  void letGoSelection();

  void newFile();
  //    void openFile();

  void zoomIn();
  void zoomOut();
  void zoomTo(qreal newZoom);
  void zoomFitWidth();
  void zoomFitHeight();

  void rotateSelection(qreal angle);

  MrDoc::Document m_currentDocument;
  QVector<QPixmap> m_pageBuffer;
  QVector<QImage> m_pageImageBuffer;
  QMutex m_pageImageBufferMutex;

  QColor m_currentColor;
  qreal m_currentPenWidth;

  qreal m_currentAngle;

  void setCurrentPattern(QVector<qreal> newPattern);
  QVector<qreal> getCurrentPattern();

  MrDoc::Selection currentSelection;
  MrDoc::Selection &clipboard = static_cast<TabletApplication *>(qApp)->m_clipboard;

  int m_selectingOnPage;

  QScrollArea *m_scrollArea;

  QUndoStack m_undoStack;

  qreal m_zoom;

private:
  QTime m_timer;

  QTimer *m_updateTimer;
  QTimer *m_updateDirtyTimer;

  qreal m_count;

  void scrollDocumentToPageNum(int pageNum);

  QVector<qreal> m_currentPattern = MrDoc::solidLinePattern;

  QCursor m_penCursor;
  QCursor m_circleCursor;
  QCursor m_rulerCursor;
  QCursor m_eraserCursor;

  MrDoc::Stroke m_currentStroke;
  QRect m_currentUpdateRect;

  state m_currentState;

  bool m_penDown = false;
  int m_drawingOnPage;

  tool m_currentTool;
  tool m_previousTool;
  bool m_realEraser;

  qreal m_currentDashOffset;

  qreal m_minWidthMultiplier = 0.0;
  qreal m_maxWidthMultiplier = 1.25;

  QPointF m_currentCOSPos;
  QPointF m_firstMousePos;
  QPointF m_previousMousePos;
  QPointF m_previousPagePos;
  MrDoc::Selection::GrabZone m_grabZone = MrDoc::Selection::GrabZone::None;

  void startDrawing(QPointF mousePos, qreal pressure);
  void continueDrawing(QPointF mousePos, qreal pressure);
  void stopDrawing(QPointF mousePos, qreal pressure);

  void startRuling(QPointF mousePos);
  void continueRuling(QPointF mousePos);
  void stopRuling(QPointF mousePos);

  void startCircling(QPointF mousePos);
  void continueCircling(QPointF mousePos);
  void stopCircling(QPointF mousePos);

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

private slots:
  void updateAllDirtyBuffers();

  void undo();
  void redo();

  void selectAll();
  void copy();
  void paste();
  void cut();

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

  void solidPattern();
  void dashPattern();
  void dashDotPattern();
  void dotPattern();

  void updateWhileDrawing();

signals:
  void pen();
  void ruler();
  void circle();
  void eraser();
  void select();
  void hand();

  void updateGUI();

  void modified();

protected:
  void paintEvent(QPaintEvent *event) Q_DECL_OVERRIDE;
  void mousePressEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
  void mouseMoveEvent(QMouseEvent *event) Q_DECL_OVERRIDE;
  void mouseReleaseEvent(QMouseEvent *event) Q_DECL_OVERRIDE;

  void tabletEvent(QTabletEvent *event) Q_DECL_OVERRIDE;

signals:

public slots:
};

#endif // WIDGET_H
