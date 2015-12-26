#include "widget.h"
#include "tictoc.h"
#include "mrdoc.h"
#include "commands.h"
#include "tabletapplication.h"

#include <QMouseEvent>
#include <QFileDialog>
#include <QPdfWriter>
#include <QPixmap>
#include <QBitmap>
#include <QUndoCommand>
#include <QScrollArea>
#include <QScrollBar>
#include <QDebug>
#include <QtConcurrent>
#include <QSettings>
#include <QTimer>
#include <qmath.h>

#define PAGE_GAP 10.0
#define ZOOM_STEP 1.2

#include <QPainter>
#include <QRectF>

Widget::Widget(QWidget *parent) : QWidget(parent)
//Widget::Widget(QWidget *parent) : QOpenGLWidget(parent)
{
    QSettings settings;
//    qDebug() << settings.applicationVersion();

    currentState = state::IDLE;

    // setup cursors
    QPixmap penCursorBitmap = QPixmap(":/images/penCursor3.png");
    QPixmap penCursorMask  = QPixmap(":/images/penCursor3Mask.png");
    penCursorBitmap.setMask(QBitmap(penCursorMask));
    penCursor = QCursor(penCursorBitmap, -1, -1);

    QPixmap circleCursorBitmap = QPixmap(":/images/circleCursor.png");
    QPixmap circleCursorMask  = QPixmap(":/images/circleCursorMask.png");
    circleCursorBitmap.setMask(QBitmap(circleCursorMask));
    circleCursor = QCursor(circleCursorBitmap, -1, -1);

    QPixmap rulerCursorBitmap = QPixmap(":/images/rulerCursor.png");
    QPixmap rulerCursorMask  = QPixmap(":/images/rulerCursorMask.png");
    rulerCursorBitmap.setMask(QBitmap(rulerCursorMask));
    rulerCursor = QCursor(rulerCursorBitmap, -1, -1);

    QPixmap eraserCursorBitmap = QPixmap(":/images/eraserCursor.png");
    QPixmap eraserCursorMask  = QPixmap(":/images/eraserCursorMask.png");
    eraserCursorBitmap.setMask(QBitmap(eraserCursorMask));
    eraserCursor = QCursor(eraserCursorBitmap, -1, -1);

    currentTool = tool::PEN;
    previousTool = tool::NONE;
    realEraser = false;
    setCursor(penCursorBitmap);

    currentDocument = new MrDoc::Document();

    currentPenWidth = 1.41;
    currentColor = QColor(0,0,0);
    zoom = 1.0;

    currentCOSPos.setX(0.0);
    currentCOSPos.setY(0.0);
    updateAllPageBuffers();
    setGeometry(getWidgetGeometry());

    parent->updateGeometry();
    parent->update();

    updateTimer = new QTimer(this);
    connect(updateTimer, SIGNAL(timeout()), this, SLOT(updateWhileDrawing()));
}

void Widget::updateAllPageBuffers()
{
    pageBuffer.clear();
    QVector<QFuture<void> > future;
    for (int buffNum = 0; buffNum < currentDocument->pages.size(); ++buffNum)
    {
        pageBuffer.append(QPixmap());
    }

//    for (int buffNum = 0; buffNum < currentDocument->pages.size(); ++buffNum)
//    {
//        updateBuffer(buffNum);
//    }

    // shouldn't work, since you're only allowed to draw on pixmaps in the gui thread. but it somehow works.
    for (int buffNum = 0; buffNum < currentDocument->pages.size(); ++buffNum)
    {
        future.append(QtConcurrent::run(this, &Widget::updateBuffer, buffNum));
    }
    for (int buffNum = 0; buffNum < currentDocument->pages.size(); ++buffNum)
    {
        future[buffNum].waitForFinished();
    }
}

void Widget::updateBuffer(int buffNum)
{
    MrDoc::Page page = currentDocument->pages.at(buffNum);
    int pixelWidth = zoom * page.getWidth();
    int pixelHeight = zoom * page.getHeight();
//    QImage image(pixelWidth, pixelHeight, QImage::Format_ARGB32_Premultiplied);
    QPixmap image(pixelWidth, pixelHeight);

    image.fill(page.backgroundColor);

    QPainter painter;
    painter.begin(&image);
    painter.setRenderHint(QPainter::Antialiasing, true);

//    currentDocument->paintPage(buffNum, painter, zoom);
    currentDocument->pages[buffNum].paint(painter, zoom);

    painter.end();

    pageBuffer.replace(buffNum, image);

//    if (pageBuffer.length() <= buffNum)
//    {
//        pageBuffer.append(image);
//    } else {
//        pageBuffer.replace(buffNum, image);
//    }
}

void Widget::updateBufferRegion(int buffNum, QRectF clipRect)
{
    QPainter painter;
    painter.begin(&pageBuffer[buffNum]);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setClipRect(clipRect);
    painter.setClipping(true);

    painter.fillRect(clipRect, currentDocument->pages.at(buffNum).backgroundColor);

    QRectF paintRect = QRectF(clipRect.topLeft() / zoom, clipRect.bottomRight() / zoom);
    currentDocument->pages[buffNum].paint(painter, zoom, paintRect);

    painter.end();
}

void Widget::drawOnBuffer(QPointF from, QPointF to, qreal pressureFrom, qreal pressureTo)
{
    QPen pen;

    QPainter painter;
    painter.begin(&pageBuffer[drawingOnPage]);
    painter.setRenderHint(QPainter::Antialiasing, true);

    qreal tmpPenWidth = zoom * currentPenWidth * (pressureFrom + pressureTo) / 2.0;
    pen.setWidthF(tmpPenWidth);
    pen.setColor(currentColor);
    pen.setCapStyle(Qt::RoundCap);
    if (currentStroke.pattern != MrDoc::Stroke::solidLinePattern)
    {
        pen.setDashPattern(currentStroke.pattern);
        pen.setDashOffset(currentDashOffset);
        if (tmpPenWidth != 0)
            currentDashOffset += 1.0/tmpPenWidth * (QLineF(zoom * from, zoom * to)).length();
    }
    painter.setPen(pen);
    painter.drawLine(zoom * from, zoom * to);

    if (from == to)
    {
        QRectF pointRect(zoom * from, QSizeF(0,0));
        qreal pad = tmpPenWidth / 2;
        painter.setPen(Qt::NoPen);
        painter.setBrush(QBrush(currentColor));
        painter.drawEllipse(pointRect.adjusted(-pad,-pad,pad,pad));
    }

    painter.end();
}


QRect Widget::getWidgetGeometry()
{
    int width = 0;
    int height = 0;
    for (int i = 0; i < pageBuffer.size(); ++i)
    {
        height += pageBuffer[i].height() + PAGE_GAP;
        if (pageBuffer[i].width() > width)
            width = pageBuffer[i].width();
    }
    height -= PAGE_GAP;
    return QRect(0, 0, width, height);
}

void Widget::paintEvent(QPaintEvent *event)
{
//    QRect widgetGeometry = getWidgetGeometry();
    QPalette p(palette());
    setAutoFillBackground(true);
    setPalette(p);

//    qInfo() << "Paint Event";

    QPainter painter(this);

    if (currentState == state::DRAWING)
    {
        QRectF rectSource;
        QTransform trans;
        for (int i = 0; i < drawingOnPage; ++i)
        {
            trans = trans.translate(0, -(pageBuffer.at(i).height() + PAGE_GAP));
        }
        rectSource = trans.mapRect(event->rect());

//        QPixmap tmp = QPixmap::fromImage(pageBuffer.at(drawingOnPage));
        painter.drawPixmap(event->rect(), pageBuffer[drawingOnPage], rectSource);

//        painter.drawImage(event->rect(), pageBuffer.at(drawingOnPage), rectSource);
        return;
    }

//    painter.setRenderHint(QPainter::Antialiasing, true);

    for (int i = 0; i < pageBuffer.size(); ++i)
    {
        QRectF rectSource;
        rectSource.setTopLeft(QPointF(0.0,0.0));
        rectSource.setWidth(pageBuffer.at(i).width());
        rectSource.setHeight(pageBuffer.at(i).height());

        QRectF rectTarget;
//        rectTarget.setTopLeft(QPointF(0.0, currentYPos));
        rectTarget.setTopLeft(QPointF(0.0, 0.0));
        rectTarget.setWidth(pageBuffer.at(i).width());
        rectTarget.setHeight(pageBuffer.at(i).height());

        painter.drawPixmap(rectTarget, pageBuffer.at(i), rectSource);

        if ((currentState == state::SELECTING || currentState == state::SELECTED || currentState == state::MOVING_SELECTION) && i == currentSelection.pageNum)
        {
            currentSelection.paint(painter, zoom);
        }

        painter.translate(QPointF(0.0, rectSource.height() + PAGE_GAP));
    }
}

void Widget::updateWhileDrawing()
{
    update(currentUpdateRect);
}

void Widget::mouseAndTabletEvent(QPointF mousePos, Qt::MouseButton button, Qt::MouseButtons buttons, Qt::KeyboardModifiers keyboardModifiers, QTabletEvent::PointerType pointerType, QEvent::Type eventType, qreal pressure, bool tabletEvent)
{
//    qInfo() << qApp->queryKeyboardModifiers();

    qInfo() << pointerType;

    // Under Linux the keyboard modifiers are not reported to tabletevent. this should work
    // everywhere.
    keyboardModifiers = qApp->queryKeyboardModifiers();

    // bencmark
    if (eventType == QEvent::MouseButtonPress)
    {
        timer.start();
        count = 1;
    }
    if (eventType == QEvent::MouseMove)
    {
        ++count;
//        qDebug() << static_cast<qreal>(count) / static_cast<qreal>(timer.elapsed()) * 1000.0;
    }
    if (eventType == QEvent::MouseButtonRelease)
    {
        ++count;
        qInfo() << static_cast<qreal>(count) / static_cast<qreal>(timer.elapsed()) * 1000.0;
    }
    // end benchmark

    int pageNum = getPageFromMousePos(mousePos);
    QPointF pagePos = getPagePosFromMousePos(mousePos, pageNum);

    bool invertEraser;
    if (keyboardModifiers & Qt::ShiftModifier)
    {
        invertEraser = true;
    } else {
        invertEraser = false;
    }

    if (tabletEvent)
    {
        pressure = minWidthMultiplier + pressure * (maxWidthMultiplier - minWidthMultiplier);
    }

    if (eventType == QEvent::MouseButtonRelease)
    {
        undoStack.endMacro();
        setPreviousTool();
    }

    if ((currentState == state::IDLE || currentState == state::SELECTED) && buttons & Qt::MiddleButton && pointerType == QTabletEvent::Pen)
    {
        if (eventType == QEvent::MouseButtonPress && button == Qt::MiddleButton)
        {
            if (currentTool != tool::HAND)
            {
                previousTool = currentTool;
            }
            previousMousePos = mousePos;
            emit hand();
            return;
        }
        if (eventType == QEvent::MouseMove)
        {
            int dx = mousePos.x() - previousMousePos.x();
            int dy = mousePos.y() - previousMousePos.y();

            scrollArea->horizontalScrollBar()->setValue(scrollArea->horizontalScrollBar()->value() - dx);
            scrollArea->verticalScrollBar()->setValue(scrollArea->verticalScrollBar()->value() - dy);

            mousePos -= QPointF(dx,dy);

            previousMousePos = mousePos;
            return;
        }
        return;
    }


    if (currentState == state::SELECTED)
    {
        if (eventType == QEvent::MouseButtonPress)
        {
            if (currentSelection.selectionPolygon.containsPoint(pagePos, Qt::OddEvenFill) && currentSelection.pageNum == pageNum)
            {
                // move selection
                startMovingSelection(mousePos);
                return;
            } else {
                letGoSelection();
                update();
            }
        }
        if (eventType == QEvent::MouseMove)
        {

        }
        if (eventType == QEvent::MouseButtonRelease)
        {
            setPreviousTool();
        }
    }

    if (currentState == state::IDLE && button == Qt::RightButton)
    {
        previousTool = currentTool;
        startSelecting(mousePos);
        emit select();
        return;
    }


    if (currentState == state::MOVING_SELECTION)
    {
        if (eventType == QEvent::MouseMove)
        {
            continueMovingSelection(mousePos);
            return;
        }
        if (eventType == QEvent::MouseButtonRelease)
        {
            currentState = state::SELECTED;
            setPreviousTool();
        }
    }

    if (currentState == state::SELECTING)
    {
        if (eventType == QEvent::MouseButtonPress)
        {

        }
        if (eventType == QEvent::MouseMove)
        {
            continueSelecting(mousePos);
            return;
        }
        if (eventType == QEvent::MouseButtonRelease)
        {
            stopSelecting(mousePos);
            emit updateGUI();
            setPreviousTool();
            return;
        }
    }

    if (currentState == state::DRAWING)
    {
        if (eventType == QEvent::MouseButtonPress)
        {

        }
        if (eventType == QEvent::MouseMove)
        {
//            QtConcurrent::run(this, &Widget::continueDrawing, mousePos, pressure);
            continueDrawing(mousePos, pressure);
            return;
        }
        if (eventType == QEvent::MouseButtonRelease)
        {
            stopDrawing(mousePos, pressure);
            setPreviousTool();
            return;
        }
    }

    if (currentState == state::RULING)
    {
        if (eventType == QEvent::MouseMove)
        {
            continueRuling(mousePos);
            return;
        }
        if (eventType == QEvent::MouseButtonRelease)
        {
            stopRuling(mousePos);
            setPreviousTool();
            return;
        }
    }

    if (currentState == state::CIRCLING)
    {
        if (eventType == QEvent::MouseMove)
        {
            continueCircling(mousePos);
            return;
        }
        if (eventType == QEvent::MouseButtonRelease)
        {
            stopCircling(mousePos);
            setPreviousTool();
            return;
        }
    }

    if (currentState == state::IDLE)
    {
        if (eventType == QEvent::MouseButtonPress)
        {
            if (pointerType == QTabletEvent::Pen)
            {
                if (currentTool == tool::PEN)
                {
                    startDrawing(mousePos, pressure);
                    return;
                }
                if (currentTool == tool::RULER)
                {
                    startRuling(mousePos);
                    return;
                }
                if (currentTool == tool::CIRCLE)
                {
                    startCircling(mousePos);
                    return;
                }
                if (currentTool == tool::ERASER)
                {
                    undoStack.beginMacro("erase");
                    erase(mousePos, invertEraser);
                    return;
                }
                if (currentTool == tool::SELECT)
                {
                    startSelecting(mousePos);
                    return;
                }
                if (currentTool == tool::HAND)
                {
                    previousMousePos = mousePos;
                }
            }
            if (pointerType == QTabletEvent::Eraser)
            {
                previousTool = currentTool;
                emit eraser();
                undoStack.beginMacro("erase");
                erase(mousePos, invertEraser);
            }
        }
        if (eventType == QEvent::MouseMove)
        {
            if (pointerType == QTabletEvent::Eraser || currentTool == tool::ERASER)
            {
                erase(mousePos, invertEraser);
            }
            if (currentTool == tool::HAND)
            {
                int dx = mousePos.x() - previousMousePos.x();
                int dy = mousePos.y() - previousMousePos.y();

                scrollArea->horizontalScrollBar()->setValue(scrollArea->horizontalScrollBar()->value() - dx);
                scrollArea->verticalScrollBar()->setValue(scrollArea->verticalScrollBar()->value() - dy);

                mousePos -= QPointF(dx,dy);

                previousMousePos = mousePos;
            }
        }
        if (eventType == QEvent::MouseButtonRelease)
        {
            setPreviousTool();
        }
    }
}

void Widget::tabletEvent(QTabletEvent *event)
{
    event->accept();

//    qDebug("tabletevent");

//    event->setAccepted(true);

    QPointF mousePos = QPointF(event->hiResGlobalX(), event->hiResGlobalY()) - mapToGlobal(QPoint(0,0));
    qreal pressure = event->pressure();

    QEvent::Type eventType;
    if (event->type() == QTabletEvent::TabletPress)
    {
        qInfo() << "tabletPressEvent";
        eventType = QEvent::MouseButtonPress;
        penDown = true;
    }
    if (event->type() == QTabletEvent::TabletMove)
    {
        eventType = QEvent::MouseMove;
        penDown = true;
    }
    if (event->type() == QTabletEvent::TabletRelease)
    {
        eventType = QEvent::MouseButtonRelease;
    }

    Qt::KeyboardModifiers keyboardModifiers = event->modifiers();

//    qInfo() << keyboardModifiers;
//    qInfo() << "tabletEvent";

    mouseAndTabletEvent(mousePos, event->button(), event->buttons(), keyboardModifiers, event->pointerType(), eventType, pressure, true);

}

void Widget::mousePressEvent(QMouseEvent *event)
{
//    return; // ignore mouse event
    qInfo() << "mousePressEvent";
    bool usingTablet = static_cast<TabletApplication*>(qApp)->isUsingTablet();

    if (!usingTablet)
    {
        if (!penDown)
        {
            QPointF mousePos = event->localPos();
            qreal pressure = 1;

            Qt::KeyboardModifiers keyboardModifiers = event->modifiers();
            mouseAndTabletEvent(mousePos, event->button(), event->buttons(), keyboardModifiers, QTabletEvent::Pen, event->type(), pressure, false);
        }
    }
}

void Widget::mouseMoveEvent(QMouseEvent *event)
{
//    return; // ignore mouse event
//    qInfo() << "mouseMoveEvent";
    bool usingTablet = static_cast<TabletApplication*>(qApp)->isUsingTablet();

    if (!usingTablet)
    {
        if (!penDown)
        {
            QPointF mousePos = event->localPos();
            qreal pressure = 1;

            Qt::KeyboardModifiers keyboardModifiers = event->modifiers();
            mouseAndTabletEvent(mousePos, event->button(), event->buttons(), keyboardModifiers, QTabletEvent::Pen, event->type(), pressure, false);
        }
    }
}

void Widget::mouseReleaseEvent(QMouseEvent *event)
{
//    return; // ignore mouse event
    qInfo() << "mouseReleaseEvent";
    bool usingTablet = static_cast<TabletApplication*>(qApp)->isUsingTablet();

    if (!usingTablet)
    {
        if (!penDown)
        {
            QPointF mousePos = event->localPos();
            qreal pressure = 1;

            Qt::KeyboardModifiers keyboardModifiers = event->modifiers();
            mouseAndTabletEvent(mousePos, event->button(), event->buttons(), keyboardModifiers, QTabletEvent::Pen, event->type(), pressure, false);
        }
    }
    penDown = false;
}

void Widget::setPreviousTool()
{
    if (previousTool == tool::PEN)
    {
        emit pen();
    }
    if (previousTool == tool::RULER)
    {
        emit ruler();
    }
    if (previousTool == tool::CIRCLE)
    {
        emit circle();
    }
    if (previousTool == tool::ERASER)
    {
        emit eraser();
    }
    if (previousTool == tool::SELECT)
    {
        emit select();
    }
    if (previousTool == tool::HAND)
    {
        emit hand();
    }

    previousTool = tool::NONE;
}

void Widget::startSelecting(QPointF mousePos)
{
    int pageNum = getPageFromMousePos(mousePos);
    QPointF pagePos = getPagePosFromMousePos(mousePos, pageNum);

    MrDoc::Selection newSelection;

    newSelection.pageNum = pageNum;
    newSelection.setWidth(currentDocument->pages[pageNum].getWidth());
    newSelection.setHeight(currentDocument->pages[pageNum].getHeight());
    newSelection.selectionPolygon.append(pagePos);

    currentSelection = newSelection;

//    selecting = true;
    currentState = state::SELECTING;
    selectingOnPage = pageNum;
}

void Widget::continueSelecting(QPointF mousePos)
{
    int pageNum = selectingOnPage;
    QPointF pagePos = getPagePosFromMousePos(mousePos, pageNum);

    currentSelection.selectionPolygon.append(pagePos);

    update();
}

void Widget::stopSelecting(QPointF mousePos)
{
    int pageNum = selectingOnPage;
    QPointF pagePos = getPagePosFromMousePos(mousePos, pageNum);

    currentSelection.selectionPolygon.append(pagePos);

    QVector<int> strokesInSelection;

    for (int i = 0; i < currentDocument->pages[pageNum].strokes.size(); ++i)
    {
        MrDoc::Stroke stroke = currentDocument->pages[pageNum].strokes.at(i);
        bool containsStroke = true;
        for (int j = 0; j < stroke.points.size(); ++j)
        {
            if (!currentSelection.selectionPolygon.containsPoint(stroke.points.at(j), Qt::OddEvenFill)) {
                containsStroke = false;
            }
        }
        if (containsStroke)
        {
            strokesInSelection.append(i);
        }
    }

    if (strokesInSelection.size() == 0)
    {
        // nothing selected
        currentState = state::IDLE;
    } else {
        // something selected
        currentState = state::SELECTED;

        for (int i = strokesInSelection.size()-1; i >= 0; --i)
        {
            currentSelection.strokes.prepend(currentDocument->pages[pageNum].strokes.at(strokesInSelection.at(i)));
        }
        currentSelection.finalize();
        currentSelection.updateBuffer(zoom);

        CreateSelectionCommand* selectCommand = new CreateSelectionCommand(this, pageNum, strokesInSelection);
        undoStack.push(selectCommand);
    }
    emit updateGUI();
    update();
}

void Widget::letGoSelection()
{
    if (getCurrentState() == state::SELECTED)
    {
        int pageNum = currentSelection.pageNum;
        ReleaseSelectionCommand* releaseCommand = new ReleaseSelectionCommand(this, pageNum);
        undoStack.push(releaseCommand);
        updateBuffer(pageNum);
        update();
    }
}

void Widget::startRuling(QPointF mousePos)
{
    currentDocument->setDocumentChanged(true);
    emit modified();

    int pageNum = getPageFromMousePos(mousePos);
    QPointF pagePos = getPagePosFromMousePos(mousePos, pageNum);

    MrDoc::Stroke newStroke;
    newStroke.pattern = currentPattern;
    newStroke.points.append(pagePos);
    newStroke.pressures.append(1);
    newStroke.penWidth = currentPenWidth;
    newStroke.color = currentColor;
    currentStroke = newStroke;
    currentState = state::RULING;

    previousMousePos = mousePos;
    firstMousePos = mousePos;
    drawingOnPage = pageNum;
}

void Widget::continueRuling(QPointF mousePos)
{
    QPointF pagePos = getPagePosFromMousePos(mousePos, drawingOnPage);
    QPointF previousPagePos = getPagePosFromMousePos(previousMousePos, drawingOnPage);

    QPointF firstPagePos = currentStroke.points.at(0);

    QPointF oldPagePos = pagePos;

    currentDashOffset = 0.0;

    if (currentStroke.points.length() > 1)
    {
        oldPagePos = currentStroke.points.at(1);
        currentStroke.points.removeAt(1);
        currentStroke.pressures.removeAt(1);
    }

    currentStroke.points.append(pagePos);
    currentStroke.pressures.append(1);

    QRect clipRect(zoom*firstPagePos.toPoint(), zoom*pagePos.toPoint());
    QRect oldClipRect(zoom*firstPagePos.toPoint(), zoom*previousPagePos.toPoint());
    clipRect = clipRect.normalized().united(oldClipRect.normalized());
    int clipRad = zoom*currentPenWidth / 2 + 2;
    clipRect = clipRect.normalized().adjusted(-clipRad, -clipRad, clipRad, clipRad);
    updateBufferRegion(drawingOnPage, clipRect);
    drawOnBuffer(firstPagePos, pagePos, 1, 1);

    QRect updateRect(firstMousePos.toPoint(), mousePos.toPoint());
    QRect oldUpdateRect(firstMousePos.toPoint(), previousMousePos.toPoint());
    updateRect = updateRect.normalized().united(oldUpdateRect.normalized());
    int rad = currentPenWidth * zoom / 2 + 2;
    updateRect = updateRect.normalized().adjusted(-rad, -rad, +rad, +rad);

    update(updateRect);

    previousMousePos = mousePos;
}

void Widget::stopRuling(QPointF mousePos)
{
    QPointF pagePos = getPagePosFromMousePos(mousePos, drawingOnPage);

    if (currentStroke.points.length() > 1)
    {
        currentStroke.points.removeAt(1);
        currentStroke.pressures.removeAt(1);
    }

    currentStroke.points.append(pagePos);
    currentStroke.pressures.append(1);

    AddStrokeCommand* addCommand = new AddStrokeCommand(this, drawingOnPage, currentStroke);
    undoStack.push(addCommand);

    currentState = state::IDLE;

    update();
}

void Widget::startCircling(QPointF mousePos)
{
    currentDocument->setDocumentChanged(true);
    emit modified();

    int pageNum = getPageFromMousePos(mousePos);
    QPointF pagePos = getPagePosFromMousePos(mousePos, pageNum);

    MrDoc::Stroke newStroke;
//    newStroke.points.append(pagePos);
//    newStroke.pressures.append(1);
    newStroke.pattern = currentPattern;
    newStroke.penWidth = currentPenWidth;
    newStroke.color = currentColor;
    currentStroke = newStroke;
    currentState = state::CIRCLING;

    previousMousePos = mousePos;
    firstMousePos = mousePos;
    drawingOnPage = pageNum;
}

void Widget::continueCircling(QPointF mousePos)
{
    QPointF pagePos = getPagePosFromMousePos(mousePos, drawingOnPage);
    QPointF previousPagePos = getPagePosFromMousePos(previousMousePos, drawingOnPage);
    QPointF firstPagePos = getPagePosFromMousePos(firstMousePos, drawingOnPage);

    currentDashOffset = 0.0;

    QPointF oldPagePos = pagePos;

//    if (currentStroke.points.length() > 1)
//    {
//        oldPagePos = currentStroke.points.at(1);
//        currentStroke.points.removeAt(1);
//        currentStroke.pressures.removeAt(1);
//    }

    MrDoc::Stroke oldStroke = currentStroke;

    currentStroke.points.clear();
    currentStroke.pressures.clear();

    qreal radius = QLineF(firstPagePos, pagePos).length();
    qreal phi0 = QLineF(firstPagePos, pagePos).angle() * M_PI / 180.0;

    int N = 100;
    for (int i = 0; i < N; ++i)
    {
        qreal phi = phi0 + i * (2.0 * M_PI / (N-1));
        qreal x = firstPagePos.x() + radius * cos(phi);
        qreal y = firstPagePos.y() - radius * sin(phi);
        currentStroke.points.append(QPointF(x,y));
        currentStroke.pressures.append(1.0);
    }

    QTransform scaleTrans;
    scaleTrans = scaleTrans.scale(zoom,zoom);

    QRect clipRect = scaleTrans.mapRect(currentStroke.points.boundingRect()).toRect();
    QRect oldClipRect = scaleTrans.mapRect(oldStroke.points.boundingRect()).toRect();
    clipRect = clipRect.normalized().united(oldClipRect.normalized());
    int clipRad = zoom*currentPenWidth / 2 + 2;
    clipRect = clipRect.normalized().adjusted(-clipRad, -clipRad, clipRad, clipRad);
    updateBufferRegion(drawingOnPage, clipRect);

    for (int i = 0; i < currentStroke.points.length()-1; ++i)
    {
        drawOnBuffer(currentStroke.points.at(i), currentStroke.points.at(i+1), 1, 1);
    }

/*
//    QRect updateRect(firstMousePos.toPoint(), mousePos.toPoint());
//    QRect oldUpdateRect(firstMousePos.toPoint(), previousMousePos.toPoint());
    QRect updateRect = currentStroke.points.boundingRect().toRect();
//    updateRect = updateRect.normalized().united(oldUpdateRect.normalized());
    int rad = currentPenWidth * zoom / 2 + 2;
    updateRect = updateRect.normalized().adjusted(-rad, -rad, +rad, +rad);
*/
//    update(updateRect);
    update();

    previousMousePos = mousePos;
}

void Widget::stopCircling(QPointF mousePos)
{
    QPointF pagePos = getPagePosFromMousePos(mousePos, drawingOnPage);

    AddStrokeCommand* addCommand = new AddStrokeCommand(this, drawingOnPage, currentStroke);
    undoStack.push(addCommand);

    currentState = state::IDLE;

    update();
}

void Widget::startDrawing(QPointF mousePos, qreal pressure)
{
    updateTimer->start(33); // 33 -> 30 fps

    currentDocument->setDocumentChanged(true);
    emit modified();

    currentUpdateRect = QRect();

    int pageNum = getPageFromMousePos(mousePos);
    QPointF pagePos = getPagePosFromMousePos(mousePos, pageNum);

    currentDashOffset = 0.0;

    MrDoc::Stroke newStroke;
    newStroke.pattern = currentPattern;
    newStroke.points.append(pagePos);
    newStroke.pressures.append(pressure);
    newStroke.penWidth = currentPenWidth;
    newStroke.color = currentColor;
    currentStroke = newStroke;
//    drawing = true;
    currentState = state::DRAWING;

    previousMousePos = mousePos;
    drawingOnPage = pageNum;
}

void Widget::continueDrawing(QPointF mousePos, qreal pressure)
{
    QPointF pagePos = getPagePosFromMousePos(mousePos, drawingOnPage);
    QPointF previousPagePos = getPagePosFromMousePos(previousMousePos, drawingOnPage);

    qreal previousPressure = currentStroke.pressures.last();
    currentStroke.points.append(pagePos);
    currentStroke.pressures.append(pressure);
    drawOnBuffer(previousPagePos, pagePos, previousPressure, pressure);

    QRect updateRect(previousMousePos.toPoint(), mousePos.toPoint());
    int rad = currentPenWidth * zoom / 2 + 2;
    updateRect = updateRect.normalized().adjusted(-rad, -rad, +rad, +rad);

    currentUpdateRect = currentUpdateRect.united(updateRect);

//    update(updateRect);
//    repaint(updateRect);
//    update();

//    if (currentStroke.points.size() % 5 == 0)
//        repaint();

    previousMousePos = mousePos;
}

void Widget::stopDrawing(QPointF mousePos, qreal pressure)
{
    updateTimer->stop();

    QPointF pagePos = getPagePosFromMousePos(mousePos, drawingOnPage);
    QPointF previousPagePos = getPagePosFromMousePos(previousMousePos, drawingOnPage);

    qreal previousPressure = currentStroke.pressures.last();
    currentStroke.points.append(pagePos);
    currentStroke.pressures.append(pressure);
    drawOnBuffer(previousPagePos, pagePos, previousPressure, pressure);

//    currentDocument->pages[drawingOnPage].strokes.append(currentStroke);
    AddStrokeCommand* addCommand = new AddStrokeCommand(this, drawingOnPage, currentStroke, -1, false, true);
    undoStack.push(addCommand);

//    drawing = false;
    currentState = state::IDLE;

//    updateBuffer(drawingOnPage);
    update();
}

void Widget::erase(QPointF mousePos, bool invertEraser)
{
    int pageNum = getPageFromMousePos(mousePos);
    QPointF pagePos = getPagePosFromMousePos(mousePos, pageNum);

    QList<MrDoc::Stroke> *strokes = &(currentDocument->pages[pageNum].strokes);

    qreal eraserWidth = 10;

//    QLineF lineA = QLineF(pagePos + QPointF(-eraserWidth,-eraserWidth) / 2, pagePos + QPointF( eraserWidth,  eraserWidth) / 2);
//    QLineF lineB = QLineF(pagePos + QPointF( eraserWidth,-eraserWidth) / 2, pagePos + QPointF(-eraserWidth,  eraserWidth) / 2); // lineA and lineB form a cross X

    QLineF lineA = QLineF(pagePos + QPointF(-eraserWidth,-eraserWidth) / 2, pagePos + QPointF(-eraserWidth,  eraserWidth) / 2);
    QLineF lineB = QLineF(pagePos + QPointF(-eraserWidth, eraserWidth) / 2, pagePos + QPointF( eraserWidth,  eraserWidth) / 2);
    QLineF lineC = QLineF(pagePos + QPointF( eraserWidth, eraserWidth) / 2, pagePos + QPointF( eraserWidth, -eraserWidth) / 2);
    QLineF lineD = QLineF(pagePos + QPointF( eraserWidth,-eraserWidth) / 2, pagePos + QPointF(-eraserWidth, -eraserWidth) / 2); // lineA B C D form a square

    QRectF rectE = QRectF(pagePos + QPointF(-eraserWidth, eraserWidth) / 2, pagePos + QPointF( eraserWidth, -eraserWidth) / 2);

    QVector<int> strokesToDelete;
    QPointF iPoint;

    QPointF iPointA;
    QPointF iPointB;
    QPointF iPointC;
    QPointF iPointD;

    bool intersected;

    if (realEraser || (!realEraser && invertEraser))
    {
        for (int i = strokes->size()-1; i >= 0; --i)
        {
            MrDoc::Stroke stroke = strokes->at(i);
            if (rectE.intersects(stroke.points.boundingRect()) || !stroke.points.boundingRect().isValid() ) // this is done for speed
            {
                for (int j = 0; j < stroke.points.length()-1; ++j)
                {
                    QLineF line = QLineF(stroke.points.at(j), stroke.points.at(j+1));
                    if (line.intersect(lineA, &iPointA) == QLineF::BoundedIntersection && iPointA != stroke.points.first() && iPointA != stroke.points.last())
                    {
                        iPoint = iPointA;
                        intersected = true;
                    }
                    else if (line.intersect(lineB, &iPointB) == QLineF::BoundedIntersection && iPointB != stroke.points.first() && iPointB != stroke.points.last())
                    {
                        iPoint = iPointB;
                        intersected = true;
                    }
                    else if (line.intersect(lineC, &iPointC) == QLineF::BoundedIntersection && iPointC != stroke.points.first() && iPointC != stroke.points.last())
                    {
                        iPoint = iPointC;
                        intersected = true;
                    }
                    else if (line.intersect(lineD, &iPointD) == QLineF::BoundedIntersection && iPointD != stroke.points.first() && iPointD != stroke.points.last())
                    {
                        iPoint = iPointD;
                        intersected = true;
                    }
                    else
                    {
                        intersected = false;
                    }

                    if (intersected)
                    {
//                        if (iPoint != stroke.points.first() && iPoint != stroke.points.last())
                        {
                            MrDoc::Stroke splitStroke = stroke;
                            splitStroke.points = splitStroke.points.mid(0,j+1);
                            splitStroke.points.append(iPoint);
                            splitStroke.pressures = splitStroke.pressures.mid(0,j+1);
                            qreal lastPressure = splitStroke.pressures.last();
                            splitStroke.pressures.append(lastPressure);
                            stroke.points = stroke.points.mid(j+1);
                            stroke.points.prepend(iPoint);
                            stroke.pressures = stroke.pressures.mid(j+1);
                            qreal firstPressure = stroke.pressures.first();
                            stroke.pressures.prepend(firstPressure);
                            RemoveStrokeCommand *removeStrokeCommand = new RemoveStrokeCommand(this, pageNum, i, false);
                            undoStack.push(removeStrokeCommand);
                            AddStrokeCommand *addStrokeCommand = new AddStrokeCommand(this, pageNum, stroke, i, false, false);
                            undoStack.push(addStrokeCommand);
                            addStrokeCommand = new AddStrokeCommand(this, pageNum, splitStroke, i, false, false);
                            undoStack.push(addStrokeCommand);
//                            strokes->insert(i, splitStroke);
                            i += 2;
                            break;
                        }
                    }
                }
            }
        }
    }
//    return;

    eraserWidth = eraserWidth * 0.99;
    lineA = QLineF(pagePos + QPointF(-eraserWidth,-eraserWidth) / 2, pagePos + QPointF(-eraserWidth,  eraserWidth) / 2);
    lineB = QLineF(pagePos + QPointF(-eraserWidth, eraserWidth) / 2, pagePos + QPointF( eraserWidth,  eraserWidth) / 2);
    lineC = QLineF(pagePos + QPointF( eraserWidth, eraserWidth) / 2, pagePos + QPointF( eraserWidth, -eraserWidth) / 2);
    lineD = QLineF(pagePos + QPointF( eraserWidth,-eraserWidth) / 2, pagePos + QPointF(-eraserWidth, -eraserWidth) / 2); // lineA B C D form a square

    rectE = QRectF(pagePos + QPointF(-eraserWidth, eraserWidth) / 2, pagePos + QPointF( eraserWidth, -eraserWidth) / 2);

    for (int i = 0; i < strokes->size(); ++i)
    {
        const MrDoc::Stroke stroke = strokes->at(i);
        if (rectE.intersects(stroke.points.boundingRect()) || !stroke.points.boundingRect().isValid() ) // this is done for speed
        {
            bool foundStrokeToDelete = false;
            for (int j = 0; j < stroke.points.length(); ++j)
            {
                if (rectE.contains(stroke.points.at(j)))
                {
                    strokesToDelete.append(i);
                    foundStrokeToDelete = true;
                    break;
                }
            }
            if (foundStrokeToDelete == false)
            {
                for (int j = 0; j < stroke.points.length()-1; ++j)
                {
                    QLineF line = QLineF(stroke.points.at(j), stroke.points.at(j+1));
                    if (line.intersect(lineA, &iPoint) == QLineF::BoundedIntersection ||
                        line.intersect(lineB, &iPoint) == QLineF::BoundedIntersection ||
                        line.intersect(lineC, &iPoint) == QLineF::BoundedIntersection ||
                        line.intersect(lineD, &iPoint) == QLineF::BoundedIntersection)
                    {
                        strokesToDelete.append(i);
                        break;
                    }
                }
            }
        }
    }

    if (strokesToDelete.size() > 0)
    {
        currentDocument->setDocumentChanged(true);
        emit modified();

        QRect updateRect;
        std::sort(strokesToDelete.begin(), strokesToDelete.end(), std::greater<int>());
        for (int i = 0; i < strokesToDelete.size(); ++i)
        {
            updateRect = updateRect.united(currentDocument->pages[pageNum].strokes.at(strokesToDelete.at(i)).points.boundingRect().toRect());
            RemoveStrokeCommand* removeCommand = new RemoveStrokeCommand(this, pageNum, strokesToDelete[i]);
            undoStack.push(removeCommand);
        }
    }
}

void Widget::startMovingSelection(QPointF mousePos)
{
    currentDocument->setDocumentChanged(true);
    emit modified();

    int pageNum = getPageFromMousePos(mousePos);
    previousPagePos = getPagePosFromMousePos(mousePos, pageNum);
    currentState = state::MOVING_SELECTION;
}

void Widget::continueMovingSelection(QPointF mousePos)
{
    int pageNum = getPageFromMousePos(mousePos);
    QPointF pagePos = getPagePosFromMousePos(mousePos, pageNum);
//    currentSelection.move(1 * (pagePos - previousPagePos));

    QPointF delta = (pagePos - previousPagePos);

    QTransform transform;
    transform = transform.translate(delta.x(), delta.y());

//    currentSelection.transform(transform, pageNum);
    TransformSelectionCommand* transSelectCommand = new TransformSelectionCommand(this, pageNum, transform);
    undoStack.push(transSelectCommand);

    previousPagePos = pagePos;
//    update(currentSelection.selectionPolygon.boundingRect().toRect());
}

int Widget::getPageFromMousePos(QPointF mousePos)
{
    qreal y = mousePos.y() - currentCOSPos.y();
    int pageNum = 0;
    while (y > zoom * (currentDocument->pages[pageNum].getHeight() ) + PAGE_GAP)
    {
        y -= (currentDocument->pages[pageNum].getHeight() * zoom) + PAGE_GAP;
        pageNum += 1;
        if (pageNum >= currentDocument->pages.size())
        {
            pageNum = currentDocument->pages.size() - 1;
            break;
        }
    }
    return pageNum;
}

int Widget::getCurrentPage()
{
    QPoint globalMousePos = parentWidget()->mapToGlobal(QPoint(0,0)) + QPoint(parentWidget()->size().width()/2, parentWidget()->size().height()/2);
    QPoint pos = this->mapFromGlobal(globalMousePos);
    int pageNum = this->getPageFromMousePos(pos);

    return pageNum;
//    return getPageFromMousePos(QPointF(0.0, 2.0));
}

QPointF Widget::getPagePosFromMousePos(QPointF mousePos, int pageNum)
{
    qreal x = mousePos.x();
    qreal y = mousePos.y();
    for (int i = 0; i < pageNum; ++i)
    {
//        y -= (currentDocument->pages[i].getHeight() * zoom + PAGE_GAP); // THIS DOESN'T WORK PROPERLY (should be floor(...getHeight(), or just use pageBuffer[i].height())
        y -= (pageBuffer[i].height() + PAGE_GAP);
    }
//    y -= (pageNum) * (currentDocument->pages[0].getHeight() * zoom + PAGE_GAP);

    QPointF pagePos = (QPointF(x,y)) / zoom;


    return pagePos;
}

QPointF Widget::getAbsolutePagePosFromMousePos(QPointF mousePos)
{
    int pageNum = getPageFromMousePos(mousePos);
    QPointF pagePos = getPagePosFromMousePos(mousePos, pageNum);

    qreal y = 0.0;
    for (int i = 0; i < pageNum; ++i)
    {
        y += (pageBuffer[i].height() + PAGE_GAP);
    }
    y *= zoom;

    pagePos.setY(y + pagePos.y());

    return pagePos;
}

void Widget::newFile()
{
    letGoSelection();

    delete currentDocument;
    currentDocument = new MrDoc::Document();
    pageBuffer.clear();
    undoStack.clear();
    updateAllPageBuffers();
    QRect widgetGeometry = getWidgetGeometry();
    resize(widgetGeometry.width(), widgetGeometry.height());

    emit modified();

    update();
}

void Widget::zoomIn()
{
    qreal newZoom = zoom * ZOOM_STEP;

    zoomTo(newZoom);
}

void Widget::zoomOut()
{
    qreal newZoom = zoom / ZOOM_STEP;

    zoomTo(newZoom);
}

/**
 * @brief Widget::zoomTo
 * @param newZoom
 * @todo Should probably zoom to the center of the current view.
 */
void Widget::zoomTo(qreal newZoom)
{
    if (newZoom > MAX_ZOOM)
    {
        newZoom = MAX_ZOOM;
    }
    if (newZoom < MIN_ZOOM)
    {
        newZoom = MIN_ZOOM;
    }
    if (newZoom == zoom)
    {
        return;
    }

    zoom = newZoom;

    int prevHMax = scrollArea->horizontalScrollBar()->maximum();
    int prevVMax = scrollArea->verticalScrollBar()->maximum();

    int prevH = scrollArea->horizontalScrollBar()->value();
    int prevV = scrollArea->verticalScrollBar()->value();

    updateAllPageBuffers();
    setGeometry(getWidgetGeometry());

    int newHMax = scrollArea->horizontalScrollBar()->maximum();
    int newVMax = scrollArea->verticalScrollBar()->maximum();

    int newH, newV;

    if (prevHMax != 0)
    {
        newH = double(prevH) / prevHMax * newHMax;
    } else {
        newH = newHMax / 2;
    }
    if (prevVMax != 0)
    {
        newV = double(prevV) / prevVMax * newVMax;
    } else {
        newV = newVMax / 2;
    }

    scrollArea->horizontalScrollBar()->setValue(newH);
    scrollArea->verticalScrollBar()->setValue(newV);

    update();
}

void Widget::zoomFitWidth()
{
    int pageNum = getCurrentPage();

    QSize widgetSize = this->parentWidget()->size();
    qreal newZoom = widgetSize.width() / currentDocument->pages[pageNum].getWidth();

    zoomTo(newZoom);
}

void Widget::zoomFitHeight()
{
    int pageNum = getCurrentPage();

    QSize widgetSize = this->parentWidget()->size();
    qreal newZoom = widgetSize.height() / currentDocument->pages[pageNum].getHeight();

    zoomTo(newZoom);
}

void Widget::pageFirst()
{
    scrollDocumentToPageNum(0);
}

void Widget::pageLast()
{
    scrollDocumentToPageNum(currentDocument->pages.size()-1);
}

void Widget::pageUp()
{
//    int pageNum = getPageFromMousePos(QPointF(0.0,1.0)); // curret upper page displayed
    int pageNum = getCurrentPage();
    --pageNum;
    scrollDocumentToPageNum(pageNum);
}

void Widget::pageDown()
{
    //    int pageNum = getPageFromMousePos(QPointF(0.0,1.0)); // curret upper page displayed
    int pageNum = getCurrentPage();
    ++pageNum;

    if (pageNum >= currentDocument->pages.size())
    {
        pageAddEnd();
    }

    scrollDocumentToPageNum(pageNum);
}

void Widget::pageAddBefore()
{
    int pageNum = getCurrentPage();
    AddPageCommand* addPageCommand = new AddPageCommand(this, pageNum);
    undoStack.push(addPageCommand);
    setGeometry(getWidgetGeometry());
    update();
    currentDocument->setDocumentChanged(true);
    emit modified();
}

void Widget::pageAddAfter()
{
    int pageNum = getCurrentPage() + 1;
    AddPageCommand* addPageCommand = new AddPageCommand(this, pageNum);
    undoStack.push(addPageCommand);
    setGeometry(getWidgetGeometry());
    update();
    currentDocument->setDocumentChanged(true);

    emit modified();
}

void Widget::pageAddBeginning()
{
    AddPageCommand* addPageCommand = new AddPageCommand(this, 0);
    undoStack.push(addPageCommand);
    setGeometry(getWidgetGeometry());
    update();
    currentDocument->setDocumentChanged(true);

    emit modified();
}

void Widget::pageAddEnd()
{
    AddPageCommand* addPageCommand = new AddPageCommand(this, currentDocument->pages.size());
    undoStack.push(addPageCommand);
    setGeometry(getWidgetGeometry());
    update();
    currentDocument->setDocumentChanged(true);

    emit modified();
}

void Widget::pageRemove()
{
    if (currentDocument->pages.size() > 1)
    {
        int pageNum = getCurrentPage();
        RemovePageCommand* removePageCommand = new RemovePageCommand(this, pageNum);
        undoStack.push(removePageCommand);
        setGeometry(getWidgetGeometry());
        update();
        currentDocument->setDocumentChanged(true);
        emit modified();
    }
}

void Widget::scrollDocumentToPageNum(int pageNum)
{
    if (pageNum >= currentDocument->pages.size())
    {
        return; // page doesn't exist
    }
    if (pageNum < 0)
    {
        pageNum = 0;
    }
    qreal y = 0.0;
//    qreal x = currentCOSPos.x();
    for (int i = 0; i < pageNum; ++i)
    {
        y += (currentDocument->pages[i].getHeight()) * zoom + PAGE_GAP;
    }

    scrollArea->verticalScrollBar()->setValue(y);

//    currentCOSPos = QPointF(x, y);
//    updateAllPageBuffers();
//    update();
}


void Widget::setCurrentTool(tool toolID)
{
    if (currentState == state::IDLE || currentState == state::SELECTED)
    {
        if (toolID == tool::SELECT && currentState == state::SELECTED)
        {
            letGoSelection();
        }
        currentTool = toolID;
        if (toolID == tool::PEN)
            setCursor(penCursor);
        if (toolID == tool::RULER)
            setCursor(rulerCursor);
        if (toolID == tool::CIRCLE)
            setCursor(circleCursor);
        if (toolID == tool::ERASER)
            setCursor(eraserCursor);
        if (toolID == tool::SELECT)
            setCursor(Qt::CrossCursor);
        if (toolID == tool::HAND)
            setCursor(Qt::OpenHandCursor);
    }
}

void Widget::setDocument(MrDoc::Document* newDocument)
{
    delete currentDocument;
    currentDocument = newDocument;
    undoStack.clear();
    pageBuffer.clear();
    zoomFitWidth();
}

void Widget::copy()
{
    clipboard = currentSelection;
    update();
}

void Widget::paste()
{
    MrDoc::Selection tmpSelection = clipboard;
    tmpSelection.pageNum = getCurrentPage();

    QPoint globalMousePos = parentWidget()->mapToGlobal(QPoint(0,0)) + QPoint(parentWidget()->size().width()/2, parentWidget()->size().height()/2);
    QPoint mousePos = this->mapFromGlobal(globalMousePos);
    QPointF selectionPos = getPagePosFromMousePos(mousePos, getCurrentPage()) - tmpSelection.selectionPolygon.boundingRect().center();

    QTransform myTrans;
    myTrans = myTrans.translate(selectionPos.x(), selectionPos.y());

    for (int i = 0; i < tmpSelection.strokes.size(); ++i)
    {
        tmpSelection.strokes[i].points = myTrans.map(tmpSelection.strokes[i].points);
    }
    tmpSelection.finalize();
    tmpSelection.updateBuffer(zoom);

    undoStack.beginMacro("Paste");
    if (currentState == state::SELECTED)
    {
        letGoSelection();
    }
    PasteCommand *pasteCommand = new PasteCommand(this, tmpSelection);
    undoStack.push(pasteCommand);
    undoStack.endMacro();

    currentDocument->setDocumentChanged(true);
    emit modified();
}

void Widget::cut()
{
    CutCommand* cutCommand = new CutCommand(this);
    undoStack.push(cutCommand);
//    clipboard = currentSelection;
//    currentSelection = Selection();
//    currentState = state::IDLE;
//    update();
}

void Widget::undo()
{
    if (undoStack.canUndo() && (currentState == state::IDLE || currentState == state::SELECTED))
    {
        undoStack.undo();
    }
}

void Widget::redo()
{
    if (undoStack.canRedo() && (currentState == state::IDLE || currentState == state::SELECTED))
    {
        undoStack.redo();
    }
}

void Widget::setCurrentState(state newState)
{
    currentState = newState;
}

Widget::state Widget::getCurrentState()
{
    return currentState;
}

void Widget::setCurrentColor(QColor newColor)
{
    currentColor = newColor;
    if (currentState == state::SELECTED)
    {
        ChangeColorOfSelectionCommand *changeColorCommand = new ChangeColorOfSelectionCommand(this, newColor);
        undoStack.push(changeColorCommand);
        currentSelection.updateBuffer(zoom);
    }
}

QColor Widget::getCurrentColor()
{
    return currentColor;
}

void Widget::veryFine()
{
    setCurrentPenWidth(veryFinePenWidth);
    emit updateGUI();
}

void Widget::fine()
{
    setCurrentPenWidth(finePenWidth);
    emit updateGUI();
}

void Widget::medium()
{
    setCurrentPenWidth(mediumPenWidth);
    emit updateGUI();
}

void Widget::thick()
{
    setCurrentPenWidth(thickPenWidth);
    emit updateGUI();
}

void Widget::veryThick()
{
    setCurrentPenWidth(veryThickPenWidth);
    emit updateGUI();
}

void Widget::rotateSelection(qreal angle)
{
    QTransform rotateTrans;

    qreal dx = currentSelection.selectionPolygon.boundingRect().center().x();
    qreal dy = currentSelection.selectionPolygon.boundingRect().center().y();

    rotateTrans = rotateTrans.translate(dx, dy).rotate(-angle).translate(-dx, -dy);
    TransformSelectionCommand* transCommand = new TransformSelectionCommand(this, currentSelection.pageNum, rotateTrans);
    undoStack.push(transCommand);
    currentSelection.finalize();
    currentSelection.updateBuffer(zoom);
    update();
}

void Widget::setCurrentPattern(QVector<qreal> newPattern)
{
    currentPattern = newPattern;
    emit updateGUI();
}

QVector<qreal> Widget::getCurrentPattern()
{
    return currentPattern;
}

void Widget::solidPattern()
{
    setCurrentPattern(MrDoc::Stroke::solidLinePattern);
}

void Widget::dashPattern()
{
    setCurrentPattern(MrDoc::Stroke::dashLinePattern);
}

void Widget::dashDotPattern()
{
    setCurrentPattern(MrDoc::Stroke::dashDotLinePattern);
}

void Widget::dotPattern()
{
    setCurrentPattern(MrDoc::Stroke::dotLinePattern);
}
