#include "widget.h"
#include "curve.h"
#include "page.h"
//#include "tictoc.h"
#include "document.h"
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
#include <qmath.h>

#define PAGE_GAP 10.0
#define ZOOM_STEP 1.2

#include <QPainter>
#include <QRectF>

Widget::Widget(QWidget *parent) : QWidget(parent)
{
    currentState = state::IDLE;

    // setup cursors
    QPixmap penCursorBitmap = QPixmap(":/images/penCursor3.png");
    QPixmap penCursorMask  = QPixmap(":/images/penCursor3Mask.png");
    penCursorBitmap.setMask(QBitmap(penCursorMask));
    penCursor = QCursor(penCursorBitmap, -1, -1);

    QPixmap eraserCursorBitmap = QPixmap(":/images/eraserCursor.png");
    QPixmap eraserCursorMask  = QPixmap(":/images/eraserCursorMask.png");
    eraserCursorBitmap.setMask(QBitmap(eraserCursorMask));
    eraserCursor = QCursor(eraserCursorBitmap, -1, -1);

    currentTool = tool::PEN;
    previousTool = tool::NONE;
    realEraser = false;
    setCursor(penCursorBitmap);

    currentDocument = new Document();

    currentPenWidth = 1.41;
    currentColor = QColor(0,0,0);
    zoom = 1;

    currentCOSPos.setX(0.0);
    currentCOSPos.setY(0.0);
    updateAllPageBuffers();
    setGeometry(getWidgetGeometry());

    parent->updateGeometry();
    parent->update();
}

void Widget::updateAllPageBuffers()
{
    pageBuffer.clear();
    for (int buffNum = 0; buffNum < currentDocument->pages.size(); ++buffNum)
    {
        updateBuffer(buffNum);
    }
}

void Widget::updateBuffer(int buffNum)
{
    Page page = currentDocument->pages.at(buffNum);
    int pixelWidth = zoom * page.getWidth();
    int pixelHeight = zoom * page.getHeight();
    QImage image(pixelWidth, pixelHeight, QImage::Format_ARGB32_Premultiplied);

    image.fill(page.backgroundColor);

    QPainter painter;
    painter.begin(&image);
    painter.setRenderHint(QPainter::Antialiasing, true);

//    currentDocument->paintPage(buffNum, painter, zoom);
    currentDocument->pages[buffNum].paint(painter, zoom);

    painter.end();

    if (pageBuffer.length() <= buffNum)
    {
        pageBuffer.append(image);
    } else {
        pageBuffer.replace(buffNum, image);
    }
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

void Widget::drawOnBuffer(QPointF from, QPointF to, qreal pressure)
{
    QPen pen;

    QPainter painter;
    painter.begin(&pageBuffer[drawingOnPage]);
    painter.setRenderHint(QPainter::Antialiasing, true);

    qreal tmpPenWidth = zoom * currentPenWidth * pressure;
    pen.setWidthF(tmpPenWidth);
    pen.setColor(currentColor);
    pen.setCapStyle(Qt::RoundCap);
    if (currentCurve.pattern != Curve::solidLinePattern)
    {
        pen.setDashPattern(currentCurve.pattern);
        pen.setDashOffset(currentDashOffset);
        if (tmpPenWidth != 0)
            currentDashOffset += 1.0/tmpPenWidth * (QLineF(zoom * from, zoom * to)).length();
    }
    painter.setPen(pen);
    painter.drawLine(zoom * from, zoom * to);

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
    QRect widgetGeometry = getWidgetGeometry();
    QPalette p(palette());
    setAutoFillBackground(true);
    setPalette(p);

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
        qDebug() << rectSource;
        painter.drawImage(event->rect(), pageBuffer.at(drawingOnPage), rectSource);
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

        painter.drawImage(rectTarget, pageBuffer.at(i), rectSource);

//        if ((selecting || selected) && i == currentSelection.pageNum)
        if ((currentState == state::SELECTING || currentState == state::SELECTED || currentState == state::MODIFYING_SELECTION) && i == currentSelection.pageNum)
        {
            currentSelection.paint(painter, zoom);
        }


//        currentYPos += (rectSource.height() + PAGE_GAP*zoom);
        painter.translate(QPointF(0.0, rectSource.height() + PAGE_GAP));
    }
//    std::cerr << "paint "; toc();
}

void Widget::mouseAndTabletEvent(QPointF mousePos, Qt::MouseButton button, Qt::MouseButtons buttons, Qt::KeyboardModifiers keyboardModifiers, QTabletEvent::PointerType pointerType, QEvent::Type eventType, qreal pressure, bool tabletEvent)
{
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

    if (currentState == Widget::state::IDLE && button == Qt::RightButton)
    {
        previousTool = currentTool;
        startSelecting(mousePos);
        emit select();
        return;
    }


    if (currentState == state::MODIFYING_SELECTION)
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

//    event->setAccepted(true);

    QPointF mousePos = QPointF(event->hiResGlobalX(), event->hiResGlobalY()) - mapToGlobal(QPoint(0,0));
    qreal pressure = event->pressure();

    QEvent::Type eventType;
    if (event->type() == QTabletEvent::TabletPress)
    {
        eventType = QEvent::MouseButtonPress;
    }
    if (event->type() == QTabletEvent::TabletMove)
    {
        eventType = QEvent::MouseMove;
    }
    if (event->type() == QTabletEvent::TabletRelease)
    {
        eventType = QEvent::MouseButtonRelease;
    }

    Qt::KeyboardModifiers keyboardModifiers = event->modifiers();

    mouseAndTabletEvent(mousePos, event->button(), event->buttons(), keyboardModifiers, event->pointerType(), eventType, pressure, true);

    penDown = true;
}

void Widget::mousePressEvent(QMouseEvent *event)
{
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

    Selection newSelection;

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

    QVector<int> curvesInSelection;

    for (int i = 0; i < currentDocument->pages[pageNum].curves.size(); ++i)
    {
        Curve curve = currentDocument->pages[pageNum].curves.at(i);
        bool containsCurve = true;
        for (int j = 0; j < curve.points.size(); ++j)
        {
            if (!currentSelection.selectionPolygon.containsPoint(curve.points.at(j), Qt::OddEvenFill)) {
                containsCurve = false;
            }
        }
        if (containsCurve)
        {
            curvesInSelection.append(i);
        }
    }

    if (curvesInSelection.size() == 0)
    {
        // nothing selected
        currentState = state::IDLE;
    } else {
        // something selected
        currentState = state::SELECTED;

        for (int i = curvesInSelection.size()-1; i >= 0; --i)
        {
            currentSelection.curves.prepend(currentDocument->pages[pageNum].curves.at(curvesInSelection.at(i)));
        }
        currentSelection.finalize();
        currentSelection.updateBuffer(zoom);

        CreateSelectionCommand* selectCommand = new CreateSelectionCommand(this, pageNum, curvesInSelection);
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

    Curve newCurve;
    newCurve.pattern = currentPattern;
    newCurve.points.append(pagePos);
    newCurve.pressures.append(1);
    newCurve.penWidth = currentPenWidth;
    newCurve.color = currentColor;
    currentCurve = newCurve;
    currentState = state::RULING;

    previousMousePos = mousePos;
    firstMousePos = mousePos;
    drawingOnPage = pageNum;
}

void Widget::continueRuling(QPointF mousePos)
{
    QPointF pagePos = getPagePosFromMousePos(mousePos, drawingOnPage);
    QPointF previousPagePos = getPagePosFromMousePos(previousMousePos, drawingOnPage);

    QPointF firstPagePos = currentCurve.points.at(0);

    QPointF oldPagePos = pagePos;

    currentDashOffset = 0.0;

    if (currentCurve.points.length() > 1)
    {
        oldPagePos = currentCurve.points.at(1);
        currentCurve.points.removeAt(1);
        currentCurve.pressures.removeAt(1);
    }

    currentCurve.points.append(pagePos);
    currentCurve.pressures.append(1);

    QRect clipRect(zoom*firstPagePos.toPoint(), zoom*pagePos.toPoint());
    QRect oldClipRect(zoom*firstPagePos.toPoint(), zoom*previousPagePos.toPoint());
    clipRect = clipRect.normalized().united(oldClipRect.normalized());
    int clipRad = zoom*currentPenWidth / 2 + 2;
    clipRect = clipRect.normalized().adjusted(-clipRad, -clipRad, clipRad, clipRad);
    updateBufferRegion(drawingOnPage, clipRect);
    drawOnBuffer(firstPagePos, pagePos, 1);

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

    if (currentCurve.points.length() > 1)
    {
        currentCurve.points.removeAt(1);
        currentCurve.pressures.removeAt(1);
    }

    currentCurve.points.append(pagePos);
    currentCurve.pressures.append(1);

    AddCurveCommand* addCommand = new AddCurveCommand(this, drawingOnPage, currentCurve);
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

    Curve newCurve;
//    newCurve.points.append(pagePos);
//    newCurve.pressures.append(1);
    newCurve.pattern = currentPattern;
    newCurve.penWidth = currentPenWidth;
    newCurve.color = currentColor;
    currentCurve = newCurve;
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

//    if (currentCurve.points.length() > 1)
//    {
//        oldPagePos = currentCurve.points.at(1);
//        currentCurve.points.removeAt(1);
//        currentCurve.pressures.removeAt(1);
//    }

    Curve oldCurve = currentCurve;

    currentCurve.points.clear();
    currentCurve.pressures.clear();

    qreal radius = QLineF(firstPagePos, pagePos).length();
    qreal phi0 = QLineF(firstPagePos, pagePos).angle() * M_PI / 180.0;

    int N = 100;
    for (int i = 0; i < N; ++i)
    {
        qreal phi = phi0 + i * (2.0 * M_PI / (N-1));
        qreal x = firstPagePos.x() + radius * cos(phi);
        qreal y = firstPagePos.y() - radius * sin(phi);
        currentCurve.points.append(QPointF(x,y));
        currentCurve.pressures.append(1.0);
    }

    QTransform scaleTrans;
    scaleTrans = scaleTrans.scale(zoom,zoom);

    QRect clipRect = scaleTrans.mapRect(currentCurve.points.boundingRect()).toRect();
    QRect oldClipRect = scaleTrans.mapRect(oldCurve.points.boundingRect()).toRect();
    clipRect = clipRect.normalized().united(oldClipRect.normalized());
    int clipRad = zoom*currentPenWidth / 2 + 2;
    clipRect = clipRect.normalized().adjusted(-clipRad, -clipRad, clipRad, clipRad);
    updateBufferRegion(drawingOnPage, clipRect);

    for (int i = 0; i < currentCurve.points.length()-1; ++i)
    {
        drawOnBuffer(currentCurve.points.at(i), currentCurve.points.at(i+1), 1);
    }


    QRect updateRect(firstMousePos.toPoint(), mousePos.toPoint());
    QRect oldUpdateRect(firstMousePos.toPoint(), previousMousePos.toPoint());
    updateRect = updateRect.normalized().united(oldUpdateRect.normalized());
    int rad = currentPenWidth * zoom / 2 + 2;
    updateRect = updateRect.normalized().adjusted(-rad, -rad, +rad, +rad);

    update(updateRect);
    update();

    previousMousePos = mousePos;
}

void Widget::stopCircling(QPointF mousePos)
{
    QPointF pagePos = getPagePosFromMousePos(mousePos, drawingOnPage);

    AddCurveCommand* addCommand = new AddCurveCommand(this, drawingOnPage, currentCurve);
    undoStack.push(addCommand);

    currentState = state::IDLE;

    update();
}

void Widget::startDrawing(QPointF mousePos, qreal pressure)
{
    currentDocument->setDocumentChanged(true);
    emit modified();

    int pageNum = getPageFromMousePos(mousePos);
    QPointF pagePos = getPagePosFromMousePos(mousePos, pageNum);

    currentDashOffset = 0.0;

    Curve newCurve;
    newCurve.pattern = currentPattern;
    newCurve.points.append(pagePos);
    newCurve.pressures.append(pressure);
    newCurve.penWidth = currentPenWidth;
    newCurve.color = currentColor;
    currentCurve = newCurve;
//    drawing = true;
    currentState = state::DRAWING;

    previousMousePos = mousePos;
    drawingOnPage = pageNum;
}

void Widget::continueDrawing(QPointF mousePos, qreal pressure)
{
    QPointF pagePos = getPagePosFromMousePos(mousePos, drawingOnPage);
    QPointF previousPagePos = getPagePosFromMousePos(previousMousePos, drawingOnPage);

    currentCurve.points.append(pagePos);
    currentCurve.pressures.append(pressure);
    drawOnBuffer(previousPagePos, pagePos, pressure);

    QRect updateRect(previousMousePos.toPoint(), mousePos.toPoint());
    int rad = currentPenWidth * zoom / 2 + 2;
    updateRect = updateRect.normalized().adjusted(-rad, -rad, +rad, +rad);

    update(updateRect);
//    update();

    previousMousePos = mousePos;
}

void Widget::stopDrawing(QPointF mousePos, qreal pressure)
{
    QPointF pagePos = getPagePosFromMousePos(mousePos, drawingOnPage);

    currentCurve.points.append(pagePos);
    currentCurve.pressures.append(pressure);

//    currentDocument->pages[drawingOnPage].curves.append(currentCurve);
    AddCurveCommand* addCommand = new AddCurveCommand(this, drawingOnPage, currentCurve);
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

    QList<Curve> *curves = &(currentDocument->pages[pageNum].curves);

    qreal eraserWidth = 10;

//    QLineF lineA = QLineF(pagePos + QPointF(-eraserWidth,-eraserWidth) / 2, pagePos + QPointF( eraserWidth,  eraserWidth) / 2);
//    QLineF lineB = QLineF(pagePos + QPointF( eraserWidth,-eraserWidth) / 2, pagePos + QPointF(-eraserWidth,  eraserWidth) / 2); // lineA and lineB form a cross X

    QLineF lineA = QLineF(pagePos + QPointF(-eraserWidth,-eraserWidth) / 2, pagePos + QPointF(-eraserWidth,  eraserWidth) / 2);
    QLineF lineB = QLineF(pagePos + QPointF(-eraserWidth, eraserWidth) / 2, pagePos + QPointF( eraserWidth,  eraserWidth) / 2);
    QLineF lineC = QLineF(pagePos + QPointF( eraserWidth, eraserWidth) / 2, pagePos + QPointF( eraserWidth, -eraserWidth) / 2);
    QLineF lineD = QLineF(pagePos + QPointF( eraserWidth,-eraserWidth) / 2, pagePos + QPointF(-eraserWidth, -eraserWidth) / 2); // lineA B C D form a square

    QRectF rectE = QRectF(pagePos + QPointF(-eraserWidth, eraserWidth) / 2, pagePos + QPointF( eraserWidth, -eraserWidth) / 2);

    QVector<int> curvesToDelete;
    QPointF iPoint;

    QPointF iPointA;
    QPointF iPointB;
    QPointF iPointC;
    QPointF iPointD;

    bool intersected;

    if (realEraser || (!realEraser && invertEraser))
    {
        for (int i = curves->size()-1; i >= 0; --i)
        {
            Curve curve = curves->at(i);
            if (rectE.intersects(curve.points.boundingRect()) || !curve.points.boundingRect().isValid() ) // this is done for speed
            {
                for (int j = 0; j < curve.points.length()-1; ++j)
                {
                    QLineF line = QLineF(curve.points.at(j), curve.points.at(j+1));
                    if (line.intersect(lineA, &iPointA) == QLineF::BoundedIntersection && iPointA != curve.points.first() && iPointA != curve.points.last())
                    {
                        iPoint = iPointA;
                        intersected = true;
                    }
                    else if (line.intersect(lineB, &iPointB) == QLineF::BoundedIntersection && iPointB != curve.points.first() && iPointB != curve.points.last())
                    {
                        iPoint = iPointB;
                        intersected = true;
                    }
                    else if (line.intersect(lineC, &iPointC) == QLineF::BoundedIntersection && iPointC != curve.points.first() && iPointC != curve.points.last())
                    {
                        iPoint = iPointC;
                        intersected = true;
                    }
                    else if (line.intersect(lineD, &iPointD) == QLineF::BoundedIntersection && iPointD != curve.points.first() && iPointD != curve.points.last())
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
//                        if (iPoint != curve.points.first() && iPoint != curve.points.last())
                        {
                            Curve splitCurve = curve;
                            splitCurve.points = splitCurve.points.mid(0,j+1);
                            splitCurve.points.append(iPoint);
                            splitCurve.pressures = splitCurve.pressures.mid(0,j+1);
                            qreal lastPressure = splitCurve.pressures.last();
                            splitCurve.pressures.append(lastPressure);
                            curve.points = curve.points.mid(j+1);
                            curve.points.prepend(iPoint);
                            curve.pressures = curve.pressures.mid(j+1);
                            qreal firstPressure = curve.pressures.first();
                            curve.pressures.prepend(firstPressure);
                            RemoveCurveCommand *removeCurveCommand = new RemoveCurveCommand(this, pageNum, i, false);
                            undoStack.push(removeCurveCommand);
                            AddCurveCommand *addCurveCommand = new AddCurveCommand(this, pageNum, curve, i, false);
                            undoStack.push(addCurveCommand);
                            addCurveCommand = new AddCurveCommand(this, pageNum, splitCurve, i, false);
                            undoStack.push(addCurveCommand);
//                            curves->insert(i, splitCurve);
                            i += 2;
                            qDebug() << iPoint;
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

    for (int i = 0; i < curves->size(); ++i)
    {
        const Curve curve = curves->at(i);
        if (rectE.intersects(curve.points.boundingRect()) || !curve.points.boundingRect().isValid() ) // this is done for speed
        {
            for (int j = 0; j < curve.points.length()-1; ++j)
            {
                QLineF line = QLineF(curve.points.at(j), curve.points.at(j+1));
                if (line.intersect(lineA, &iPoint) == QLineF::BoundedIntersection ||
                    line.intersect(lineB, &iPoint) == QLineF::BoundedIntersection ||
                    line.intersect(lineC, &iPoint) == QLineF::BoundedIntersection ||
                    line.intersect(lineD, &iPoint) == QLineF::BoundedIntersection ||
                    rectE.contains(curve.points.at(j)) ||
                    rectE.contains(curve.points.at(j+1)))
                {
                    curvesToDelete.append(i);
                    break;
                }
            }
        }
    }

    if (curvesToDelete.size() > 0)
    {
        currentDocument->setDocumentChanged(true);
        emit modified();

        QRect updateRect;
        std::sort(curvesToDelete.begin(), curvesToDelete.end(), std::greater<int>());
        for (int i = 0; i < curvesToDelete.size(); ++i)
        {
            updateRect = updateRect.united(currentDocument->pages[pageNum].curves.at(curvesToDelete.at(i)).points.boundingRect().toRect());
            RemoveCurveCommand* removeCommand = new RemoveCurveCommand(this, pageNum, curvesToDelete[i]);
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
    currentState = state::MODIFYING_SELECTION;
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

//    QPointF pagePos = (QPointF(x,y) - currentCOSPos) / zoom;
    QPointF pagePos = (QPointF(x,y)) / zoom;


    return pagePos;
}

void Widget::newFile()
{
//    if (currentDocument->getDocumentChanged())
//    {
//        return;
//    }

    letGoSelection();

    delete currentDocument;
    currentDocument = new Document();
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
//    QSize widgetSize = this->size();
//    QPointF widgetMidPoint = QPointF(widgetSize.width(), widgetSize.height());
//    currentCOSPos = currentCOSPos + (1 - ZOOM_STEP) * (0.5 * widgetMidPoint - currentCOSPos);

    int previousH = scrollArea->horizontalScrollBar()->value();
    int previousV = scrollArea->verticalScrollBar()->value();

    zoom *= ZOOM_STEP;
    if (zoom > MAX_ZOOM)
    {
        zoom = MAX_ZOOM;
    }
    updateAllPageBuffers();
    setGeometry(getWidgetGeometry());
    update();

    scrollArea->horizontalScrollBar()->setValue(ZOOM_STEP * previousH + (ZOOM_STEP - 1) * scrollArea->size().width()/2);
    scrollArea->verticalScrollBar()->setValue(ZOOM_STEP * previousV + (ZOOM_STEP - 1) * scrollArea->size().height()/2);
}

void Widget::zoomOut()
{
//    QSize widgetSize = this->size();
//    QPointF widgetMidPoint = QPointF(widgetSize.width(), widgetSize.height());
//    currentCOSPos = currentCOSPos + (1 - 1/ZOOM_STEP) * (0.5 * widgetMidPoint - currentCOSPos);
    int previousH = scrollArea->horizontalScrollBar()->value();
    int previousV = scrollArea->verticalScrollBar()->value();

    zoom /= ZOOM_STEP;
    if (zoom < MIN_ZOOM)
        zoom = MIN_ZOOM;
    updateAllPageBuffers();
    setGeometry(getWidgetGeometry());
    update();

    scrollArea->horizontalScrollBar()->setValue(1/ZOOM_STEP * previousH - (ZOOM_STEP - 1) * scrollArea->size().width()/2);
    scrollArea->verticalScrollBar()->setValue(1/ZOOM_STEP * previousV - (ZOOM_STEP - 1) * scrollArea->size().height()/2);
}

void Widget::zoomTo(qreal newZoom)
{
    if (newZoom >= MIN_ZOOM && newZoom <= MAX_ZOOM)
        zoom = newZoom;
}

void Widget::zoomFitWidth()
{
    QSize widgetSize = this->parentWidget()->size();
    int pageNum = getCurrentPage();
    zoom = widgetSize.width() / currentDocument->pages[pageNum].getWidth();

    updateAllPageBuffers();
    setGeometry(getWidgetGeometry());
    update();
}

void Widget::zoomFitHeight()
{
    QSize widgetSize = this->parentWidget()->size();
    int pageNum = getCurrentPage();
    zoom = widgetSize.height() / currentDocument->pages[pageNum].getHeight();

    updateAllPageBuffers();
    setGeometry(getWidgetGeometry());
    update();
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
            setCursor(penCursor);
        if (toolID == tool::CIRCLE)
            setCursor(penCursor);
        if (toolID == tool::ERASER)
            setCursor(eraserCursor);
        if (toolID == tool::SELECT)
            setCursor(Qt::CrossCursor);
        if (toolID == tool::HAND)
            setCursor(Qt::OpenHandCursor);
    }
}

void Widget::setDocument(Document* newDocument)
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
    if (currentState == state::SELECTED)
    {
        letGoSelection();
    }
    currentSelection = clipboard;
    currentSelection.pageNum = getCurrentPage();

//    qreal dx = currentDocument->pages[getCurrentPage()].getWidth() / 2.0 - currentSelection.selectionPolygon.boundingRect().center().x();
//    qreal dy = currentDocument->pages[getCurrentPage()].getHeight() / 2.0 - currentSelection.selectionPolygon.boundingRect().center().y();

    QPoint globalMousePos = parentWidget()->mapToGlobal(QPoint(0,0)) + QPoint(parentWidget()->size().width()/2, parentWidget()->size().height()/2);
    QPoint mousePos = this->mapFromGlobal(globalMousePos);
    QPointF selectionPos = getPagePosFromMousePos(mousePos, getCurrentPage()) - currentSelection.selectionPolygon.boundingRect().center();
//    QPointF selectionPos = getPagePosFromMousePos(QPointF(scrollArea->width()/2.0, scrollArea->height()/2.0), getCurrentPage()) - currentSelection.selectionPolygon.boundingRect().center();

    QTransform myTrans;
    myTrans = myTrans.translate(selectionPos.x(), selectionPos.y());

    for (int i = 0; i < currentSelection.curves.size(); ++i)
    {
        currentSelection.curves[i].points = myTrans.map(currentSelection.curves[i].points);
    }
    currentSelection.finalize();

    currentState = state::SELECTED;
    update();
}

void Widget::cut()
{
    clipboard = currentSelection;
    currentSelection = Selection();
    currentState = state::IDLE;
    update();
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
    TransformSelectionCommand* transCommand = new TransformSelectionCommand(this, getCurrentPage(), rotateTrans);
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
    setCurrentPattern(Curve::solidLinePattern);
}

void Widget::dashPattern()
{
    setCurrentPattern(Curve::dashLinePattern);
}

void Widget::dashDotPattern()
{
    setCurrentPattern(Curve::dashDotLinePattern);
}

void Widget::dotPattern()
{
    setCurrentPattern(Curve::dotLinePattern);
}
