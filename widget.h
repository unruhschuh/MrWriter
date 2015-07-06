#ifndef WIDGET_H
#define WIDGET_H

//#define PEN    1
//#define ERASER 2
//#define SELECT 3

#define MIN_ZOOM 0.1
#define MAX_ZOOM 10.0

#include <QWidget>
#include <QVector>
#include <QPainter>
#include <QTabletEvent>
#include <QUndoStack>
#include <QScrollArea>

#include "tabletapplication.h"
#include "curve.h"
#include "document.h"
#include "selection.h"

class Widget : public QWidget
{
    Q_OBJECT
public:
    explicit Widget(QWidget *parent = 0);

    enum class tool { NONE, PEN, ERASER, SELECT, HAND };
    enum class state { IDLE, DRAWING, SELECTING, SELECTED, MODIFYING_SELECTION };

    void setCurrentTool(tool toolID);

    void mouseAndTabletEvent(QPointF mousePos, Qt::MouseButton button, Qt::MouseButtons buttons, QTabletEvent::PointerType pointerType, QEvent::Type eventType, qreal pressure, bool tabletEvent);

    void updateAllPageBuffers();
    void updateBuffer(int i);
    void updateBufferRegion(int buffNum, QRectF clipRect);
    void drawOnBuffer(QPointF from, QPointF to, qreal pressure);
    int getPageFromMousePos(QPointF mousePos);
    QPointF getPagePosFromMousePos(QPointF mousePos, int pageNum);
    QRect getWidgetGeometry();
    int getCurrentPage();

    void setCurrentState(state newState);
    state getCurrentState();

    void setCurrentColor(QColor newColor);

    void setDocument(Document* newDocument);

    void newFile();
//    void openFile();

    void zoomIn();
    void zoomOut();
    void zoomTo(qreal newZoom);
    void zoomFitWidth();

    Document* currentDocument;
    QVector<QImage> pageBuffer;

    QColor currentColor;
    float currentPenWidth;

    Selection currentSelection;

    int selectingOnPage;

    QScrollArea* scrollArea;

    QUndoStack undoStack;

    qreal zoom;

private:

    void scrollDocumentToPageNum(int pageNum);

    QCursor penCursor;
    QCursor eraserCursor;

    Curve currentCurve;
    Selection &clipboard = static_cast<TabletApplication*>(qApp)->clipboard;

    state currentState;

    bool penDown = false;
    int drawingOnPage;

    tool currentTool;
    tool previousTool;

    qreal minWidthMultiplier = 0.0;
    qreal maxWidthMultiplier = 1.25;

    QPointF currentCOSPos;
    QPointF previousMousePos;
    QPointF previousPagePos;


    void startDrawing(QPointF mousePos, qreal pressure);
    void continueDrawing(QPointF mousePos, qreal pressure);
    void stopDrawing(QPointF mousePos, qreal pressure);

    void startSelecting(QPointF mousePos);
    void continueSelecting(QPointF mousePos);
    void stopSelecting(QPointF mousePos);
    void letGoSelection();

    void startMovingSelection(QPointF mousePos);
    void continueMovingSelection(QPointF mousePos);

    void setPreviousTool();

    void erase(QPointF mousePos);

private slots:
    void undo();
    void redo();
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

signals:
    void pen();
    void eraser();
    void select();
    void hand();

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
