#include "widget.h"
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
// Widget::Widget(QWidget *parent) : QOpenGLWidget(parent)
{
  QSettings settings;
  //    qDebug() << settings.applicationVersion();

  m_currentState = state::IDLE;

  // setup cursors
  QPixmap penCursorBitmap = QPixmap(":/images/penCursor3.png");
  QPixmap penCursorMask = QPixmap(":/images/penCursor3Mask.png");
  penCursorBitmap.setMask(QBitmap(penCursorMask));
  m_penCursor = QCursor(penCursorBitmap, -1, -1);

  QPixmap circleCursorBitmap = QPixmap(":/images/circleCursor.png");
  QPixmap circleCursorMask = QPixmap(":/images/circleCursorMask.png");
  circleCursorBitmap.setMask(QBitmap(circleCursorMask));
  m_circleCursor = QCursor(circleCursorBitmap, -1, -1);

  QPixmap rulerCursorBitmap = QPixmap(":/images/rulerCursor.png");
  QPixmap rulerCursorMask = QPixmap(":/images/rulerCursorMask.png");
  rulerCursorBitmap.setMask(QBitmap(rulerCursorMask));
  m_rulerCursor = QCursor(rulerCursorBitmap, -1, -1);

  QPixmap eraserCursorBitmap = QPixmap(":/images/eraserCursor.png");
  QPixmap eraserCursorMask = QPixmap(":/images/eraserCursorMask.png");
  eraserCursorBitmap.setMask(QBitmap(eraserCursorMask));
  m_eraserCursor = QCursor(eraserCursorBitmap, -1, -1);

  m_currentTool = tool::PEN;
  m_previousTool = tool::NONE;
  m_realEraser = false;
  setCursor(penCursorBitmap);

  m_currentDocument = MrDoc::Document();

  m_currentPenWidth = 1.41;
  m_currentColor = QColor(0, 0, 0);
  m_zoom = 1.0;

  m_currentCOSPos.setX(0.0);
  m_currentCOSPos.setY(0.0);
  updateAllPageBuffers();
  setGeometry(getWidgetGeometry());

  parent->updateGeometry();
  parent->update();

  m_updateTimer = new QTimer(this);
  connect(m_updateTimer, SIGNAL(timeout()), this, SLOT(updateWhileDrawing()));

  m_updateDirtyTimer = new QTimer(this);
  connect(m_updateDirtyTimer, SIGNAL(timeout()), this, SLOT(updateAllDirtyBuffers()));
  m_updateDirtyTimer->setInterval(15);
}

void Widget::updateAllPageBuffers()
{
  QVector<QFuture<void>> future;
  m_pageImageBuffer.clear();
  for (int buffNum = 0; buffNum < m_currentDocument.pages.size(); ++buffNum)
  {
    m_pageImageBuffer.append(QImage());
  }

  for (int buffNum = 0; buffNum < m_currentDocument.pages.size(); ++buffNum)
  {
    future.append(QtConcurrent::run(this, &Widget::updateImageBuffer, buffNum));
  }
  m_pageBuffer.clear();
  for (int buffNum = 0; buffNum < m_currentDocument.pages.size(); ++buffNum)
  {
    future[buffNum].waitForFinished();
    m_pageBuffer.append(QPixmap::fromImage(m_pageImageBuffer.at(buffNum)));

    // safe some memory
    m_pageImageBufferMutex.lock();
    m_pageImageBuffer[buffNum] = QImage();
    m_pageImageBufferMutex.unlock();
  }
  m_pageImageBuffer.clear();
}

void Widget::updateImageBuffer(int buffNum)
{
  MrDoc::Page const &page = m_currentDocument.pages.at(buffNum);
  int pixelWidth = m_zoom * page.width() * devicePixelRatio();
  int pixelHeight = m_zoom * page.height() * devicePixelRatio();
  QImage image(pixelWidth, pixelHeight, QImage::Format_ARGB32_Premultiplied);
  image.setDevicePixelRatio(devicePixelRatio());

  image.fill(page.backgroundColor());

  QPainter painter;
  painter.begin(&image);
  painter.setRenderHint(QPainter::Antialiasing, true);

  m_currentDocument.pages[buffNum].paint(painter, m_zoom);

  painter.end();

  m_pageImageBufferMutex.lock();
  m_pageImageBuffer.replace(buffNum, image);
  m_pageImageBufferMutex.unlock();
}

void Widget::updateBuffer(int buffNum)
{
  MrDoc::Page const &page = m_currentDocument.pages.at(buffNum);
  int pixelWidth = m_zoom * page.width() * devicePixelRatio();
  int pixelHeight = m_zoom * page.height() * devicePixelRatio();
  QPixmap pixmap(pixelWidth, pixelHeight);
  pixmap.setDevicePixelRatio(devicePixelRatio());

  pixmap.fill(page.backgroundColor());

  QPainter painter;
  painter.begin(&pixmap);
  painter.setRenderHint(QPainter::Antialiasing, true);

  m_currentDocument.pages[buffNum].paint(painter, m_zoom);

  painter.end();

  m_pageBuffer.replace(buffNum, pixmap);
}

void Widget::updateBufferRegion(int buffNum, QRectF const &clipRect)
{
  QPainter painter;
  painter.begin(&m_pageBuffer[buffNum]);
  painter.setRenderHint(QPainter::Antialiasing, true);
  painter.setClipRect(clipRect);
  painter.setClipping(true);

  painter.fillRect(clipRect, m_currentDocument.pages.at(buffNum).backgroundColor());
  //    painter.fillRect(clipRect, Qt::red);

  QRectF paintRect = QRectF(clipRect.topLeft() / m_zoom, clipRect.bottomRight() / m_zoom);
  m_currentDocument.pages[buffNum].paint(painter, m_zoom, paintRect);

  painter.end();
}

void Widget::updateAllDirtyBuffers()
{
  for (int buffNum = 0; buffNum < m_currentDocument.pages.size(); ++buffNum)
  {
    QRectF const &dirtyRect = m_currentDocument.pages.at(buffNum).dirtyRect();
    if (!dirtyRect.isNull())
    {
      QRectF dirtyBufferRect = QRectF(dirtyRect.topLeft() * m_zoom, dirtyRect.bottomRight() * m_zoom);
      updateBufferRegion(buffNum, dirtyBufferRect);
      m_currentDocument.pages[buffNum].clearDirtyRect();
    }
  }
  update();
}

void Widget::drawOnBuffer(bool last)
{
  QPainter painter;
  painter.begin(&m_pageBuffer[m_drawingOnPage]);
  painter.setRenderHint(QPainter::Antialiasing, true);

  m_currentStroke.paint(painter, m_zoom, last);
}

QRect Widget::getWidgetGeometry()
{
  int width = 0;
  int height = 0;
  for (int i = 0; i < m_pageBuffer.size(); ++i)
  {
    height += m_pageBuffer[i].height()/devicePixelRatio() + PAGE_GAP;
    if (m_pageBuffer[i].width()/devicePixelRatio() > width)
      width = m_pageBuffer[i].width()/devicePixelRatio();
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

  if (m_currentState == state::DRAWING)
  {
    QRectF rectSource;
    QTransform trans;
    for (int i = 0; i < m_drawingOnPage; ++i)
    {
      trans = trans.translate(0, -(m_pageBuffer.at(i).height() + PAGE_GAP*devicePixelRatio()));
    }
    trans = trans.scale(devicePixelRatio(),devicePixelRatio());
    rectSource = trans.mapRect(event->rect());

    //        QPixmap tmp = QPixmap::fromImage(pageBuffer.at(drawingOnPage));
    painter.drawPixmap(event->rect(), m_pageBuffer[m_drawingOnPage], rectSource);

    //        painter.drawImage(event->rect(), pageBuffer.at(drawingOnPage), rectSource);
    return;
  }

  //    painter.setRenderHint(QPainter::Antialiasing, true);

  for (int i = 0; i < m_pageBuffer.size(); ++i)
  {
    QRectF rectSource;
    rectSource.setTopLeft(QPointF(0.0, 0.0));
    rectSource.setWidth(m_pageBuffer.at(i).width());
    rectSource.setHeight(m_pageBuffer.at(i).height());

    QRectF rectTarget;
    //        rectTarget.setTopLeft(QPointF(0.0, currentYPos));
    rectTarget.setTopLeft(QPointF(0.0, 0.0));
    rectTarget.setWidth(m_pageBuffer.at(i).width()/devicePixelRatio());
    rectTarget.setHeight(m_pageBuffer.at(i).height()/devicePixelRatio());

    painter.drawPixmap(rectTarget, m_pageBuffer.at(i), rectSource);

    if ((m_currentState == state::SELECTING || m_currentState == state::SELECTED || m_currentState == state::MOVING_SELECTION ||
         m_currentState == state::RESIZING_SELECTION || m_currentState == state::ROTATING_SELECTION) &&
        i == currentSelection.pageNum())
    {
      currentSelection.paint(painter, m_zoom);
    }

    painter.translate(QPointF(0.0, rectSource.height()/devicePixelRatio() + PAGE_GAP));
  }
}

void Widget::updateWhileDrawing()
{
  update(m_currentUpdateRect);
  m_currentUpdateRect.setWidth(0);
  m_currentUpdateRect.setHeight(0);
}

void Widget::mouseAndTabletEvent(QPointF mousePos, Qt::MouseButton button, Qt::MouseButtons buttons, Qt::KeyboardModifiers keyboardModifiers,
                                 QTabletEvent::PointerType pointerType, QEvent::Type eventType, qreal pressure, bool tabletEvent)
{
  // Under Linux the keyboard modifiers are not reported to tabletevent. this should work
  // everywhere.
  keyboardModifiers = qApp->queryKeyboardModifiers();

  // bencmark
  if (eventType == QEvent::MouseButtonPress)
  {
    m_timer.start();
    m_count = 1;
  }
  if (eventType == QEvent::MouseMove)
  {
    ++m_count;
  }
  if (eventType == QEvent::MouseButtonRelease)
  {
    ++m_count;
    //    qInfo() << static_cast<qreal>(count) / static_cast<qreal>(timer.elapsed()) * 1000.0;
  }
  // end benchmark

  int pageNum = getPageFromMousePos(mousePos);
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
    pressure = m_minWidthMultiplier + pressure * (m_maxWidthMultiplier - m_minWidthMultiplier);
  }

  if (eventType == QEvent::MouseButtonRelease)
  {
    m_undoStack.endMacro();
    setPreviousTool();
  }

  if ((m_currentState == state::IDLE || m_currentState == state::SELECTED) && buttons & Qt::MiddleButton && pointerType == QTabletEvent::Pen)
  {
    if (eventType == QEvent::MouseButtonPress && button == Qt::MiddleButton)
    {
      if (m_currentTool != tool::HAND)
      {
        m_previousTool = m_currentTool;
      }
      m_previousMousePos = mousePos;
      emit hand();
      return;
    }
    if (eventType == QEvent::MouseMove)
    {
      int dx = 1 * (mousePos.x() - m_previousMousePos.x());
      int dy = 1 * (mousePos.y() - m_previousMousePos.y());

      m_scrollArea->horizontalScrollBar()->setValue(m_scrollArea->horizontalScrollBar()->value() - dx);
      m_scrollArea->verticalScrollBar()->setValue(m_scrollArea->verticalScrollBar()->value() - dy);

      qInfo() << dy;

      mousePos -= QPointF(dx, dy);

      m_previousMousePos = mousePos;
      return;
    }
    return;
  }

  if (m_currentState == state::SELECTED)
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

  if (m_currentState == state::IDLE && button == Qt::RightButton)
  {
    m_previousTool = m_currentTool;
    startSelecting(mousePos);
    emit select();
    return;
  }

  if (m_currentState == state::MOVING_SELECTION)
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

  if (m_currentState == state::ROTATING_SELECTION)
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

  if (m_currentState == state::RESIZING_SELECTION)
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

  if (m_currentState == state::SELECTING)
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

  if (m_currentState == state::DRAWING)
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

  if (m_currentState == state::RULING)
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

  if (m_currentState == state::CIRCLING)
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

  if (m_currentState == state::IDLE)
  {
    if (eventType == QEvent::MouseButtonPress)
    {
      if (pointerType == QTabletEvent::Pen)
      {
        if (m_currentTool == tool::PEN)
        {
          startDrawing(mousePos, pressure);
          return;
        }
        if (m_currentTool == tool::RULER)
        {
          startRuling(mousePos);
          return;
        }
        if (m_currentTool == tool::CIRCLE)
        {
          startCircling(mousePos);
          return;
        }
        if (m_currentTool == tool::ERASER)
        {
          m_undoStack.beginMacro("erase");
          erase(mousePos, invertEraser);
          return;
        }
        if (m_currentTool == tool::SELECT)
        {
          startSelecting(mousePos);
          return;
        }
        if (m_currentTool == tool::HAND)
        {
          m_previousMousePos = mousePos;
        }
      }
      if (pointerType == QTabletEvent::Eraser)
      {
        m_previousTool = m_currentTool;
        emit eraser();
        m_undoStack.beginMacro("erase");
        erase(mousePos, invertEraser);
      }
    }
    if (eventType == QEvent::MouseMove)
    {
      if (pointerType == QTabletEvent::Eraser || m_currentTool == tool::ERASER)
      {
        erase(mousePos, invertEraser);
      }
      if (m_currentTool == tool::HAND)
      {
        int dx = 1 * (mousePos.x() - m_previousMousePos.x());
        int dy = 1 * (mousePos.y() - m_previousMousePos.y());

        m_scrollArea->horizontalScrollBar()->setValue(m_scrollArea->horizontalScrollBar()->value() - dx);
        m_scrollArea->verticalScrollBar()->setValue(m_scrollArea->verticalScrollBar()->value() - dy);

        mousePos -= QPointF(dx, dy);

        m_previousMousePos = mousePos;
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
  if (event->type() == QTabletEvent::TabletPress)
  {
    eventType = QEvent::MouseButtonPress;
    m_penDown = true;
  }
  if (event->type() == QTabletEvent::TabletMove)
  {
    eventType = QEvent::MouseMove;
    m_penDown = true;
  }
  if (event->type() == QTabletEvent::TabletRelease)
  {
    eventType = QEvent::MouseButtonRelease;
  }

  Qt::KeyboardModifiers keyboardModifiers = event->modifiers();

  mouseAndTabletEvent(mousePos, event->button(), event->buttons(), keyboardModifiers, event->pointerType(), eventType, pressure, true);
}

void Widget::mousePressEvent(QMouseEvent *event)
{
  bool usingTablet = static_cast<TabletApplication *>(qApp)->isUsingTablet();

  if (!usingTablet)
  {
    if (!m_penDown)
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
    if (!m_penDown)
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
    if (!m_penDown)
    {
      QPointF mousePos = event->localPos();
      qreal pressure = 1;

      Qt::KeyboardModifiers keyboardModifiers = event->modifiers();
      mouseAndTabletEvent(mousePos, event->button(), event->buttons(), keyboardModifiers, QTabletEvent::Pen, event->type(), pressure, false);
    }
  }
  m_penDown = false;
}

void Widget::setPreviousTool()
{
  if (m_previousTool == tool::PEN)
  {
    emit pen();
  }
  if (m_previousTool == tool::RULER)
  {
    emit ruler();
  }
  if (m_previousTool == tool::CIRCLE)
  {
    emit circle();
  }
  if (m_previousTool == tool::ERASER)
  {
    emit eraser();
  }
  if (m_previousTool == tool::SELECT)
  {
    emit select();
  }
  if (m_previousTool == tool::HAND)
  {
    emit hand();
  }

  m_previousTool = tool::NONE;
}

void Widget::startSelecting(QPointF mousePos)
{
  int pageNum = getPageFromMousePos(mousePos);
  QPointF pagePos = getPagePosFromMousePos(mousePos, pageNum);

  MrDoc::Selection newSelection;

  newSelection.setPageNum(pageNum);
  newSelection.setWidth(m_currentDocument.pages[pageNum].width());
  newSelection.setHeight(m_currentDocument.pages[pageNum].height());
  newSelection.appendToSelectionPolygon(pagePos);

  currentSelection = newSelection;

  //    selecting = true;
  m_currentState = state::SELECTING;
  m_selectingOnPage = pageNum;
}

void Widget::continueSelecting(QPointF mousePos)
{
  int pageNum = m_selectingOnPage;
  QPointF pagePos = getPagePosFromMousePos(mousePos, pageNum);

  currentSelection.appendToSelectionPolygon(pagePos);

  update();
}

void Widget::stopSelecting(QPointF mousePos)
{
  int pageNum = m_selectingOnPage;
  QPointF pagePos = getPagePosFromMousePos(mousePos, pageNum);

  currentSelection.appendToSelectionPolygon(pagePos);

  if (!m_currentDocument.pages[pageNum].getStrokes(currentSelection.selectionPolygon()).isEmpty())
  {
    CreateSelectionCommand *createSelectionCommand = new CreateSelectionCommand(this, pageNum, currentSelection);
    m_undoStack.push(createSelectionCommand);

    emit updateGUI();
    update();
  }
  else
  {
    setCurrentState(state::IDLE);
  }
}

void Widget::letGoSelection()
{
  if (getCurrentState() == state::SELECTED)
  {
    int pageNum = currentSelection.pageNum();
    ReleaseSelectionCommand *releaseCommand = new ReleaseSelectionCommand(this, pageNum);
    m_undoStack.push(releaseCommand);
    updateAllDirtyBuffers();
    setCurrentState(state::IDLE);
  }
}

void Widget::startRuling(QPointF mousePos)
{
  m_currentDocument.setDocumentChanged(true);
  emit modified();

  int pageNum = getPageFromMousePos(mousePos);
  QPointF pagePos = getPagePosFromMousePos(mousePos, pageNum);

  MrDoc::Stroke newStroke;
  newStroke.m_pattern = m_currentPattern;
  newStroke.m_points.append(pagePos);
  newStroke.m_pressures.append(1);
  newStroke.m_penWidth = m_currentPenWidth;
  newStroke.m_color = m_currentColor;
  m_currentStroke = newStroke;
  m_currentState = state::RULING;

  m_previousMousePos = mousePos;
  m_firstMousePos = mousePos;
  m_drawingOnPage = pageNum;
}

void Widget::continueRuling(QPointF mousePos)
{
  QPointF pagePos = getPagePosFromMousePos(mousePos, m_drawingOnPage);
  QPointF previousPagePos = getPagePosFromMousePos(m_previousMousePos, m_drawingOnPage);

  QPointF firstPagePos = m_currentStroke.m_points.at(0);

  QPointF oldPagePos = pagePos;

  m_currentDashOffset = 0.0;

  if (m_currentStroke.m_points.length() > 1)
  {
    oldPagePos = m_currentStroke.m_points.at(1);
    m_currentStroke.m_points.removeAt(1);
    m_currentStroke.m_pressures.removeAt(1);
  }

  m_currentStroke.m_points.append(pagePos);
  m_currentStroke.m_pressures.append(1);

  QRect clipRect(m_zoom * firstPagePos.toPoint(), m_zoom * pagePos.toPoint());
  QRect oldClipRect(m_zoom * firstPagePos.toPoint(), m_zoom * previousPagePos.toPoint());
  clipRect = clipRect.normalized().united(oldClipRect.normalized());
  int clipRad = m_zoom * m_currentPenWidth / 2 + 2;
  clipRect = clipRect.normalized().adjusted(-clipRad, -clipRad, clipRad, clipRad);
  updateBufferRegion(m_drawingOnPage, clipRect);
  drawOnBuffer();

  QRect updateRect(m_firstMousePos.toPoint(), mousePos.toPoint());
  QRect oldUpdateRect(m_firstMousePos.toPoint(), m_previousMousePos.toPoint());
  updateRect = updateRect.normalized().united(oldUpdateRect.normalized());
  int rad = m_currentPenWidth * m_zoom / 2 + 2;
  updateRect = updateRect.normalized().adjusted(-rad, -rad, +rad, +rad);

  update(updateRect);

  m_previousMousePos = mousePos;
}

void Widget::stopRuling(QPointF mousePos)
{
  QPointF pagePos = getPagePosFromMousePos(mousePos, m_drawingOnPage);

  if (m_currentStroke.m_points.length() > 1)
  {
    m_currentStroke.m_points.removeAt(1);
    m_currentStroke.m_pressures.removeAt(1);
  }

  m_currentStroke.m_points.append(pagePos);
  m_currentStroke.m_pressures.append(1);

  AddStrokeCommand *addCommand = new AddStrokeCommand(this, m_drawingOnPage, m_currentStroke);
  m_undoStack.push(addCommand);

  m_currentState = state::IDLE;

  update();
}

void Widget::startCircling(QPointF mousePos)
{
  m_currentDocument.setDocumentChanged(true);
  emit modified();

  int pageNum = getPageFromMousePos(mousePos);

  MrDoc::Stroke newStroke;
  //    newStroke.points.append(pagePos);
  //    newStroke.pressures.append(1);
  newStroke.m_pattern = m_currentPattern;
  newStroke.m_penWidth = m_currentPenWidth;
  newStroke.m_color = m_currentColor;
  m_currentStroke = newStroke;
  m_currentState = state::CIRCLING;

  m_previousMousePos = mousePos;
  m_firstMousePos = mousePos;
  m_drawingOnPage = pageNum;
}

void Widget::continueCircling(QPointF mousePos)
{
  QPointF pagePos = getPagePosFromMousePos(mousePos, m_drawingOnPage);
  QPointF firstPagePos = getPagePosFromMousePos(m_firstMousePos, m_drawingOnPage);

  m_currentDashOffset = 0.0;

  MrDoc::Stroke oldStroke = m_currentStroke;

  m_currentStroke.m_points.clear();
  m_currentStroke.m_pressures.clear();

  qreal radius = QLineF(firstPagePos, pagePos).length();
  qreal phi0 = QLineF(firstPagePos, pagePos).angle() * M_PI / 180.0;

  int N = 100;
  for (int i = 0; i < N; ++i)
  {
    qreal phi = phi0 + i * (2.0 * M_PI / (N - 1));
    qreal x = firstPagePos.x() + radius * cos(phi);
    qreal y = firstPagePos.y() - radius * sin(phi);
    m_currentStroke.m_points.append(QPointF(x, y));
    m_currentStroke.m_pressures.append(1.0);
  }

  QTransform scaleTrans;
  scaleTrans = scaleTrans.scale(m_zoom, m_zoom);

  QRect clipRect = scaleTrans.mapRect(m_currentStroke.m_points.boundingRect()).toRect();
  QRect oldClipRect = scaleTrans.mapRect(oldStroke.m_points.boundingRect()).toRect();
  clipRect = clipRect.normalized().united(oldClipRect.normalized());
  int clipRad = m_zoom * m_currentPenWidth / 2 + 2;
  clipRect = clipRect.normalized().adjusted(-clipRad, -clipRad, clipRad, clipRad);
  updateBufferRegion(m_drawingOnPage, clipRect);

  drawOnBuffer();

  update();

  m_previousMousePos = mousePos;
}

void Widget::stopCircling(QPointF mousePos)
{
  continueCircling(mousePos);

  AddStrokeCommand *addCommand = new AddStrokeCommand(this, m_drawingOnPage, m_currentStroke);
  m_undoStack.push(addCommand);

  m_currentState = state::IDLE;

  update();
}

void Widget::startDrawing(QPointF mousePos, qreal pressure)
{
  m_updateTimer->start(33); // 33 -> 30 fps

  m_currentDocument.setDocumentChanged(true);
  emit modified();

  m_currentUpdateRect = QRect();

  int pageNum = getPageFromMousePos(mousePos);
  QPointF pagePos = getPagePosFromMousePos(mousePos, pageNum);

  m_currentDashOffset = 0.0;

  MrDoc::Stroke newStroke;
  newStroke.m_pattern = m_currentPattern;
  newStroke.m_points.append(pagePos);
  newStroke.m_pressures.append(pressure);
  newStroke.m_penWidth = m_currentPenWidth;
  newStroke.m_color = m_currentColor;
  m_currentStroke = newStroke;
  //    drawing = true;
  m_currentState = state::DRAWING;

  m_previousMousePos = mousePos;
  m_drawingOnPage = pageNum;
}

void Widget::continueDrawing(QPointF mousePos, qreal pressure)
{
  QPointF pagePos = getPagePosFromMousePos(mousePos, m_drawingOnPage);

  m_currentStroke.m_points.append(pagePos);
  m_currentStroke.m_pressures.append(pressure);
  drawOnBuffer(true);

  QRect updateRect(m_previousMousePos.toPoint(), mousePos.toPoint());
  int rad = m_currentPenWidth * m_zoom / 2 + 2;
  updateRect = updateRect.normalized().adjusted(-rad, -rad, +rad, +rad);

  m_currentUpdateRect = m_currentUpdateRect.united(updateRect);

  m_previousMousePos = mousePos;
}

void Widget::stopDrawing(QPointF mousePos, qreal pressure)
{
  m_updateTimer->stop();

  QPointF pagePos = getPagePosFromMousePos(mousePos, m_drawingOnPage);

  m_currentStroke.m_points.append(pagePos);
  m_currentStroke.m_pressures.append(pressure);
  drawOnBuffer();

  AddStrokeCommand *addCommand = new AddStrokeCommand(this, m_drawingOnPage, m_currentStroke, -1, false, true);
  m_undoStack.push(addCommand);

  //  currentState = state::IDLE;
  setCurrentState(state::IDLE);

  update();
}

void Widget::erase(QPointF mousePos, bool invertEraser)
{
  int pageNum = getPageFromMousePos(mousePos);
  QPointF pagePos = getPagePosFromMousePos(mousePos, pageNum);

  const QVector<MrDoc::Stroke> &strokes = m_currentDocument.pages[pageNum].strokes();

  qreal eraserWidth = 10;

  //    QLineF lineA = QLineF(pagePos + QPointF(-eraserWidth,-eraserWidth) / 2.0, pagePos + QPointF( eraserWidth,  eraserWidth) / 2.0);
  //    QLineF lineB = QLineF(pagePos + QPointF( eraserWidth,-eraserWidth) / 2.0, pagePos + QPointF(-eraserWidth,  eraserWidth) / 2.0); // lineA and lineB
  //    form a cross X

  QLineF lineA = QLineF(pagePos + QPointF(-eraserWidth, -eraserWidth) / 2.0, pagePos + QPointF(-eraserWidth, eraserWidth) / 2.0);
  QLineF lineB = QLineF(pagePos + QPointF(-eraserWidth, eraserWidth) / 2.0, pagePos + QPointF(eraserWidth, eraserWidth) / 2.0);
  QLineF lineC = QLineF(pagePos + QPointF(eraserWidth, eraserWidth) / 2.0, pagePos + QPointF(eraserWidth, -eraserWidth) / 2.0);
  QLineF lineD = QLineF(pagePos + QPointF(eraserWidth, -eraserWidth) / 2.0, pagePos + QPointF(-eraserWidth, -eraserWidth) / 2.0); // lineA B C D form a square

  QRectF rectE = QRectF(pagePos + QPointF(-eraserWidth, eraserWidth) / 2.0, pagePos + QPointF(eraserWidth, -eraserWidth) / 2.0);

  QVector<int> strokesToDelete;
  QPointF iPoint;

  QPointF iPointA;
  QPointF iPointB;
  QPointF iPointC;
  QPointF iPointD;

  bool intersected;

  if (m_realEraser || (!m_realEraser && invertEraser))
  {
    for (int i = strokes.size() - 1; i >= 0; --i)
    {
      MrDoc::Stroke stroke = strokes.at(i);
      if (rectE.intersects(stroke.m_points.boundingRect()) || !stroke.m_points.boundingRect().isValid()) // this is done for speed
      {
        for (int j = 0; j < stroke.m_points.length() - 1; ++j)
        {
          QLineF line = QLineF(stroke.m_points.at(j), stroke.m_points.at(j + 1));
          if (line.intersect(lineA, &iPointA) == QLineF::BoundedIntersection && iPointA != stroke.m_points.first() && iPointA != stroke.m_points.last())
          {
            iPoint = iPointA;
            intersected = true;
          }
          else if (line.intersect(lineB, &iPointB) == QLineF::BoundedIntersection && iPointB != stroke.m_points.first() && iPointB != stroke.m_points.last())
          {
            iPoint = iPointB;
            intersected = true;
          }
          else if (line.intersect(lineC, &iPointC) == QLineF::BoundedIntersection && iPointC != stroke.m_points.first() && iPointC != stroke.m_points.last())
          {
            iPoint = iPointC;
            intersected = true;
          }
          else if (line.intersect(lineD, &iPointD) == QLineF::BoundedIntersection && iPointD != stroke.m_points.first() && iPointD != stroke.m_points.last())
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
              splitStroke.m_points = splitStroke.m_points.mid(0, j + 1);
              splitStroke.m_points.append(iPoint);
              splitStroke.m_pressures = splitStroke.m_pressures.mid(0, j + 1);
              qreal lastPressure = splitStroke.m_pressures.last();
              splitStroke.m_pressures.append(lastPressure);

              stroke.m_points = stroke.m_points.mid(j + 1);
              stroke.m_points.prepend(iPoint);
              stroke.m_pressures = stroke.m_pressures.mid(j + 1);
              qreal firstPressure = stroke.m_pressures.first();
              stroke.m_pressures.prepend(firstPressure);

              RemoveStrokeCommand *removeStrokeCommand = new RemoveStrokeCommand(this, pageNum, i, false);
              m_undoStack.push(removeStrokeCommand);
              AddStrokeCommand *addStrokeCommand = new AddStrokeCommand(this, pageNum, stroke, i, false, false);
              m_undoStack.push(addStrokeCommand);
              addStrokeCommand = new AddStrokeCommand(this, pageNum, splitStroke, i, false, false);
              m_undoStack.push(addStrokeCommand);
              //                            strokes.insert(i, splitStroke);
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
  lineA = QLineF(pagePos + QPointF(-eraserWidth, -eraserWidth) / 2.0, pagePos + QPointF(-eraserWidth, eraserWidth) / 2.0);

  lineB = QLineF(pagePos + QPointF(-eraserWidth, eraserWidth) / 2.0, pagePos + QPointF(eraserWidth, eraserWidth) / 2.0);
  lineC = QLineF(pagePos + QPointF(eraserWidth, eraserWidth) / 2.0, pagePos + QPointF(eraserWidth, -eraserWidth) / 2.0);
  lineD = QLineF(pagePos + QPointF(eraserWidth, -eraserWidth) / 2.0, pagePos + QPointF(-eraserWidth, -eraserWidth) / 2.0); // lineA B C D form a square

  rectE = QRectF(pagePos + QPointF(-eraserWidth, eraserWidth) / 2.0, pagePos + QPointF(eraserWidth, -eraserWidth) / 2.0);

  for (int i = 0; i < strokes.size(); ++i)
  {
    const MrDoc::Stroke stroke = strokes.at(i);
    if (rectE.intersects(stroke.m_points.boundingRect()) || !stroke.m_points.boundingRect().isValid()) // this is done for speed
    {
      bool foundStrokeToDelete = false;
      for (int j = 0; j < stroke.m_points.length(); ++j)
      {
        if (rectE.contains(stroke.m_points.at(j)))
        {
          strokesToDelete.append(i);
          foundStrokeToDelete = true;
          break;
        }
      }
      if (foundStrokeToDelete == false)
      {
        for (int j = 0; j < stroke.m_points.length() - 1; ++j)
        {
          QLineF line = QLineF(stroke.m_points.at(j), stroke.m_points.at(j + 1));
          if (line.intersect(lineA, &iPoint) == QLineF::BoundedIntersection || line.intersect(lineB, &iPoint) == QLineF::BoundedIntersection ||
              line.intersect(lineC, &iPoint) == QLineF::BoundedIntersection || line.intersect(lineD, &iPoint) == QLineF::BoundedIntersection)
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
    m_currentDocument.setDocumentChanged(true);
    emit modified();

    //    QRect updateRect;
    std::sort(strokesToDelete.begin(), strokesToDelete.end(), std::greater<int>());
    for (int i = 0; i < strokesToDelete.size(); ++i)
    {
      //      updateRect = updateRect.united(currentDocument.pages[pageNum].m_strokes.at(strokesToDelete.at(i)).points.boundingRect().toRect());
      RemoveStrokeCommand *removeCommand = new RemoveStrokeCommand(this, pageNum, strokesToDelete[i]);
      m_undoStack.push(removeCommand);
    }
  }
  updateAllDirtyBuffers();
}

void Widget::startMovingSelection(QPointF mousePos)
{
  m_currentDocument.setDocumentChanged(true);
  emit modified();

  int pageNum = getPageFromMousePos(mousePos);
  m_previousPagePos = getPagePosFromMousePos(mousePos, pageNum);
  setCurrentState(state::MOVING_SELECTION);
}

void Widget::continueMovingSelection(QPointF mousePos)
{
  int pageNum = getPageFromMousePos(mousePos);
  QPointF pagePos = getPagePosFromMousePos(mousePos, pageNum);
  //    currentSelection.move(1 * (pagePos - previousPagePos));

  QPointF delta = (pagePos - m_previousPagePos);

  QTransform transform;
  transform.translate(delta.x(), delta.y());

  //    currentSelection.transform(transform, pageNum);
  TransformSelectionCommand *transSelectCommand = new TransformSelectionCommand(this, pageNum, transform);
  m_undoStack.push(transSelectCommand);

  m_previousPagePos = pagePos;
  //    update(currentSelection.selectionPolygon.boundingRect().toRect());
}

void Widget::startRotatingSelection(QPointF mousePos)
{
  m_currentDocument.setDocumentChanged(true);
  emit modified();

  m_currentAngle = 0.0;

  int pageNum = currentSelection.pageNum();
  m_previousPagePos = getPagePosFromMousePos(mousePos, pageNum);
  setCurrentState(state::ROTATING_SELECTION);
}

void Widget::continueRotatingSelection(QPointF mousePos)
{
  int pageNum = currentSelection.pageNum();
  QPointF pagePos = getPagePosFromMousePos(mousePos, pageNum);

  m_currentAngle = QLineF(currentSelection.boundingRect().center(), pagePos).angleTo(QLineF(currentSelection.boundingRect().center(), m_previousPagePos));
  currentSelection.setAngle(m_currentAngle);
}

void Widget::stopRotatingSelection(QPointF mousePos)
{
  continueRotatingSelection(mousePos);
  int pageNum = currentSelection.pageNum();

  QTransform transform;
  transform.translate(currentSelection.boundingRect().center().x(), currentSelection.boundingRect().center().y());
  transform.rotate(m_currentAngle);
  transform.translate(-currentSelection.boundingRect().center().x(), -currentSelection.boundingRect().center().y());

  currentSelection.setAngle(0.0);

  TransformSelectionCommand *transSelectCommand = new TransformSelectionCommand(this, pageNum, transform);
  m_undoStack.push(transSelectCommand);

  currentSelection.finalize();
  currentSelection.updateBuffer(m_zoom);
  setCurrentState(state::SELECTED);
}

void Widget::startResizingSelection(QPointF mousePos, MrDoc::Selection::GrabZone grabZone)
{
  m_currentDocument.setDocumentChanged(true);
  emit modified();

  m_grabZone = grabZone;

  int pageNum = currentSelection.pageNum();
  m_previousPagePos = getPagePosFromMousePos(mousePos, pageNum);
  setCurrentState(state::RESIZING_SELECTION);
}

void Widget::continueResizingSelection(QPointF mousePos)
{
  int pageNum = currentSelection.pageNum();
  QPointF pagePos = getPagePosFromMousePos(mousePos, pageNum);
  QPointF delta = (pagePos - m_previousPagePos);

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
  m_undoStack.push(transSelectCommand);

  m_previousPagePos = pagePos;
}

void Widget::stopResizingSelection(QPointF mousePos)
{
  continueResizingSelection(mousePos);

  currentSelection.finalize();
  currentSelection.updateBuffer(m_zoom);
  setCurrentState(state::SELECTED);
}

int Widget::getPageFromMousePos(QPointF mousePos)
{
  qreal y = mousePos.y(); // - currentCOSPos.y();
  int pageNum = 0;
  while (y > (floor(m_currentDocument.pages[pageNum].height() * m_zoom)) + PAGE_GAP)
  {
    y -= (floor(m_currentDocument.pages[pageNum].height() * m_zoom)) + PAGE_GAP;
    pageNum += 1;
    if (pageNum >= m_currentDocument.pages.size())
    {
      pageNum = m_currentDocument.pages.size() - 1;
      break;
    }
  }
  return pageNum;
}

int Widget::getCurrentPage()
{
  QPoint globalMousePos = parentWidget()->mapToGlobal(QPoint(0, 0)) + QPoint(parentWidget()->size().width() / 2, parentWidget()->size().height() / 2);
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
    //        y -= (currentDocument.pages[i].height() * zoom + PAGE_GAP); // THIS DOESN'T WORK PROPERLY (should be floor(...height(), or just use
    //        pageBuffer[i].height()/devicePixelRatio())
    y -= (m_pageBuffer[i].height()/devicePixelRatio() + PAGE_GAP);
  }
  //    y -= (pageNum) * (currentDocument.pages[0].height() * zoom + PAGE_GAP);

  QPointF pagePos = (QPointF(x, y)) / m_zoom;

  return pagePos;
}

QPointF Widget::getAbsolutePagePosFromMousePos(QPointF mousePos)
{
  int pageNum = getPageFromMousePos(mousePos);
  QPointF pagePos = getPagePosFromMousePos(mousePos, pageNum);

  qreal y = 0.0;
  for (int i = 0; i < pageNum; ++i)
  {
    y += (m_pageBuffer[i].height()/devicePixelRatio() + PAGE_GAP);
  }
  y *= m_zoom;

  pagePos.setY(y + pagePos.y());

  return pagePos;
}

void Widget::newFile()
{
  letGoSelection();

  m_currentDocument = MrDoc::Document();
  m_pageBuffer.clear();
  m_undoStack.clear();
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

  int prevHMax = m_scrollArea->horizontalScrollBar()->maximum();
  int prevVMax = m_scrollArea->verticalScrollBar()->maximum();

  int prevH = m_scrollArea->horizontalScrollBar()->value();
  int prevV = m_scrollArea->verticalScrollBar()->value();

  updateAllPageBuffers();
  currentSelection.updateBuffer(m_zoom);
  setGeometry(getWidgetGeometry());

  int newHMax = m_scrollArea->horizontalScrollBar()->maximum();
  int newVMax = m_scrollArea->verticalScrollBar()->maximum();

  int newH, newV;

  if (prevHMax != 0)
  {
    newH = double(prevH) / prevHMax * newHMax;
  }
  else
  {
    newH = newHMax / 2;
  }
  if (prevVMax != 0)
  {
    newV = double(prevV) / prevVMax * newVMax;
  }
  else
  {
    newV = newVMax / 2;
  }

  m_scrollArea->horizontalScrollBar()->setValue(newH);
  m_scrollArea->verticalScrollBar()->setValue(newV);

  update();
}

void Widget::zoomFitWidth()
{
  int pageNum = getCurrentPage();

  QSize widgetSize = this->parentWidget()->size();
  qreal newZoom = widgetSize.width() / m_currentDocument.pages[pageNum].width();

  zoomTo(newZoom);
}

void Widget::zoomFitHeight()
{
  int pageNum = getCurrentPage();

  QSize widgetSize = this->parentWidget()->size();
  qreal newZoom = widgetSize.height() / m_currentDocument.pages[pageNum].height();

  zoomTo(newZoom);
}

void Widget::pageFirst()
{
  scrollDocumentToPageNum(0);
}

void Widget::pageLast()
{
  scrollDocumentToPageNum(m_currentDocument.pages.size() - 1);
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

  if (pageNum >= m_currentDocument.pages.size())
  {
    pageAddEnd();
  }

  scrollDocumentToPageNum(pageNum);
}

void Widget::pageAddBefore()
{
  int pageNum = getCurrentPage();
  AddPageCommand *addPageCommand = new AddPageCommand(this, pageNum);
  m_undoStack.push(addPageCommand);
  setGeometry(getWidgetGeometry());
  update();
  m_currentDocument.setDocumentChanged(true);
  emit modified();
}

void Widget::pageAddAfter()
{
  int pageNum = getCurrentPage() + 1;
  AddPageCommand *addPageCommand = new AddPageCommand(this, pageNum);
  m_undoStack.push(addPageCommand);
  setGeometry(getWidgetGeometry());
  update();
  m_currentDocument.setDocumentChanged(true);

  emit modified();
}

void Widget::pageAddBeginning()
{
  AddPageCommand *addPageCommand = new AddPageCommand(this, 0);
  m_undoStack.push(addPageCommand);
  setGeometry(getWidgetGeometry());
  update();
  m_currentDocument.setDocumentChanged(true);

  emit modified();
}

void Widget::pageAddEnd()
{
  AddPageCommand *addPageCommand = new AddPageCommand(this, m_currentDocument.pages.size());
  m_undoStack.push(addPageCommand);
  setGeometry(getWidgetGeometry());
  update();
  m_currentDocument.setDocumentChanged(true);

  emit modified();
}

void Widget::pageRemove()
{
  if (m_currentDocument.pages.size() > 1)
  {
    int pageNum = getCurrentPage();
    RemovePageCommand *removePageCommand = new RemovePageCommand(this, pageNum);
    m_undoStack.push(removePageCommand);
    setGeometry(getWidgetGeometry());
    update();
    m_currentDocument.setDocumentChanged(true);
    emit modified();
  }
}

void Widget::scrollDocumentToPageNum(int pageNum)
{
  if (pageNum >= m_currentDocument.pages.size())
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
    y += (m_currentDocument.pages[i].height()) * m_zoom + PAGE_GAP;
  }

  m_scrollArea->verticalScrollBar()->setValue(y);

  //    currentCOSPos = QPointF(x, y);
  //    updateAllPageBuffers();
  //    update();
}

void Widget::setCurrentTool(tool toolID)
{
  if (m_currentState == state::IDLE || m_currentState == state::SELECTED)
  {
    if (toolID == tool::SELECT && m_currentState == state::SELECTED)
    {
      letGoSelection();
    }
    m_currentTool = toolID;
    if (toolID == tool::PEN)
      setCursor(m_penCursor);
    if (toolID == tool::RULER)
      setCursor(m_rulerCursor);
    if (toolID == tool::CIRCLE)
      setCursor(m_circleCursor);
    if (toolID == tool::ERASER)
      setCursor(m_eraserCursor);
    if (toolID == tool::SELECT)
      setCursor(Qt::CrossCursor);
    if (toolID == tool::HAND)
      setCursor(Qt::OpenHandCursor);
  }
}

void Widget::setDocument(const MrDoc::Document &newDocument)
{
  m_currentDocument = newDocument;
  m_undoStack.clear();
  m_pageBuffer.clear();
  m_zoom = 0.0; // otherwise zoomTo() doesn't do anything if zoom == newZoom
  zoomFitWidth();
  pageFirst();
}

void Widget::selectAll()
{

  if (m_currentState == state::SELECTED)
  {
    letGoSelection();
  }

  int pageNum = getCurrentPage();
  QRectF selectRect;
  for (auto &stroke : m_currentDocument.pages[pageNum].strokes())
  {
    selectRect = selectRect.united(stroke.boundingRect());
  }
  QPolygonF selectionPolygon = QPolygonF(selectRect);

  MrDoc::Selection selection;
  selection.setPageNum(pageNum);
  selection.setSelectionPolygon(selectionPolygon);

  if (!m_currentDocument.pages[pageNum].getStrokes(selection.selectionPolygon()).isEmpty())
  {
    currentSelection = selection;
    CreateSelectionCommand *createSelectionCommand = new CreateSelectionCommand(this, pageNum, selection);
    m_undoStack.push(createSelectionCommand);

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
  clipboard = currentSelection;
  update();
}

void Widget::paste()
{
  MrDoc::Selection tmpSelection = clipboard;
  tmpSelection.setPageNum(getCurrentPage());

  QPoint globalMousePos = parentWidget()->mapToGlobal(QPoint(0, 0)) + QPoint(parentWidget()->size().width() / 2, parentWidget()->size().height() / 2);
  QPoint mousePos = this->mapFromGlobal(globalMousePos);
  QPointF selectionPos = getPagePosFromMousePos(mousePos, getCurrentPage()) - tmpSelection.boundingRect().center();

  QTransform myTrans;
  myTrans = myTrans.translate(selectionPos.x(), selectionPos.y());

  tmpSelection.transform(myTrans, tmpSelection.pageNum());

  //  for (int i = 0; i < tmpSelection.strokes().size(); ++i)
  //  {
  //    tmpSelection.m_strokes[i].points = myTrans.map(tmpSelection.m_strokes[i].points);
  //  }
  tmpSelection.finalize();
  tmpSelection.updateBuffer(m_zoom);

  m_undoStack.beginMacro("Paste");
  if (m_currentState == state::SELECTED)
  {
    letGoSelection();
  }
  PasteCommand *pasteCommand = new PasteCommand(this, tmpSelection);
  m_undoStack.push(pasteCommand);
  m_undoStack.endMacro();

  m_currentDocument.setDocumentChanged(true);
  emit modified();
}

void Widget::cut()
{
  CutCommand *cutCommand = new CutCommand(this);
  m_undoStack.push(cutCommand);
  //    clipboard = currentSelection;
  //    currentSelection = Selection();
  //    currentState = state::IDLE;
  //    update();
}

void Widget::undo()
{
  if (m_undoStack.canUndo() && (m_currentState == state::IDLE || m_currentState == state::SELECTED))
  {
    m_undoStack.undo();
    currentSelection.updateBuffer(m_zoom);
    updateAllDirtyBuffers();
  }
}

void Widget::redo()
{
  if (m_undoStack.canRedo() && (m_currentState == state::IDLE || m_currentState == state::SELECTED))
  {
    m_undoStack.redo();
    currentSelection.updateBuffer(m_zoom);
    updateAllDirtyBuffers();
  }
}

void Widget::setCurrentState(state newState)
{
  m_currentState = newState;
  if (m_currentState == state::IDLE || m_currentState == state::SELECTED)
  {
    updateAllDirtyBuffers();
    m_updateDirtyTimer->stop();
  }
  else
  {
    m_updateDirtyTimer->start();
  }
}

Widget::state Widget::getCurrentState()
{
  return m_currentState;
}

void Widget::setCurrentColor(QColor newColor)
{
  m_currentColor = newColor;
  if (m_currentState == state::SELECTED)
  {
    ChangeColorOfSelectionCommand *changeColorCommand = new ChangeColorOfSelectionCommand(this, newColor);
    m_undoStack.push(changeColorCommand);
    currentSelection.updateBuffer(m_zoom);
    update();
  }
}

QColor Widget::getCurrentColor()
{
  return m_currentColor;
}

void Widget::veryFine()
{
  setCurrentPenWidth(m_veryFinePenWidth);
  emit updateGUI();
}

void Widget::fine()
{
  setCurrentPenWidth(m_finePenWidth);
  emit updateGUI();
}

void Widget::medium()
{
  setCurrentPenWidth(m_mediumPenWidth);
  emit updateGUI();
}

void Widget::thick()
{
  setCurrentPenWidth(m_thickPenWidth);
  emit updateGUI();
}

void Widget::veryThick()
{
  setCurrentPenWidth(m_veryThickPenWidth);
  emit updateGUI();
}

void Widget::rotateSelection(qreal angle)
{
  QTransform rotateTrans;

  qreal dx = currentSelection.boundingRect().center().x();
  qreal dy = currentSelection.boundingRect().center().y();

  rotateTrans = rotateTrans.translate(dx, dy).rotate(-angle).translate(-dx, -dy);
  TransformSelectionCommand *transCommand = new TransformSelectionCommand(this, currentSelection.pageNum(), rotateTrans);
  m_undoStack.push(transCommand);
  currentSelection.finalize();
  currentSelection.updateBuffer(m_zoom);
  update();
}

void Widget::setCurrentPattern(QVector<qreal> newPattern)
{
  m_currentPattern = newPattern;
  emit updateGUI();
  if (m_currentState == state::SELECTED)
  {
    ChangePatternOfSelectionCommand *changePatternCommand = new ChangePatternOfSelectionCommand(this, newPattern);
    m_undoStack.push(changePatternCommand);
    currentSelection.updateBuffer(m_zoom);
    update();
  }
}

QVector<qreal> Widget::getCurrentPattern()
{
  return m_currentPattern;
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
  m_currentPenWidth = penWidth;
  if (m_currentState == state::SELECTED)
  {
    ChangePenWidthOfSelectionCommand *changePenWidthCommand = new ChangePenWidthOfSelectionCommand(this, penWidth);
    m_undoStack.push(changePenWidthCommand);
    currentSelection.updateBuffer(m_zoom);
    update();
  }
}
