#ifndef WIDGET_H
#define WIDGET_H

#define MIN_ZOOM 0.1
#define MAX_ZOOM 10.0

#include <QMutex>
#include <QOpenGLWidget>
#include <QPainter>
#include <QPlainTextEdit>
#include <QScrollArea>
#include <QTabletEvent>
#include <QTime>
#include <QTimer>
#include <QUndoStack>
#include <QVector>
#include <QWidget>

#include "document.h"
#include "mrdoc.h"
#include "stroke.h"
#include "tabletapplication.h"
#include "text.h"

class Widget : public QWidget
{
  Q_OBJECT

/*##############################################################################
# PUBLIC TYPES
##############################################################################*/
public:

  enum class Tool
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
  enum class State
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
  enum class Cursor
  {
    PENCIL,
    DOT
  };

/*##############################################################################
# PUBLIC FUNCTIONS
##############################################################################*/
public:

  explicit Widget(QWidget *parent = nullptr);

  bool inputEnabled();

  void setCurrentTool(Tool p_tool);
  void setPenCursor(const QString &p_resourceName);

  inline Tool currentTool()
  {
    return m_currentTool;
  }

  void setCurrentPenWidth(qreal p_penWidth);

  qreal currentPenWidth()
  {
    return m_currentPenWidth;
  }

  void setCurrentPenCursor(Widget::Cursor p_cursor);

  Widget::Cursor currentPenCursor()
  {
    return m_currentPenCursor;
  }

  void mouseAndTabletEvent(QPointF mousePos, Qt::MouseButton button, Qt::MouseButtons buttons, Qt::KeyboardModifiers keyboardModifiers,
                           QTabletEvent::PointerType pointerType, QEvent::Type eventType, qreal pressure, bool tabletEvent);

  bool pageVisible(size_t buffNum) const;
  size_t firstVisiblePage() const;
  size_t lastVisiblePage() const;

  void updateAllPageBuffers(bool force = false);
  void updateImageBuffer(size_t buffNum);
  void updateBuffer(size_t i);
  void updateBufferRegion(size_t buffNum, QRectF const &clipRect);
  void drawGrid(QPainter &painter, size_t buffNum);
  void drawOnBuffer(bool last = false);
  size_t getPageFromMousePos(QPointF mousePos) const;
  QPointF getPagePosFromMousePos(QPointF mousePos, size_t pageNum) const;
  QPointF getAbsolutePagePosFromMousePos(QPointF mousePos) const;
  QPointF getMousePosFromPagePos(QPointF pagePos, size_t pageNum) const;
  QRect getWidgetGeometry() const;
  size_t currentPage() const;

  void setCurrentState(State newState);
  State currentState();

  void setCurrentFont(QFont font);
  void setCurrentColor(QColor newColor);
  QColor currentColor();

  void setDocument(const MrDoc::Document &newDocument);

  void newFile();

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

  void setCurrentPattern(QVector<qreal> newPattern);
  QVector<qreal> currentPattern();

/*##############################################################################
# PUBLIC SLOTS
##############################################################################*/
public slots:

  void letGoSelection();
  void enableInput();
  void disableInput();

  void undo();
  void redo();

  void selectAll();
  void copy();
  void paste();
  void pasteImage();
  void pasteText();
  void cut();
  void deleteSlot();
  void toTheBack();

/*##############################################################################
# SINGALS
##############################################################################*/
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

/*##############################################################################
# PUBLIC MEMBERS
##############################################################################*/
public:

  static constexpr qreal m_veryFinePenWidth = 0.42;
  static constexpr qreal m_finePenWidth = 0.85;
  static constexpr qreal m_mediumPenWidth = 1.41;
  static constexpr qreal m_thickPenWidth = 2.26;
  static constexpr qreal m_veryThickPenWidth = 5.67;

  MrDoc::Document m_currentDocument;
  std::vector<QPixmap> m_pageBuffer;
  QPixmap m_currentPageOverlay;
  std::vector<QImage> m_pageImageBuffer;
  QMutex m_pageImageBufferMutex;

  QColor m_currentColor;
  qreal m_currentPenWidth;

  Widget::Cursor m_currentPenCursor;

  qreal m_currentAngle;

  QFont m_currentFont;

  MrDoc::Selection m_currentSelection;
  MrDoc::Selection &m_clipboard = static_cast<TabletApplication *>(qApp)->clipboard;

  size_t m_selectingOnPage;

  QScrollArea *m_scrollArea;

  QUndoStack m_undoStack;

  qreal m_zoom;

  QString m_statusText;

/*******************************************************************************
* PROTECTED FUNCTIONS
*******************************************************************************/
protected:

  virtual void paintEvent(QPaintEvent *event) override;
  virtual void mousePressEvent(QMouseEvent *event) override;
  virtual void mouseMoveEvent(QMouseEvent *event) override;
  virtual void mouseReleaseEvent(QMouseEvent *event) override;
  virtual void mouseDoubleClickEvent(QMouseEvent *event) override;

  virtual bool event(QEvent *event) override;

  virtual void dragEnterEvent(QDragEnterEvent* event) override;
  virtual void dragMoveEvent(QDragMoveEvent* event) override;
  virtual void dragLeaveEvent(QDragLeaveEvent* event) override;
  virtual void dropEvent(QDropEvent* event) override;

  virtual void showEvent(QShowEvent* event) override;

  void tabletEvent(QTabletEvent *event) override;

/*------------------------------------------------------------------------------
| PRIVATE FUNCTIONS
------------------------------------------------------------------------------*/
private:

  void scrollDocumentToPageNum(size_t pageNum);

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
  void stopTexting(QPointF mousePos);
  void startEditingText(QPointF mousePos, size_t index);

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

/*------------------------------------------------------------------------------
| PRIVATE SLOTS
------------------------------------------------------------------------------*/
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

/*------------------------------------------------------------------------------
| PRIVATE MEMBERS
------------------------------------------------------------------------------*/
private:

  QTime m_timer;

  QTimer *m_updateTimer;
  QTimer *m_updateDirtyTimer;

  bool m_inputEnabled = true;

  QVector<qreal> m_currentPattern = MrDoc::solidLinePattern;

  QCursor m_penCursor;
  QCursor m_circleCursor;
  QCursor m_rulerCursor;
  QCursor m_eraserCursor;
  QCursor m_strokeEraserCursor;

  MrDoc::Stroke m_currentStroke;
  QRect m_currentUpdateRect;

  MrDoc::Text m_currentText;

  State m_currentState;

  bool m_penDown = false;
  size_t m_drawingOnPage;

  Tool m_currentTool;
  Tool m_previousTool;

  bool m_showGrid;
  bool m_snapToGrid;
  qreal m_gridWidth;

  qreal m_currentDashOffset;

  qreal m_minWidthMultiplier = 0.0;
  qreal m_maxWidthMultiplier = 1.25;

  QPointF m_currentCOSPos;
  QPointF m_firstMousePos;
  QPointF m_previousMousePos;
  QPointF m_previousPagePos;
  MrDoc::Selection::GrabZone m_grabZone = MrDoc::Selection::GrabZone::None;

  QPlainTextEdit * m_textEdit;


};

#endif // WIDGET_H
