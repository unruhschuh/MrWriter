#include "widget.h"
#include "mrdoc.h"
#include "commands.h"
#include "tabletapplication.h"
#include "tools.h"

#include <QClipboard>
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
// Widget::Widget(QWidget *parent) : QOpenGLWidget(parent)
{
  QSettings settings;
  //    qDebug() << settings.applicationVersion();

  currentState = state::IDLE;

  // setup cursors
  QPixmap penCursorBitmap = QPixmap(":/images/penCursor3.png");
  QPixmap penCursorMask = QPixmap(":/images/penCursor3Mask.png");
  penCursorBitmap.setMask(QBitmap(penCursorMask));
  penCursor = QCursor(penCursorBitmap, -1, -1);

  QPixmap circleCursorBitmap = QPixmap(":/images/circleCursor.png");
  QPixmap circleCursorMask = QPixmap(":/images/circleCursorMask.png");
  circleCursorBitmap.setMask(QBitmap(circleCursorMask));
  circleCursor = QCursor(circleCursorBitmap, -1, -1);

  QPixmap rulerCursorBitmap = QPixmap(":/images/rulerCursor.png");
  QPixmap rulerCursorMask = QPixmap(":/images/rulerCursorMask.png");
  rulerCursorBitmap.setMask(QBitmap(rulerCursorMask));
  rulerCursor = QCursor(rulerCursorBitmap, -1, -1);

  QPixmap eraserCursorBitmap = QPixmap(":/images/eraserCursor.png");
  QPixmap eraserCursorMask = QPixmap(":/images/eraserCursorMask.png");
  eraserCursorBitmap.setMask(QBitmap(eraserCursorMask));
  eraserCursor = QCursor(eraserCursorBitmap, -1, -1);

  QPixmap strokeEraserCursorBitmap = QPixmap(":/images/strokeEraserCursor.png");
  QPixmap strokeEraserCursorMask = QPixmap(":/images/strokeEraserCursorMask.png");
  strokeEraserCursorBitmap.setMask(QBitmap(strokeEraserCursorMask));
  strokeEraserCursor = QCursor(strokeEraserCursorBitmap, -1, -1);

  currentTool = tool::PEN;
  previousTool = tool::NONE;
  setCursor(penCursorBitmap);

  currentDocument = MrDoc::Document();

  currentPenWidth = 1.41;
  currentColor = QColor(0, 0, 0);
  m_zoom = 1.0;

  showGrid = false;
  snapToGrid = false;
  gridWidth = 14.1732;

  currentCOSPos.setX(0.0);
  currentCOSPos.setY(0.0);
  setGeometry(getWidgetGeometry());

  parent->updateGeometry();
  parent->update();

  updateTimer = new QTimer(this);
  connect(updateTimer, SIGNAL(timeout()), this, SLOT(updateWhileDrawing()));

  updateDirtyTimer = new QTimer(this);
  connect(updateDirtyTimer, SIGNAL(timeout()), this, SLOT(updateAllDirtyBuffers()));
  updateDirtyTimer->setInterval(15);

//  updateAllPageBuffers();
}

void Widget::showEvent( QShowEvent* event ) {
    QWidget::showEvent( event );
  updateAllPageBuffers();
}

size_t Widget::firstVisiblePage() const
{
  size_t firstVisiblePageNum = getPageFromMousePos(this->mapFromGlobal(scrollArea->mapToGlobal(QPoint(0, 0))));
  return firstVisiblePageNum;
}

size_t Widget::lastVisiblePage() const
{
  size_t lastVisiblePageNum = getPageFromMousePos(this->mapFromGlobal(scrollArea->mapToGlobal(QPoint(0, static_cast<int>(scrollArea->height())))));
  return lastVisiblePageNum;
}

bool Widget::pageVisible(size_t buffNum) const
{
  size_t firstVisiblePangeNum = firstVisiblePage();
  size_t lastVisiblePangeNum = lastVisiblePage();

  if (buffNum >= firstVisiblePangeNum && buffNum <= lastVisiblePangeNum)
  {
    return true;
  } else {
    return false;
  }
}

void Widget::updateAllPageBuffers(bool force)
{
  if (pageBuffer.empty())
  {
    for (size_t pageNum = 0; pageNum < currentDocument.pages.size(); ++pageNum)
    {
      pageBuffer.push_back(QPixmap(0,0));
    }
  }
  for (size_t pageNum = 0; pageNum < currentDocument.pages.size(); ++pageNum)
  {
    if (pageVisible(pageNum))
    {
      if ( force ||
           ( pageBuffer.at(pageNum).width() != currentDocument.pages.at(pageNum).pixelWidth(m_zoom, devicePixelRatio()) ||
             pageBuffer.at(pageNum).height() != currentDocument.pages.at(pageNum).pixelHeight(m_zoom, devicePixelRatio()) ) )
      {
        updateBuffer(pageNum);
      }
    } else {
      pageBuffer.at(pageNum) = QPixmap(0,0);
    }
  }
}

/**
 * @brief Widget::updateImageBuffer for concurrent update with QFuture in Widget::updateAllPageBuffers() since QPixmap cannot be used concurrently
 */
void Widget::updateImageBuffer(size_t buffNum)
{
  const auto &page = currentDocument.pages.at(buffNum);
  int pixelWidth = page.pixelWidth(m_zoom, devicePixelRatio());
  int pixelHeight = page.pixelHeight(m_zoom, devicePixelRatio());
  QImage image(pixelWidth, pixelHeight, QImage::Format_ARGB32_Premultiplied);
  image.setDevicePixelRatio(devicePixelRatio());

  image.fill(page.backgroundColor());

  QPainter painter;
  painter.begin(&image);
  painter.setRenderHint(QPainter::Antialiasing, true);

  if (showGrid)
  {
    drawGrid(painter, buffNum);
  }

  currentDocument.pages[buffNum].paint(painter, m_zoom);

  painter.end();

  pageImageBufferMutex.lock();
  pageImageBuffer.at(buffNum) = image;
  pageImageBufferMutex.unlock();
}

void Widget::updateBuffer(size_t buffNum)
{
  const auto &page = currentDocument.pages.at(buffNum);
  int pixelWidth = page.pixelWidth(m_zoom, devicePixelRatio());
  int pixelHeight = page.pixelHeight(m_zoom, devicePixelRatio());
  QPixmap pixmap(pixelWidth, pixelHeight);
  pixmap.setDevicePixelRatio(devicePixelRatio());

  pixmap.fill(page.backgroundColor());

  QPainter painter;
  painter.begin(&pixmap);
  painter.setRenderHint(QPainter::Antialiasing, true);

  if (showGrid)
  {
    drawGrid(painter, buffNum);
  }

  currentDocument.pages[buffNum].paint(painter, m_zoom);

  painter.end();

  pageBuffer.at(buffNum) = pixmap;
}

void Widget::updateBufferRegion(size_t buffNum, QRectF const &clipRect)
{
  QPainter painter;
  painter.begin(&pageBuffer[buffNum]);
  painter.setRenderHint(QPainter::Antialiasing, true);
  painter.setClipRect(clipRect);
  painter.setClipping(true);

  painter.fillRect(clipRect, currentDocument.pages.at(buffNum).backgroundColor());

  if (showGrid)
  {
    drawGrid(painter, buffNum);
  }
  //    painter.fillRect(clipRect, Qt::red);

  QRectF paintRect = QRectF(clipRect.topLeft() / m_zoom, clipRect.bottomRight() / m_zoom);
  currentDocument.pages[buffNum].paint(painter, m_zoom, paintRect);

  painter.end();
}

void Widget::updateAllDirtyBuffers()
{
  for (size_t buffNum = 0; buffNum < currentDocument.pages.size(); ++buffNum)
  {
    QRectF const &dirtyRect = currentDocument.pages.at(buffNum).dirtyRect();
    if (!dirtyRect.isNull())
    {
      QRectF dirtyBufferRect = QRectF(dirtyRect.topLeft() * m_zoom, dirtyRect.bottomRight() * m_zoom);
      updateBufferRegion(buffNum, dirtyBufferRect);
      currentDocument.pages[buffNum].clearDirtyRect();
    }
  }
  update();
}

void Widget::drawGrid(QPainter &painter, size_t buffNum)
{
  const auto &page = currentDocument.pages.at(buffNum);
  int pixelWidth = page.pixelWidth(m_zoom, devicePixelRatio());
  int pixelHeight = page.pixelHeight(m_zoom, devicePixelRatio());

  int gridPixelWidth = static_cast<int>(m_zoom * gridWidth * devicePixelRatio());

  int numGridLinesX = static_cast<int>(pixelWidth / gridPixelWidth);
  int numGridLinesY = static_cast<int>(pixelHeight / gridPixelWidth);

  QPen pen;
  pen.setColor(MrDoc::gridColor);
  pen.setCapStyle(Qt::FlatCap);
  pen.setWidthF(0.5 * devicePixelRatioF());
  painter.setPen(pen);
  for (int i = 0; i < numGridLinesY; i++)
  {
    painter.drawLine(QLineF(0.0, static_cast<qreal>(i+1) * gridWidth * m_zoom, pixelWidth, static_cast<qreal>(i+1) * gridWidth * m_zoom));
  }
  for (int i = 0; i < numGridLinesX; i++)
  {
    painter.drawLine(QLineF(static_cast<qreal>(i+1) * gridWidth * m_zoom, 0, static_cast<qreal>(i+1) * gridWidth * m_zoom, pixelHeight));
  }
}

void Widget::drawOnBuffer(bool last)
{
  QPainter painter;
  painter.begin(&pageBuffer[drawingOnPage]);
  painter.setRenderHint(QPainter::Antialiasing, true);

  currentStroke.paint(painter, m_zoom, last);
}

QRect Widget::getWidgetGeometry() const
{
  int width = 0;
  int height = 0;
  for (size_t i = 0; i < static_cast<size_t>(pageBuffer.size()); ++i)
  {
    height += currentDocument.pages.at(i).pixelHeight(m_zoom, 1 /*devicePixelRatio()*/) + PAGE_GAP;
    if (currentDocument.pages.at(i).pixelWidth(m_zoom, 1 /*devicePixelRatio()*/) > width)
    {
      width = currentDocument.pages.at(i).pixelWidth(m_zoom, 1); //devicePixelRatio());
    }

    /*
    height += pageBuffer[i].height()/devicePixelRatio() + PAGE_GAP;
    if (pageBuffer[i].width()/devicePixelRatio() > width)
      width = pageBuffer[i].width()/devicePixelRatio();
      */
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

  QPainter painter(this);

  if (currentState == state::DRAWING)
  {
    QRectF rectSource;
    QTransform trans;
    for (size_t i = 0; i < drawingOnPage; ++i)
    {
      //trans = trans.translate(0, -(pageBuffer.at(i).height() + PAGE_GAP*devicePixelRatio()));
      trans = trans.translate(0, -(currentDocument.pages.at(i).pixelHeight(m_zoom, devicePixelRatio()) + PAGE_GAP*devicePixelRatio()));
    }
    trans = trans.scale(devicePixelRatio(),devicePixelRatio());
    rectSource = trans.mapRect(event->rect());

    //        QPixmap tmp = QPixmap::fromImage(pageBuffer.at(drawingOnPage));
    painter.drawPixmap(event->rect(), pageBuffer[drawingOnPage], rectSource);

    //        painter.drawImage(event->rect(), pageBuffer.at(drawingOnPage), rectSource);
    return;
  }

  //    painter.setRenderHint(QPainter::Antialiasing, true);

  for (size_t i = 0; i < static_cast<size_t>(pageBuffer.size()); ++i)
  {
    QRectF rectSource;
    rectSource.setTopLeft(QPointF(0.0, 0.0));
    rectSource.setWidth(pageBuffer.at(i).width());
    rectSource.setHeight(pageBuffer.at(i).height());

    QRectF rectTarget;
    //        rectTarget.setTopLeft(QPointF(0.0, currentYPos));
    rectTarget.setTopLeft(QPointF(0.0, 0.0));
    rectTarget.setWidth(pageBuffer.at(i).width()/devicePixelRatio());
    rectTarget.setHeight(pageBuffer.at(i).height()/devicePixelRatio());

    painter.drawPixmap(rectTarget, pageBuffer.at(i), rectSource);

    if ((currentState == state::SELECTING || currentState == state::SELECTED || currentState == state::MOVING_SELECTION ||
         currentState == state::RESIZING_SELECTION || currentState == state::ROTATING_SELECTION) &&
        i == currentSelection.pageNum())
    {
      currentSelection.paint(painter, m_zoom);
    }

    auto & page = currentDocument.pages.at(i);
    int pixelHeight = page.pixelHeight(m_zoom, devicePixelRatio());
    painter.translate(QPointF(0.0, pixelHeight + PAGE_GAP * devicePixelRatio()));
    //painter.translate(QPointF(0.0, rectSource.height()/devicePixelRatio() + PAGE_GAP * devicePixelRatio()));
  }
}

void Widget::updateWhileDrawing()
{
  update(currentUpdateRect);
  currentUpdateRect.setWidth(0);
  currentUpdateRect.setHeight(0);
}

void Widget::mouseAndTabletEvent(QPointF mousePos, Qt::MouseButton button, Qt::MouseButtons buttons, Qt::KeyboardModifiers keyboardModifiers,
                                 QTabletEvent::PointerType pointerType, QEvent::Type eventType, qreal pressure, bool tabletEvent)
{
  // Under Linux the keyboard modifiers are not reported to tabletevent. this should work
  // everywhere.
  keyboardModifiers = qApp->queryKeyboardModifiers();

  // benchmark
  if (eventType == QEvent::MouseButtonPress)
  {
    timer.start();
    count = 1;
  }
  if (eventType == QEvent::MouseMove)
  {
    ++count;
  }
  if (eventType == QEvent::MouseButtonRelease)
  {
    ++count;
    //    qInfo() << static_cast<qreal>(count) / static_cast<qreal>(timer.elapsed()) * 1000.0;
  }
  // end benchmark

  size_t pageNum = getPageFromMousePos(mousePos);
  QPointF pagePos = getPagePosFromMousePos(mousePos, pageNum);

  bool invertEraser;
  if (keyboardModifiers & Qt::ShiftModifier)
  {
    invertEraser = true;
  }
  else
  {
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
      int dx = 1 * static_cast<int>(mousePos.x() - previousMousePos.x());
      int dy = 1 * static_cast<int>(mousePos.y() - previousMousePos.y());

      scrollArea->horizontalScrollBar()->setValue(scrollArea->horizontalScrollBar()->value() - dx);
      scrollArea->verticalScrollBar()->setValue(scrollArea->verticalScrollBar()->value() - dy);

      qInfo() << dy;

      mousePos -= QPointF(dx, dy);

      previousMousePos = mousePos;
      return;
    }
    return;
  }

  if (currentState == state::SELECTED)
  {
    if (eventType == QEvent::MouseButtonPress)
    {
      using GrabZone = MrDoc::Selection::GrabZone;
      GrabZone grabZone = currentSelection.grabZone(pagePos, m_zoom);
      if (grabZone == GrabZone::None)
      {
        letGoSelection();
        update();
        //      return;
      }
      else if (grabZone == GrabZone::Move)
      {
        // move selection
        startMovingSelection(mousePos);
        return;
      }
      else if (grabZone == GrabZone::Rotate)
      {
        startRotatingSelection(mousePos);
        return;
      }
      else
      {
        // resize selection
        startResizingSelection(mousePos, grabZone);
        return;
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
    if (keyboardModifiers & Qt::ShiftModifier)
    {
      currentTool = Widget::tool::RECT_SELECT;
      emit rectSelect();
    } else {
      currentTool = Widget::tool::SELECT;
      emit select();
    }
    startSelecting(mousePos);
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
      setCurrentState(state::SELECTED);
      setPreviousTool();
    }
  }

  if (currentState == state::ROTATING_SELECTION)
  {
    if (eventType == QEvent::MouseMove)
    {
      continueRotatingSelection(mousePos);
      return;
    }
    if (eventType == QEvent::MouseButtonRelease)
    {
      stopRotatingSelection(mousePos);
      setPreviousTool();
    }
  }

  if (currentState == state::RESIZING_SELECTION)
  {
    if (eventType == QEvent::MouseMove)
    {
      continueResizingSelection(mousePos);
      return;
    }
    if (eventType == QEvent::MouseButtonRelease)
    {
      stopResizingSelection(mousePos);
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

  if (currentState == state::RECTING)
  {
    if (eventType == QEvent::MouseMove)
    {
      continueRecting(mousePos);
      return;
    }
    if (eventType == QEvent::MouseButtonRelease)
    {
      stopRecting(mousePos);
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
        if (currentTool == tool::RECT)
        {
          startRecting(mousePos);
          return;
        }
        if (currentTool == tool::ERASER)
        {
          undoStack.beginMacro("erase");
          erase(mousePos, invertEraser);
          return;
        }
        if (currentTool == tool::STROKE_ERASER)
        {
          undoStack.beginMacro("erase");
          erase(mousePos, !invertEraser);
          return;
        }
        if (currentTool == tool::SELECT || currentTool == tool::RECT_SELECT)
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
      if (currentTool == tool::STROKE_ERASER)
      {
        erase(mousePos, !invertEraser);
      }
      if (currentTool == tool::HAND)
      {
        int dx = 1 * static_cast<int>(mousePos.x() - previousMousePos.x());
        int dy = 1 * static_cast<int>(mousePos.y() - previousMousePos.y());

        scrollArea->horizontalScrollBar()->setValue(scrollArea->horizontalScrollBar()->value() - dx);
        scrollArea->verticalScrollBar()->setValue(scrollArea->verticalScrollBar()->value() - dy);

        mousePos -= QPointF(dx, dy);

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

  QPointF mousePos = QPointF(event->hiResGlobalX(), event->hiResGlobalY()) - mapToGlobal(QPoint(0, 0));
  qreal pressure = event->pressure();

  QEvent::Type eventType;
  switch (event->type())
  {
  case QTabletEvent::TabletPress:
    eventType = QEvent::MouseButtonPress;
    penDown = true;
    break;
  case QTabletEvent::TabletMove:
    eventType = QEvent::MouseMove;
    penDown = true;
    break;
  case QTabletEvent::TabletRelease:
    eventType = QEvent::MouseButtonRelease;
    break;
  default:
    eventType = event->type();
  }

  Qt::KeyboardModifiers keyboardModifiers = event->modifiers();

  mouseAndTabletEvent(mousePos, event->button(), event->buttons(), keyboardModifiers, event->pointerType(), eventType, pressure, true);
}

void Widget::mousePressEvent(QMouseEvent *event)
{
  bool usingTablet = static_cast<TabletApplication *>(qApp)->isUsingTablet();

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
  bool usingTablet = static_cast<TabletApplication *>(qApp)->isUsingTablet();

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
  bool usingTablet = static_cast<TabletApplication *>(qApp)->isUsingTablet();

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

void Widget::mouseDoubleClickEvent(QMouseEvent* event)
{
  (void)event;
  emit quickmenu();
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
  if (previousTool == tool::RECT)
  {
    emit rect();
  }
  if (previousTool == tool::ERASER)
  {
    emit eraser();
  }
  if (previousTool == tool::STROKE_ERASER)
  {
    emit strokeEraser();
  }
  if (previousTool == tool::SELECT)
  {
    emit select();
  }
  if (previousTool == tool::RECT_SELECT)
  {
    emit rectSelect();
  }
  if (previousTool == tool::HAND)
  {
    emit hand();
  }

  previousTool = tool::NONE;
}

void Widget::startSelecting(QPointF mousePos)
{
  size_t pageNum = getPageFromMousePos(mousePos);
  QPointF pagePos = getPagePosFromMousePos(mousePos, pageNum);

  MrDoc::Selection newSelection;


  newSelection.setPageNum(pageNum);
  newSelection.setWidth(currentDocument.pages[pageNum].width());
  newSelection.setHeight(currentDocument.pages[pageNum].height());
  newSelection.appendToSelectionPolygon(pagePos);

  firstMousePos = mousePos;

  currentSelection = newSelection;

  //    selecting = true;
  currentState = state::SELECTING;
  selectingOnPage = pageNum;
}

void Widget::continueSelecting(QPointF mousePos)
{
  size_t pageNum = selectingOnPage;
  QPointF pagePos = getPagePosFromMousePos(mousePos, pageNum);
  QPointF firstPagePos = getPagePosFromMousePos(firstMousePos, pageNum);

  if (currentTool == Widget::tool::SELECT)
  {
    currentSelection.appendToSelectionPolygon(pagePos);
  } else if (currentTool == Widget::tool::RECT_SELECT) {
    QPolygonF selectionPolygon;
    selectionPolygon.append(QPointF(firstPagePos.x(), firstPagePos.y()));
    selectionPolygon.append(QPointF(firstPagePos.x(), pagePos.y()));
    selectionPolygon.append(QPointF(pagePos.x(), pagePos.y()));
    selectionPolygon.append(QPointF(pagePos.x(), firstPagePos.y()));
    currentSelection.setSelectionPolygon(selectionPolygon);
  }

  update();

  if (MrWriter::polygonIsClockwise(currentSelection.selectionPolygon()))
  {
    m_statusText = "selecting all intersecting strokes";
  }
  else
  {
    m_statusText = "selecting all contained strokes";
  }
  updateGUI();
}

void Widget::stopSelecting(QPointF mousePos)
{
  size_t pageNum = selectingOnPage;

  //continueSelecting(mousePos);

  if (currentSelection.selectionPolygon().length() < 3) // && currentTool == Widget::tool::SELECT)
  {
    double s = 10.0;
    QPolygonF selectionPolygon;
    selectionPolygon.append(getPagePosFromMousePos(mousePos + QPointF(-s, -s), pageNum));
    selectionPolygon.append(getPagePosFromMousePos(mousePos + QPointF( s, -s), pageNum));
    selectionPolygon.append(getPagePosFromMousePos(mousePos + QPointF( s,  s), pageNum));
    selectionPolygon.append(getPagePosFromMousePos(mousePos + QPointF(-s,  s), pageNum));
    currentSelection.setSelectionPolygon(selectionPolygon);
  }

  if (!currentDocument.pages[pageNum].getElements(currentSelection.selectionPolygon()).empty())
  {
    CreateSelectionCommand *createSelectionCommand = new CreateSelectionCommand(this, pageNum, currentSelection);
    undoStack.push(createSelectionCommand);

    emit updateGUI();
    update();
  }
  else
  {
    setCurrentState(state::IDLE);
  }
  m_statusText.clear();
  updateGUI();
}

void Widget::letGoSelection()
{
  if (getCurrentState() == state::SELECTED)
  {
    size_t pageNum = currentSelection.pageNum();
    ReleaseSelectionCommand *releaseCommand = new ReleaseSelectionCommand(this, pageNum);
    undoStack.push(releaseCommand);
    updateAllDirtyBuffers();
    setCurrentState(state::IDLE);
  }
}

QPointF Widget::pagePosToGrid(QPointF pagePos)
{
  QPointF pagePosOnGrid;
  pagePosOnGrid.setX(round(pagePos.x() / (gridWidth / 2.0)) * (gridWidth / 2.0));
  pagePosOnGrid.setY(round(pagePos.y() / (gridWidth / 2.0)) * (gridWidth / 2.0));
  return pagePosOnGrid;
}

void Widget::startRuling(QPointF mousePos)
{
  currentDocument.setDocumentChanged(true);
  emit modified();

  size_t pageNum = getPageFromMousePos(mousePos);
  QPointF pagePos = getPagePosFromMousePos(mousePos, pageNum);

  if (snapToGrid)
  {
    pagePos = pagePosToGrid(pagePos);
  }

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

  if (snapToGrid)
  {
    pagePos = pagePosToGrid(pagePos);
    previousPagePos = pagePosToGrid(previousPagePos);
  }

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

  QRect clipRect(m_zoom * firstPagePos.toPoint(), m_zoom * pagePos.toPoint());
  QRect oldClipRect(m_zoom * firstPagePos.toPoint(), m_zoom * previousPagePos.toPoint());
  clipRect = clipRect.normalized().united(oldClipRect.normalized());
  int clipRad = static_cast<int>(m_zoom * currentPenWidth / 2.0) + 2;
  clipRect = clipRect.normalized().adjusted(-clipRad, -clipRad, clipRad, clipRad);
  updateBufferRegion(drawingOnPage, clipRect);
  drawOnBuffer();

  /*
  QRect updateRect(firstMousePos.toPoint(), mousePos.toPoint());
  QRect oldUpdateRect(firstMousePos.toPoint(), previousMousePos.toPoint());
  updateRect = updateRect.normalized().united(oldUpdateRect.normalized());
  int rad = static_cast<int>(currentPenWidth * zoom / 2) + 2;
  updateRect = updateRect.normalized().adjusted(-rad, -rad, +rad, +rad);

  update(updateRect);
  */
  update();

  previousMousePos = mousePos;
}

void Widget::stopRuling(QPointF mousePos)
{
  QPointF pagePos = getPagePosFromMousePos(mousePos, drawingOnPage);

  if (snapToGrid)
  {
    pagePos = pagePosToGrid(pagePos);
  }

  if (currentStroke.points.length() > 1)
  {
    currentStroke.points.removeAt(1);
    currentStroke.pressures.removeAt(1);
  }

  currentStroke.points.append(pagePos);
  currentStroke.pressures.append(1);

  AddElementCommand *addCommand = new AddElementCommand(this, drawingOnPage, currentStroke.clone());
  undoStack.push(addCommand);

  currentState = state::IDLE;

  QTransform scaleTrans;
  scaleTrans = scaleTrans.scale(m_zoom, m_zoom);

  QRect clipRect = scaleTrans.mapRect(currentStroke.points.boundingRect()).toRect();
  int clipRad = static_cast<int>(m_zoom * currentPenWidth / 2) + 2;
  clipRect = clipRect.normalized().adjusted(-clipRad, -clipRad, clipRad, clipRad);
  updateBufferRegion(drawingOnPage, clipRect);
  update();
}

void Widget::startCircling(QPointF mousePos)
{
  currentDocument.setDocumentChanged(true);
  emit modified();

  size_t pageNum = getPageFromMousePos(mousePos);

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
  QPointF firstPagePos = getPagePosFromMousePos(firstMousePos, drawingOnPage);

  if (snapToGrid)
  {
    pagePos = pagePosToGrid(pagePos);
    firstPagePos = pagePosToGrid(firstPagePos);
  }

  currentDashOffset = 0.0;

  MrDoc::Stroke oldStroke = currentStroke;

  currentStroke.points.clear();
  currentStroke.pressures.clear();

  qreal radius = QLineF(firstPagePos, pagePos).length();
  qreal phi0 = QLineF(firstPagePos, pagePos).angle() * M_PI / 180.0;

  int N = 100;
  for (int i = 0; i <= N; ++i)
  {
    qreal phi = phi0 + i * (2.0 * M_PI / N);
    qreal x = firstPagePos.x() + radius * cos(phi);
    qreal y = firstPagePos.y() - radius * sin(phi);
    currentStroke.points.append(QPointF(x, y));
    currentStroke.pressures.append(1.0);
  }

  QTransform scaleTrans;
  scaleTrans = scaleTrans.scale(m_zoom, m_zoom);

  QRect clipRect = scaleTrans.mapRect(currentStroke.points.boundingRect()).toRect();
  QRect oldClipRect = scaleTrans.mapRect(oldStroke.points.boundingRect()).toRect();
  clipRect = clipRect.normalized().united(oldClipRect.normalized());
  int clipRad = static_cast<int>(m_zoom * currentPenWidth / 2) + 2;
  clipRect = clipRect.normalized().adjusted(-clipRad, -clipRad, clipRad, clipRad);
  updateBufferRegion(drawingOnPage, clipRect);

  drawOnBuffer();

  update();

  previousMousePos = mousePos;
}

void Widget::stopCircling(QPointF mousePos)
{
  continueCircling(mousePos);

  AddElementCommand *addCommand = new AddElementCommand(this, drawingOnPage, currentStroke.clone());
  undoStack.push(addCommand);

  currentState = state::IDLE;

  update();
}

void Widget::startRecting(QPointF mousePos)
{
  currentDocument.setDocumentChanged(true);
  emit modified();

  size_t pageNum = getPageFromMousePos(mousePos);

  MrDoc::Stroke newStroke;
  //    newStroke.points.append(pagePos);
  //    newStroke.pressures.append(1);
  newStroke.pattern = currentPattern;
  newStroke.penWidth = currentPenWidth;
  newStroke.color = currentColor;
  currentStroke = newStroke;
  currentState = state::RECTING;

  previousMousePos = mousePos;
  firstMousePos = mousePos;
  drawingOnPage = pageNum;
}

void Widget::continueRecting(QPointF mousePos)
{
  QPointF pagePos = getPagePosFromMousePos(mousePos, drawingOnPage);
  QPointF firstPagePos = getPagePosFromMousePos(firstMousePos, drawingOnPage);

  if (snapToGrid)
  {
    pagePos = pagePosToGrid(pagePos);
    firstPagePos = pagePosToGrid(firstPagePos);
  }

  currentDashOffset = 0.0;

  MrDoc::Stroke oldStroke = currentStroke;

  currentStroke.points.clear();
  currentStroke.pressures.clear();

  currentStroke.points.append(QPointF(firstPagePos.x(), firstPagePos.y()));
  currentStroke.points.append(QPointF(firstPagePos.x(), pagePos.y()));
  currentStroke.points.append(QPointF(pagePos.x(), pagePos.y()));
  currentStroke.points.append(QPointF(pagePos.x(), firstPagePos.y()));
  currentStroke.points.append(QPointF(firstPagePos.x(), firstPagePos.y()));

  for (int i = 0; i < 5; i++)
  {
    currentStroke.pressures.append(1.0);
  }

  QTransform scaleTrans;
  scaleTrans = scaleTrans.scale(m_zoom, m_zoom);

  QRect clipRect = scaleTrans.mapRect(currentStroke.points.boundingRect()).toRect();
  QRect oldClipRect = scaleTrans.mapRect(oldStroke.points.boundingRect()).toRect();
  clipRect = clipRect.normalized().united(oldClipRect.normalized());
  int clipRad = static_cast<int>(m_zoom * currentPenWidth / 2) + 2;
  clipRect = clipRect.normalized().adjusted(-clipRad, -clipRad, clipRad, clipRad);
  updateBufferRegion(drawingOnPage, clipRect);

  drawOnBuffer();

  update();

  previousMousePos = mousePos;
}

void Widget::stopRecting(QPointF mousePos)
{
  continueRecting(mousePos);

  AddElementCommand *addCommand = new AddElementCommand(this, drawingOnPage, currentStroke.clone());
  undoStack.push(addCommand);

  currentState = state::IDLE;

  update();
}

void Widget::startDrawing(QPointF mousePos, qreal pressure)
{
  updateTimer->start(33); // 33 -> 30 fps

  currentDocument.setDocumentChanged(true);
  emit modified();

  currentUpdateRect = QRect();

  size_t pageNum = getPageFromMousePos(mousePos);
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

  currentStroke.points.append(pagePos);
  currentStroke.pressures.append(pressure);
  drawOnBuffer(true);

  QRect updateRect(previousMousePos.toPoint(), mousePos.toPoint());
  int rad = static_cast<int>(currentPenWidth * m_zoom / 2.0) + 2;
  updateRect = updateRect.normalized().adjusted(-rad, -rad, +rad, +rad);

  currentUpdateRect = currentUpdateRect.united(updateRect);

  previousMousePos = mousePos;
}

void Widget::stopDrawing(QPointF mousePos, qreal pressure)
{
  updateTimer->stop();

  QPointF pagePos = getPagePosFromMousePos(mousePos, drawingOnPage);

  currentStroke.points.append(pagePos);
  currentStroke.pressures.append(pressure);
  drawOnBuffer();

  AddElementCommand *addCommand = new AddElementCommand(this, drawingOnPage, currentStroke.clone(), true, 0, false, true);
  undoStack.push(addCommand);

  //  currentState = state::IDLE;
  setCurrentState(state::IDLE);

  update();
}

void Widget::erase(QPointF mousePos, bool lineEraser)
{
  size_t pageNum = getPageFromMousePos(mousePos);
  QPointF pagePos = getPagePosFromMousePos(mousePos, pageNum);

  const std::vector<std::unique_ptr<MrDoc::Element>> &elements = currentDocument.pages[pageNum].elements();

  qreal eraserWidth = 10;

  //    QLineF lineA = QLineF(pagePos + QPointF(-eraserWidth,-eraserWidth) / 2.0, pagePos + QPointF( eraserWidth,  eraserWidth) / 2.0);
  //    QLineF lineB = QLineF(pagePos + QPointF( eraserWidth,-eraserWidth) / 2.0, pagePos + QPointF(-eraserWidth,  eraserWidth) / 2.0); // lineA and lineB
  //    form a cross X

  QLineF lineA = QLineF(pagePos + QPointF(-eraserWidth, -eraserWidth) / 2.0, pagePos + QPointF(-eraserWidth, eraserWidth) / 2.0);
  QLineF lineB = QLineF(pagePos + QPointF(-eraserWidth, eraserWidth) / 2.0, pagePos + QPointF(eraserWidth, eraserWidth) / 2.0);
  QLineF lineC = QLineF(pagePos + QPointF(eraserWidth, eraserWidth) / 2.0, pagePos + QPointF(eraserWidth, -eraserWidth) / 2.0);
  QLineF lineD = QLineF(pagePos + QPointF(eraserWidth, -eraserWidth) / 2.0, pagePos + QPointF(-eraserWidth, -eraserWidth) / 2.0); // lineA B C D form a square

  QRectF rectE = QRectF(pagePos + QPointF(-eraserWidth, eraserWidth) / 2.0, pagePos + QPointF(eraserWidth, -eraserWidth) / 2.0);

  std::vector<size_t> strokesToDelete;
  QPointF iPoint;

  QPointF iPointA;
  QPointF iPointB;
  QPointF iPointC;
  QPointF iPointD;


  if (!lineEraser)
  {
  bool intersected;
    for (size_t i = elements.size(); i--> 0; ) // reverse loop for size_t
    {
      auto stroke = dynamic_cast<MrDoc::Stroke*>(elements.at(i).get());
      if (nullptr != stroke)
      {
        if (rectE.intersects(stroke->points.boundingRect()) || !stroke->points.boundingRect().isValid()) // this is done for speed
        {

          for (int j = 0; j < stroke->points.length() - 1; ++j)
          {
            QLineF line = QLineF(stroke->points.at(j), stroke->points.at(j + 1));
            if (line.intersect(lineA, &iPointA) == QLineF::BoundedIntersection && iPointA != stroke->points.first() && iPointA != stroke->points.last())
            {
              iPoint = iPointA;
              intersected = true;
            }
            else if (line.intersect(lineB, &iPointB) == QLineF::BoundedIntersection && iPointB != stroke->points.first() && iPointB != stroke->points.last())
            {
              iPoint = iPointB;
              intersected = true;
            }
            else if (line.intersect(lineC, &iPointC) == QLineF::BoundedIntersection && iPointC != stroke->points.first() && iPointC != stroke->points.last())
            {
              iPoint = iPointC;
              intersected = true;
            }
            else if (line.intersect(lineD, &iPointD) == QLineF::BoundedIntersection && iPointD != stroke->points.first() && iPointD != stroke->points.last())
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
                MrDoc::Stroke splitStrokeA = *stroke;
                splitStrokeA.points = splitStrokeA.points.mid(0, j + 1);
                splitStrokeA.points.append(iPoint);
                splitStrokeA.pressures = splitStrokeA.pressures.mid(0, j + 1);
                qreal lastPressure = splitStrokeA.pressures.last();
                splitStrokeA.pressures.append(lastPressure);

                MrDoc::Stroke splitStrokeB = *stroke;
                splitStrokeB.points = splitStrokeB.points.mid(j + 1);
                splitStrokeB.points.prepend(iPoint);
                splitStrokeB.pressures = splitStrokeB.pressures.mid(j + 1);
                qreal firstPressure = splitStrokeB.pressures.first();
                splitStrokeB.pressures.prepend(firstPressure);

                RemoveElementCommand *removeElementCommand = new RemoveElementCommand(this, pageNum, i, false);
                undoStack.push(removeElementCommand);
                AddElementCommand *addElementCommand = new AddElementCommand(this, pageNum, splitStrokeB.clone(), false, i, false, false);
                undoStack.push(addElementCommand);
                addElementCommand = new AddElementCommand(this, pageNum, splitStrokeA.clone(), false, i, false, false);
                undoStack.push(addElementCommand);
                //                            strokes.insert(i, splitStrokeA);
                i += 2;
                break;
              }
            }
          }
        }
      }
    }
  }
  //    return;

  eraserWidth = eraserWidth * 0.99;
  lineA = QLineF(pagePos + QPointF(-eraserWidth, -eraserWidth) / 2.0, pagePos + QPointF(-eraserWidth, eraserWidth) / 2.0);

  lineB = QLineF(pagePos + QPointF(-eraserWidth, eraserWidth) / 2.0, pagePos + QPointF(eraserWidth, eraserWidth) / 2.0);
  lineC = QLineF(pagePos + QPointF(eraserWidth, eraserWidth) / 2.0, pagePos + QPointF(eraserWidth, -eraserWidth) / 2.0);
  lineD = QLineF(pagePos + QPointF(eraserWidth, -eraserWidth) / 2.0, pagePos + QPointF(-eraserWidth, -eraserWidth) / 2.0); // lineA B C D form a square

  rectE = QRectF(pagePos + QPointF(-eraserWidth, eraserWidth) / 2.0, pagePos + QPointF(eraserWidth, -eraserWidth) / 2.0);

  for (size_t i = 0; i < elements.size(); ++i)
  {
    auto stroke = dynamic_cast<MrDoc::Stroke*>(elements.at(i).get());
    if (nullptr != stroke)
    {
      if (rectE.intersects(stroke->points.boundingRect()) || !stroke->points.boundingRect().isValid()) // this is done for speed
      {
        bool foundStrokeToDelete = false;
        for (int j = 0; j < stroke->points.length(); ++j)
        {
          if (rectE.contains(stroke->points.at(j)))
          {
            strokesToDelete.push_back(i);
            foundStrokeToDelete = true;
            break;
          }
        }
        if (foundStrokeToDelete == false)
        {
          for (int j = 0; j < stroke->points.length() - 1; ++j)
          {
            QLineF line = QLineF(stroke->points.at(j), stroke->points.at(j + 1));
            if (line.intersect(lineA, &iPoint) == QLineF::BoundedIntersection || line.intersect(lineB, &iPoint) == QLineF::BoundedIntersection ||
                line.intersect(lineC, &iPoint) == QLineF::BoundedIntersection || line.intersect(lineD, &iPoint) == QLineF::BoundedIntersection)
            {
              strokesToDelete.push_back(i);
              break;
            }
          }
        }
      }
    }
  }

  if (strokesToDelete.size() > 0)
  {
    currentDocument.setDocumentChanged(true);
    emit modified();

    //    QRect updateRect;
    std::sort(strokesToDelete.begin(), strokesToDelete.end(), std::greater<int>());
    for (size_t i = 0; i < strokesToDelete.size(); ++i)
    {
      //      updateRect = updateRect.united(currentDocument.pages[pageNum].m_strokes.at(strokesToDelete.at(i)).points.boundingRect().toRect());
      RemoveElementCommand *removeCommand = new RemoveElementCommand(this, pageNum, strokesToDelete[i]);
      undoStack.push(removeCommand);
    }
  }
  updateAllDirtyBuffers();
}

void Widget::startMovingSelection(QPointF mousePos)
{
  currentDocument.setDocumentChanged(true);
  emit modified();

  size_t pageNum = getPageFromMousePos(mousePos);
  previousPagePos = getPagePosFromMousePos(mousePos, pageNum);
  if (snapToGrid)
  {
    previousPagePos = pagePosToGrid(previousPagePos);
  }

  setCurrentState(state::MOVING_SELECTION);
}

void Widget::continueMovingSelection(QPointF mousePos)
{
  size_t pageNum = getPageFromMousePos(mousePos);
  QPointF pagePos = getPagePosFromMousePos(mousePos, pageNum);
  //    currentSelection.move(1 * (pagePos - previousPagePos));

  if (snapToGrid)
  {
    pagePos = pagePosToGrid(pagePos);
  }

  QPointF delta = (pagePos - previousPagePos);

  QTransform transform;
  transform.translate(delta.x(), delta.y());

  //    currentSelection.transform(transform, pageNum);
  TransformSelectionCommand *transSelectCommand = new TransformSelectionCommand(this, pageNum, transform);
  undoStack.push(transSelectCommand);

  previousPagePos = pagePos;
  //    update(currentSelection.selectionPolygon.boundingRect().toRect());
}

void Widget::startRotatingSelection(QPointF mousePos)
{
  currentDocument.setDocumentChanged(true);
  emit modified();

  m_currentAngle = 0.0;

  size_t pageNum = currentSelection.pageNum();
  previousPagePos = getPagePosFromMousePos(mousePos, pageNum);
  setCurrentState(state::ROTATING_SELECTION);
}

void Widget::continueRotatingSelection(QPointF mousePos)
{
  size_t pageNum = currentSelection.pageNum();
  QPointF pagePos = getPagePosFromMousePos(mousePos, pageNum);

  m_currentAngle = QLineF(currentSelection.boundingRect().center(), pagePos).angleTo(QLineF(currentSelection.boundingRect().center(), previousPagePos));
  if (snapToGrid)
  {
    m_currentAngle = floor(m_currentAngle / 5.0) * 5.0;
  }
  currentSelection.setAngle(m_currentAngle);
  m_statusText = QString("%1°").arg(m_currentAngle > 180.0 ? 360.0 - m_currentAngle : - m_currentAngle);
  updateGUI();
}

void Widget::stopRotatingSelection(QPointF mousePos)
{
  continueRotatingSelection(mousePos);
  size_t pageNum = currentSelection.pageNum();

  QTransform transform;
  transform.translate(currentSelection.boundingRect().center().x(), currentSelection.boundingRect().center().y());
  transform.rotate(m_currentAngle);
  transform.translate(-currentSelection.boundingRect().center().x(), -currentSelection.boundingRect().center().y());

  currentSelection.setAngle(0.0);

  TransformSelectionCommand *transSelectCommand = new TransformSelectionCommand(this, pageNum, transform);
  undoStack.push(transSelectCommand);

  currentSelection.finalize();
  currentSelection.updateBuffer(m_zoom);
  setCurrentState(state::SELECTED);
  m_statusText = "";
  updateGUI();
}

void Widget::startResizingSelection(QPointF mousePos, MrDoc::Selection::GrabZone grabZone)
{
  currentDocument.setDocumentChanged(true);
  emit modified();

  m_grabZone = grabZone;

  size_t pageNum = currentSelection.pageNum();
  previousPagePos = getPagePosFromMousePos(mousePos, pageNum);
  setCurrentState(state::RESIZING_SELECTION);
}

void Widget::continueResizingSelection(QPointF mousePos)
{
  size_t pageNum = currentSelection.pageNum();
  QPointF pagePos = getPagePosFromMousePos(mousePos, pageNum);
  QPointF delta = (pagePos - previousPagePos);

  QTransform transform;

  qreal moveX = currentSelection.boundingRect().x();
  qreal moveY = currentSelection.boundingRect().y();

  qreal sx = 0.0;
  qreal sy = 0.0;

  qreal moveBackX = 0.0;
  qreal moveBackY = 0.0;

  using GrabZone = MrDoc::Selection::GrabZone;
  if (m_grabZone == GrabZone::Top)
  {
    sx = 1.0;
    sy = (currentSelection.boundingRect().height() - delta.y()) / currentSelection.boundingRect().height();
    moveBackX = -moveX;
    moveBackY = -moveY + delta.y() / sy;
  }
  else if (m_grabZone == GrabZone::Bottom)
  {
    sx = 1.0;
    sy = (currentSelection.boundingRect().height() + delta.y()) / currentSelection.boundingRect().height();
    moveBackX = -moveX;
    moveBackY = -moveY;
  }
  else if (m_grabZone == GrabZone::Left)
  {
    sx = (currentSelection.boundingRect().width() - delta.x()) / currentSelection.boundingRect().width();
    sy = 1.0;
    moveBackX = -moveX + delta.x() / sx;
    moveBackY = -moveY;
  }
  else if (m_grabZone == GrabZone::Right)
  {
    sx = (currentSelection.boundingRect().width() + delta.x()) / currentSelection.boundingRect().width();
    sy = 1.0;
    moveBackX = -moveX;
    moveBackY = -moveY;
  }
  else if (m_grabZone == GrabZone::TopLeft)
  {
    sx = (currentSelection.boundingRect().width() - delta.x()) / currentSelection.boundingRect().width();
    sy = (currentSelection.boundingRect().height() - delta.y()) / currentSelection.boundingRect().height();
    moveBackX = -moveX + delta.x() / sx;
    moveBackY = -moveY + delta.y() / sy;
  }
  else if (m_grabZone == GrabZone::TopRight)
  {
    sx = (currentSelection.boundingRect().width() + delta.x()) / currentSelection.boundingRect().width();
    sy = (currentSelection.boundingRect().height() - delta.y()) / currentSelection.boundingRect().height();
    moveBackX = -moveX;
    moveBackY = -moveY + delta.y() / sy;
  }
  else if (m_grabZone == GrabZone::BottomLeft)
  {
    sx = (currentSelection.boundingRect().width() - delta.x()) / currentSelection.boundingRect().width();
    sy = (currentSelection.boundingRect().height() + delta.y()) / currentSelection.boundingRect().height();
    moveBackX = -moveX + delta.x() / sx;
    moveBackY = -moveY;
  }
  else if (m_grabZone == GrabZone::BottomRight)
  {
    sx = (currentSelection.boundingRect().width() + delta.x()) / currentSelection.boundingRect().width();
    sy = (currentSelection.boundingRect().height() + delta.y()) / currentSelection.boundingRect().height();
    moveBackX = -moveX;
    moveBackY = -moveY;
  }

  if (sx <= 0.01)
  {
    sx = 1.0;
    moveX = 0.0;
    moveBackX = 0.0;
  }

  if (sy <= 0.01)
  {
    sy = 1.0;
    moveY = 0.0;
    moveBackY = 0.0;
  }

  transform.translate(moveX, moveY);
  transform.scale(sx, sy);
  transform.translate(moveBackX, moveBackY);

  //  m_currentTransform = transform * m_currentTransform;

  TransformSelectionCommand *transSelectCommand = new TransformSelectionCommand(this, pageNum, transform);
  undoStack.push(transSelectCommand);

  previousPagePos = pagePos;
}

void Widget::stopResizingSelection(QPointF mousePos)
{
  continueResizingSelection(mousePos);

  currentSelection.finalize();
  currentSelection.updateBuffer(m_zoom);
  setCurrentState(state::SELECTED);
}

size_t Widget::getPageFromMousePos(QPointF mousePos) const
{
  qreal y = mousePos.y(); // - currentCOSPos.y();
  size_t pageNum = 0;
  // while (y > (floor(currentDocument.pages[pageNum].height() * m_zoom)) + PAGE_GAP)
  while (y > (floor(currentDocument.pages[pageNum].pixelHeight(m_zoom, devicePixelRatio()))) + PAGE_GAP)
  {
    // y -= (floor(currentDocument.pages[pageNum].height() * m_zoom)) + PAGE_GAP;
    y -= (floor(currentDocument.pages[pageNum].pixelHeight(m_zoom, devicePixelRatio()))) + PAGE_GAP;
    pageNum += 1;
    if (pageNum >= currentDocument.pages.size())
    {
      pageNum = currentDocument.pages.size() - 1;
      break;
    }
  }
  return pageNum;
}

size_t Widget::getCurrentPage() const
{
  QPoint globalMousePos = parentWidget()->mapToGlobal(QPoint(0, 0)) + QPoint(parentWidget()->size().width() / 2, parentWidget()->size().height() / 2);
  QPoint pos = this->mapFromGlobal(globalMousePos);
  size_t pageNum = this->getPageFromMousePos(pos);

  return pageNum;
  //    return getPageFromMousePos(QPointF(0.0, 2.0));
}

QPointF Widget::getPagePosFromMousePos(QPointF mousePos, size_t pageNum) const
{
  qreal x = mousePos.x();
  qreal y = mousePos.y();
  for (size_t i = 0; i < pageNum; ++i)
  {
    //        y -= (currentDocument.pages[i].height() * zoom + PAGE_GAP); // THIS DOESN'T WORK PROPERLY (should be floor(...height(), or just use
    //        pageBuffer[i].height()/devicePixelRatio())
    // y -= (pageBuffer[i].height()/devicePixelRatio() + PAGE_GAP);
    y -= (currentDocument.pages[pageNum].pixelHeight(m_zoom, devicePixelRatio()) + PAGE_GAP);
  }
  //    y -= (pageNum) * (currentDocument.pages[0].height() * zoom + PAGE_GAP);

  QPointF pagePos = (QPointF(x, y)) / m_zoom;

  return pagePos;
}

QPointF Widget::getAbsolutePagePosFromMousePos(QPointF mousePos) const
{
  size_t pageNum = getPageFromMousePos(mousePos);
  QPointF pagePos = getPagePosFromMousePos(mousePos, pageNum);

  qreal y = 0.0;
  for (size_t i = 0; i < pageNum; ++i)
  {
    //y += (pageBuffer[i].height()/devicePixelRatio() + PAGE_GAP);
    y += (currentDocument.pages[pageNum].pixelHeight(m_zoom, devicePixelRatio()) + PAGE_GAP);
  }
  y *= m_zoom;

  pagePos.setY(y + pagePos.y());

  return pagePos;
}

void Widget::newFile()
{
  letGoSelection();

  currentDocument = MrDoc::Document();
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
  qreal newZoom = m_zoom * ZOOM_STEP;

  zoomTo(newZoom);
}

void Widget::zoomOut()
{
  qreal newZoom = m_zoom / ZOOM_STEP;

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
  if (newZoom == m_zoom)
  {
    return;
  }

  m_zoom = newZoom;

  int prevHMax = scrollArea->horizontalScrollBar()->maximum();
  int prevVMax = scrollArea->verticalScrollBar()->maximum();

  int prevH = scrollArea->horizontalScrollBar()->value();
  int prevV = scrollArea->verticalScrollBar()->value();

  updateAllPageBuffers();
  currentSelection.updateBuffer(m_zoom);
  setGeometry(getWidgetGeometry());

  int newHMax = scrollArea->horizontalScrollBar()->maximum();
  int newVMax = scrollArea->verticalScrollBar()->maximum();

  int newH, newV;

  if (prevHMax != 0)
  {
    newH = static_cast<int>(double(prevH) / prevHMax * newHMax);
  }
  else
  {
    newH = newHMax / 2;
  }
  if (prevVMax != 0)
  {
    newV = static_cast<int>(double(prevV) / prevVMax * newVMax);
  }
  else
  {
    newV = newVMax / 2;
  }

  scrollArea->horizontalScrollBar()->setValue(newH);
  scrollArea->verticalScrollBar()->setValue(newV);

  updateAllPageBuffers();
  update();
}

void Widget::zoomFitWidth()
{
  size_t pageNum = getCurrentPage();

  QSize widgetSize = this->parentWidget()->size();
  qreal newZoom = widgetSize.width() / currentDocument.pages[pageNum].width();

  zoomTo(newZoom);
}

void Widget::zoomFitHeight()
{
  size_t pageNum = getCurrentPage();

  QSize widgetSize = this->parentWidget()->size();
  qreal newZoom = widgetSize.height() / currentDocument.pages[pageNum].height();

  zoomTo(newZoom);
}

void Widget::toggleGrid()
{
  showGrid = !showGrid;
  updateAllPageBuffers(true);
  update();
}

void Widget::toggleSnapToGrid()
{
  snapToGrid = !snapToGrid;
  update();
}

bool Widget::showingGrid()
{
  return showGrid;
}

bool Widget::snappingToGrid()
{
  return snapToGrid;
}

void Widget::pageFirst()
{
  scrollDocumentToPageNum(0);
}

void Widget::pageLast()
{
  scrollDocumentToPageNum(currentDocument.pages.size() - 1);
}

void Widget::pageUp()
{
  size_t pageNum = getCurrentPage();
  if (pageNum > 0) pageNum--;
  scrollDocumentToPageNum(pageNum);
}

void Widget::pageDown()
{
  size_t pageNum = getCurrentPage();
  pageNum++;

  if (pageNum >= currentDocument.pages.size())
  {
    pageAddEnd();
  }

  scrollDocumentToPageNum(pageNum);
}

void Widget::pageAddBefore()
{
  size_t pageNum = getCurrentPage();
  AddPageCommand *addPageCommand = new AddPageCommand(this, pageNum);
  undoStack.push(addPageCommand);
  setGeometry(getWidgetGeometry());
  update();
  currentDocument.setDocumentChanged(true);
  emit modified();
}

void Widget::pageAddAfter()
{
  size_t pageNum = getCurrentPage() + 1;
  AddPageCommand *addPageCommand = new AddPageCommand(this, pageNum);
  undoStack.push(addPageCommand);
  setGeometry(getWidgetGeometry());
  update();
  currentDocument.setDocumentChanged(true);

  emit modified();
}

void Widget::pageAddBeginning()
{
  AddPageCommand *addPageCommand = new AddPageCommand(this, 0);
  undoStack.push(addPageCommand);
  setGeometry(getWidgetGeometry());
  update();
  currentDocument.setDocumentChanged(true);

  emit modified();
}

void Widget::pageAddEnd()
{
  AddPageCommand *addPageCommand = new AddPageCommand(this, currentDocument.pages.size());
  undoStack.push(addPageCommand);
  setGeometry(getWidgetGeometry());
  update();
  currentDocument.setDocumentChanged(true);

  emit modified();
}

void Widget::pageRemove()
{
  if (currentDocument.pages.size() > 1)
  {
    size_t pageNum = getCurrentPage();
    RemovePageCommand *removePageCommand = new RemovePageCommand(this, pageNum);
    undoStack.push(removePageCommand);
    setGeometry(getWidgetGeometry());
    update();
    currentDocument.setDocumentChanged(true);
    emit modified();
  }
}

void Widget::scrollDocumentToPageNum(size_t pageNum)
{
  if (pageNum >= currentDocument.pages.size())
  {
    return; // page doesn't exist
  }
  qreal y = 0.0;
  for (size_t i = 0; i < pageNum; ++i)
  {
    y += (currentDocument.pages[i].height()) * m_zoom + PAGE_GAP;
  }

  scrollArea->verticalScrollBar()->setValue(static_cast<int>(y));
}

void Widget::setCurrentTool(tool toolID)
{
  if (currentState == state::IDLE || currentState == state::SELECTED || currentState == state::SELECTING)
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
    if (toolID == tool::RECT)
      setCursor(penCursor); // todo rect cursor
    if (toolID == tool::ERASER)
      setCursor(eraserCursor);
    if (toolID == tool::STROKE_ERASER)
      setCursor(strokeEraserCursor);
    if (toolID == tool::SELECT)
      setCursor(Qt::CrossCursor);
    if (toolID == tool::HAND)
      setCursor(Qt::OpenHandCursor);
  }
}

void Widget::setPenCursor(const QString& resourceName)
{
  if (currentState == state::IDLE || currentState == state::SELECTED)
  {
    // setup cursor
    QPixmap penCursorBitmap = QPixmap(resourceName);
    penCursorBitmap.setMask(penCursorBitmap.mask());
    penCursor = QCursor(penCursorBitmap, -1, -1);
    setCursor(penCursor);
  }
}

void Widget::setDocument(const MrDoc::Document &newDocument)
{
  currentDocument = newDocument;
  undoStack.clear();
  pageBuffer.clear();
  m_zoom = 0.0; // otherwise zoomTo() doesn't do anything if zoom == newZoom
  zoomFitWidth();
  pageFirst();
}

void Widget::selectAll()
{

  if (currentState == state::SELECTED)
  {
    letGoSelection();
  }

  size_t pageNum = getCurrentPage();
  QRectF selectRect;
  for (auto &element : currentDocument.pages[pageNum].elements())
  {
    selectRect = selectRect.united(element->boundingRect());
  }
  QPolygonF selectionPolygon = QPolygonF(selectRect);

  MrDoc::Selection selection;
  selection.setPageNum(pageNum);
  selection.setSelectionPolygon(selectionPolygon);

  if (!currentDocument.pages[pageNum].getElements(selection.selectionPolygon()).empty())
  {
    currentSelection = selection;
    CreateSelectionCommand *createSelectionCommand = new CreateSelectionCommand(this, pageNum, selection);
    undoStack.push(createSelectionCommand);

    emit updateGUI();
    update();
  }
  else
  {
    setCurrentState(state::IDLE);
  }
}

void Widget::copy()
{
  if (currentState == state::SELECTED )
  {
    clipboard = currentSelection;

    QImage selectionBuffer = currentSelection.buffer();
    QImage clipboardImage = selectionBuffer;
    clipboardImage.fill(QColor(255,255,255));
    QPainter painter(&clipboardImage);
    painter.drawImage(0, 0, selectionBuffer);

    QClipboard *clipboard = QGuiApplication::clipboard();
    clipboard->setImage(clipboardImage);
  }
  update();
}

void Widget::paste()
{
  if (! clipboard.empty())
  {
    MrDoc::Selection tmpSelection = clipboard;
    tmpSelection.setPageNum(getCurrentPage());

    QPoint globalMousePos = parentWidget()->mapToGlobal(QPoint(0, 0)) + QPoint(parentWidget()->size().width() / 2, parentWidget()->size().height() / 2);
    QPoint mousePos = this->mapFromGlobal(globalMousePos);
    QPointF selectionPos = getPagePosFromMousePos(mousePos, getCurrentPage()) - tmpSelection.boundingRect().center();

    if (snapToGrid)
    {
      selectionPos = pagePosToGrid(selectionPos);
    }

    QTransform myTrans;
    myTrans = myTrans.translate(selectionPos.x(), selectionPos.y());

    tmpSelection.transform(myTrans, tmpSelection.pageNum());

    tmpSelection.finalize();
    tmpSelection.updateBuffer(m_zoom);

    undoStack.beginMacro("Paste");
    if (currentState == state::SELECTED)
    {
      letGoSelection();
    }
    PasteCommand *pasteCommand = new PasteCommand(this, tmpSelection);
    undoStack.push(pasteCommand);
    undoStack.endMacro();

    currentDocument.setDocumentChanged(true);
    emit modified();
  }
}

void Widget::cut()
{
  CutCommand *cutCommand = new CutCommand(this);
  undoStack.push(cutCommand);
}

void Widget::deleteSlot()
{
  DeleteCommand *deleteCommand = new DeleteCommand(this);
  undoStack.push(deleteCommand);
}

void Widget::undo()
{
  if (undoStack.canUndo() && (currentState == state::IDLE || currentState == state::SELECTED))
  {
    undoStack.undo();
    currentSelection.updateBuffer(m_zoom);
    updateAllDirtyBuffers();
  }
}

void Widget::redo()
{
  if (undoStack.canRedo() && (currentState == state::IDLE || currentState == state::SELECTED))
  {
    undoStack.redo();
    currentSelection.updateBuffer(m_zoom);
    updateAllDirtyBuffers();
  }
}

void Widget::setCurrentState(state newState)
{
  currentState = newState;
  if (currentState == state::IDLE || currentState == state::SELECTED)
  {
    updateAllDirtyBuffers();
    updateDirtyTimer->stop();
  }
  else
  {
    updateDirtyTimer->start();
  }
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
    currentSelection.updateBuffer(m_zoom);
    update();
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

void Widget::setPencilCursorIcon()
{
  currentPenCursor = Widget::cursor::PENCIL;
  setPenCursor(":/images/penCursor3.png");
  emit updateGUI();
}

void Widget::setDotCursorIcon()
{
  currentPenCursor = Widget::cursor::DOT;
  setPenCursor(":/images/dotCursor.png");
  emit updateGUI();
}

void Widget::rotateSelection(qreal angle)
{
  QTransform rotateTrans;

  qreal dx = currentSelection.boundingRect().center().x();
  qreal dy = currentSelection.boundingRect().center().y();

  rotateTrans = rotateTrans.translate(dx, dy).rotate(-angle).translate(-dx, -dy);
  TransformSelectionCommand *transCommand = new TransformSelectionCommand(this, currentSelection.pageNum(), rotateTrans);
  undoStack.push(transCommand);
  currentSelection.finalize();
  currentSelection.updateBuffer(m_zoom);
  update();
}

void Widget::setCurrentPattern(QVector<qreal> newPattern)
{
  currentPattern = newPattern;
  emit updateGUI();
  if (currentState == state::SELECTED)
  {
    ChangePatternOfSelectionCommand *changePatternCommand = new ChangePatternOfSelectionCommand(this, newPattern);
    undoStack.push(changePatternCommand);
    currentSelection.updateBuffer(m_zoom);
    update();
  }
}

QVector<qreal> Widget::getCurrentPattern()
{
  return currentPattern;
}

void Widget::solidPattern()
{
  setCurrentPattern(MrDoc::solidLinePattern);
}

void Widget::dashPattern()
{
  setCurrentPattern(MrDoc::dashLinePattern);
}

void Widget::dashDotPattern()
{
  setCurrentPattern(MrDoc::dashDotLinePattern);
}

void Widget::dotPattern()
{
  setCurrentPattern(MrDoc::dotLinePattern);
}

void Widget::setCurrentPenWidth(qreal penWidth)
{
  currentPenWidth = penWidth;
  if (currentState == state::SELECTED)
  {
    ChangePenWidthOfSelectionCommand *changePenWidthCommand = new ChangePenWidthOfSelectionCommand(this, penWidth);
    undoStack.push(changePenWidthCommand);
    currentSelection.updateBuffer(m_zoom);
    update();
  }
}

void Widget::setCurrentPenCursor(Widget::cursor cursorType)
{
    switch (cursorType) {
    case Widget::cursor::PENCIL:
        setPencilCursorIcon();
        break;
    case Widget::cursor::DOT:
        setDotCursorIcon();
        break;
    }
}
