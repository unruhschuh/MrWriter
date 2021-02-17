#include "commands.h"
#include "tabletapplication.h"
#include "tools.h"
#include "image.h"
#include "text.h"
#include "keypresseater.h"

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
#include <QImage>
#include <QImageReader>
#include <QFontMetrics>
#include <QLayout>
#include <qmath.h>

#include <memory>

#define PAGE_GAP 10.0
#define ZOOM_STEP 1.2

#include <QPainter>
#include <QRectF>

Widget::Widget(QWidget *parent) : QWidget(parent)
{
  QSettings settings;

  m_currentState = State::IDLE;

  setAcceptDrops(true);

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

  QPixmap strokeEraserCursorBitmap = QPixmap(":/images/strokeEraserCursor.png");
  QPixmap strokeEraserCursorMask = QPixmap(":/images/strokeEraserCursorMask.png");
  strokeEraserCursorBitmap.setMask(QBitmap(strokeEraserCursorMask));
  m_strokeEraserCursor = QCursor(strokeEraserCursorBitmap, -1, -1);

  m_textEdit = new QPlainTextEdit(this);
  m_textEdit->setWordWrapMode(QTextOption::NoWrap);
  m_textEdit->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  m_textEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
  m_textEdit->setStyleSheet("QPlainTextEdit { border: 0px; border-radius: 10px; min-width: 20px; padding: 0px; background-color: #EEEEEE; }");
  connect(m_textEdit, &QPlainTextEdit::textChanged, [=](){
    QFontMetrics fm(m_textEdit->font());
    QRectF textRect = fm.boundingRect(QRect(), Qt::AlignLeft, m_textEdit->toPlainText());
    QRectF geometry = m_textEdit->geometry();
    geometry.setWidth(textRect.width() + 5 * m_zoom);
    geometry.setHeight(textRect.height()+5 * m_zoom);
    m_textEdit->setGeometry(geometry.toRect());
    m_currentText.m_text = m_textEdit->toPlainText();
  });
  m_textEdit->hide();

  m_currentTool = Tool::PEN;
  m_previousTool = Tool::NONE;
  setCursor(penCursorBitmap);

  m_currentDocument = MrDoc::Document();

  m_currentPenWidth = 1.41;
  m_currentColor = QColor(0, 0, 0);
  m_zoom = 1.0;

  m_showGrid = false;
  m_snapToGrid = false;
  m_gridWidth = 14.1732;

  m_currentCOSPos.setX(0.0);
  m_currentCOSPos.setY(0.0);
  setGeometry(getWidgetGeometry());

  parent->updateGeometry();
  parent->update();

  m_updateTimer = new QTimer(this);
  connect(m_updateTimer, SIGNAL(timeout()), this, SLOT(updateWhileDrawing()));

  m_updateDirtyTimer = new QTimer(this);
  connect(m_updateDirtyTimer, SIGNAL(timeout()), this, SLOT(updateAllDirtyBuffers()));
  m_updateDirtyTimer->setInterval(15);

//  updateAllPageBuffers();
}

void Widget::enableInput()
{
  qDebug() << Q_FUNC_INFO;
  m_inputEnabled = true;
}

void Widget::disableInput()
{
  qDebug() << Q_FUNC_INFO;
  m_inputEnabled = false;
}

bool Widget::inputEnabled()
{
  return m_inputEnabled;
}

void Widget::showEvent( QShowEvent* event ) {
    QWidget::showEvent( event );
  updateAllPageBuffers();
}

size_t Widget::firstVisiblePage() const
{
  size_t firstVisiblePageNum = getPageFromMousePos(this->mapFromGlobal(m_scrollArea->mapToGlobal(QPoint(0, 0))));
  return firstVisiblePageNum;
}

size_t Widget::lastVisiblePage() const
{
  size_t lastVisiblePageNum = getPageFromMousePos(this->mapFromGlobal(m_scrollArea->mapToGlobal(QPoint(0, static_cast<int>(m_scrollArea->height())))));
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
  if (m_pageBuffer.empty())
  {
    for (size_t pageNum = 0; pageNum < m_currentDocument.pages.size(); ++pageNum)
    {
      m_pageBuffer.push_back(QPixmap(0,0));
    }
  }
  for (size_t pageNum = 0; pageNum < m_currentDocument.pages.size(); ++pageNum)
  {
    if (pageVisible(pageNum))
    {
      if ( force ||
           ( m_pageBuffer.at(pageNum).width() != m_currentDocument.pages.at(pageNum).pixelWidth(m_zoom, devicePixelRatio()) ||
             m_pageBuffer.at(pageNum).height() != m_currentDocument.pages.at(pageNum).pixelHeight(m_zoom, devicePixelRatio()) ) )
      {
        updateBuffer(pageNum);
      }
    } else {
      m_pageBuffer.at(pageNum) = QPixmap(0,0);
    }
  }
}

/**
 * @brief Widget::updateImageBuffer for concurrent update with QFuture in Widget::updateAllPageBuffers() since QPixmap cannot be used concurrently
 */
void Widget::updateImageBuffer(size_t buffNum)
{
  const auto &page = m_currentDocument.pages.at(buffNum);
  int pixelWidth = page.pixelWidth(m_zoom, devicePixelRatio());
  int pixelHeight = page.pixelHeight(m_zoom, devicePixelRatio());
  QImage image(pixelWidth, pixelHeight, QImage::Format_ARGB32_Premultiplied);
  image.setDevicePixelRatio(devicePixelRatio());

  image.fill(page.backgroundColor());

  QPainter painter;
  painter.begin(&image);
  painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

  if (m_showGrid)
  {
    drawGrid(painter, buffNum);
  }

  m_currentDocument.pages[buffNum].paint(painter, m_zoom);

  painter.end();

  m_pageImageBufferMutex.lock();
  m_pageImageBuffer.at(buffNum) = image;
  m_pageImageBufferMutex.unlock();
}

void Widget::updateBuffer(size_t buffNum)
{
  const auto &page = m_currentDocument.pages.at(buffNum);
  int pixelWidth = page.pixelWidth(m_zoom, devicePixelRatio());
  int pixelHeight = page.pixelHeight(m_zoom, devicePixelRatio());
  QPixmap pixmap(pixelWidth, pixelHeight);
  pixmap.setDevicePixelRatio(devicePixelRatio());

  pixmap.fill(page.backgroundColor());

  QPainter painter;
  painter.begin(&pixmap);
  painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

  if (m_showGrid)
  {
    drawGrid(painter, buffNum);
  }

  m_currentDocument.pages[buffNum].paint(painter, m_zoom);

  painter.end();

  m_pageBuffer.at(buffNum) = pixmap;
}

void Widget::updateBufferRegion(size_t buffNum, QRectF const &clipRect)
{
  QPainter painter;
  painter.begin(&m_pageBuffer[buffNum]);
  painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
  painter.setClipRect(clipRect);
  painter.setClipping(true);

  painter.fillRect(clipRect, m_currentDocument.pages.at(buffNum).backgroundColor());

  if (m_showGrid)
  {
    drawGrid(painter, buffNum);
  }
  //    painter.fillRect(clipRect, Qt::red);

  QRectF paintRect = QRectF(clipRect.topLeft() / m_zoom, clipRect.bottomRight() / m_zoom);
  m_currentDocument.pages[buffNum].paint(painter, m_zoom, paintRect);

  painter.end();
}

void Widget::updateAllDirtyBuffers()
{
  for (size_t buffNum = 0; buffNum < m_currentDocument.pages.size(); ++buffNum)
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

void Widget::drawGrid(QPainter &painter, size_t buffNum)
{
  const auto &page = m_currentDocument.pages.at(buffNum);
  int pixelWidth = page.pixelWidth(m_zoom, devicePixelRatio());
  int pixelHeight = page.pixelHeight(m_zoom, devicePixelRatio());

  int gridPixelWidth = static_cast<int>(m_zoom * m_gridWidth * devicePixelRatio());

  int numGridLinesX = static_cast<int>(pixelWidth / gridPixelWidth);
  int numGridLinesY = static_cast<int>(pixelHeight / gridPixelWidth);

  QPen pen;
  pen.setColor(MrDoc::gridColor);
  pen.setCapStyle(Qt::FlatCap);
  pen.setWidthF(0.5 * devicePixelRatioF());
  painter.setPen(pen);
  for (int i = 0; i < numGridLinesY; i++)
  {
    painter.drawLine(QLineF(0.0, static_cast<qreal>(i+1) * m_gridWidth * m_zoom, pixelWidth, static_cast<qreal>(i+1) * m_gridWidth * m_zoom));
  }
  for (int i = 0; i < numGridLinesX; i++)
  {
    painter.drawLine(QLineF(static_cast<qreal>(i+1) * m_gridWidth * m_zoom, 0, static_cast<qreal>(i+1) * m_gridWidth * m_zoom, pixelHeight));
  }
}

void Widget::drawOnBuffer(bool last)
{
  QPainter painter;
  painter.begin(&m_pageBuffer[m_drawingOnPage]);
  painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);

  m_currentStroke.paint(painter, m_zoom, last);
}

QRect Widget::getWidgetGeometry() const
{
  int width = 0;
  int height = 0;
  for (size_t i = 0; i < static_cast<size_t>(m_pageBuffer.size()); ++i)
  {
    height += m_currentDocument.pages.at(i).pixelHeight(m_zoom, 1) + PAGE_GAP;
    if (m_currentDocument.pages.at(i).pixelWidth(m_zoom, 1) > width)
    {
      width = m_currentDocument.pages.at(i).pixelWidth(m_zoom, 1);
    }
  }
  height -= PAGE_GAP;
  return QRect(0, 0, width, height);
}

void Widget::paintEvent(QPaintEvent *event)
{
  QPalette p(palette());
  setAutoFillBackground(true);
  setPalette(p);

  QPainter painter(this);

  if (m_currentState == State::DRAWING)
  {
    QRectF rectSource;
    QTransform trans;
    for (size_t i = 0; i < m_drawingOnPage; ++i)
    {
      trans = trans.translate(0, -(m_currentDocument.pages.at(i).pixelHeight(m_zoom) + PAGE_GAP) * devicePixelRatio());
    }
    trans = trans.scale(devicePixelRatio(),devicePixelRatio());
    rectSource = trans.mapRect(event->rect());

    painter.drawPixmap(event->rect(), m_pageBuffer[m_drawingOnPage], rectSource);

    return;
  }

  for (size_t i = 0; i < static_cast<size_t>(m_pageBuffer.size()); ++i)
  {
    QRectF rectSource;
    rectSource.setTopLeft(QPointF(0.0, 0.0));
    rectSource.setWidth(m_pageBuffer.at(i).width());
    rectSource.setHeight(m_pageBuffer.at(i).height());

    QRectF rectTarget;
    rectTarget.setTopLeft(QPointF(0.0, 0.0));
    rectTarget.setWidth(m_pageBuffer.at(i).width()/devicePixelRatio());
    rectTarget.setHeight(m_pageBuffer.at(i).height()/devicePixelRatio());

    painter.drawPixmap(rectTarget, m_pageBuffer.at(i), rectSource);

    if ((m_currentState == State::SELECTING || m_currentState == State::SELECTED || m_currentState == State::MOVING_SELECTION ||
         m_currentState == State::RESIZING_SELECTION || m_currentState == State::ROTATING_SELECTION) &&
        i == m_currentSelection.pageNum())
    {
      m_currentSelection.paint(painter, m_zoom);
    }

    auto & page = m_currentDocument.pages.at(i);
    int pixelHeight = page.pixelHeight(m_zoom, 1);
    painter.translate(QPointF(0.0, pixelHeight + PAGE_GAP));
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

  qDebug() << Q_FUNC_INFO;

  keyboardModifiers = qApp->queryKeyboardModifiers();

  if (!inputEnabled())
  {
    if (m_currentState == State::TEXTING && eventType == QEvent::MouseButtonRelease)
    {
      stopTexting(mousePos);
    }
    return;
  }

  size_t pageNum = getPageFromMousePos(mousePos);
  QPointF pagePos = getPagePosFromMousePos(mousePos, pageNum);

  if ((m_currentState == State::IDLE || m_currentState == State::SELECTED) && button & Qt::LeftButton && eventType == QEvent::MouseButtonRelease && pointerType == QTabletEvent::Pen && m_currentTool == Tool::TEXT)
  {
    size_t index = 0;
    for (auto & element : m_currentDocument.pages.at(pageNum).elements())
    {
      if (dynamic_cast<MrDoc::Text*>(element.get()) && element->boundingRect().contains(pagePos))
      {
        startEditingText(mousePos, index);
        return;
      }
      index++;
    }
    startTexting(mousePos);
    return;
  }

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

  if ((eventType == QEvent::MouseButtonPress || eventType == QEvent::MouseButtonRelease) && button == Qt::RightButton)
  {
    disableInput();
    emit quickmenu();
    return;
  }
  if ((m_currentState == State::IDLE || m_currentState == State::SELECTED) && buttons & Qt::MiddleButton && pointerType == QTabletEvent::Pen)
  {
    if (eventType == QEvent::MouseButtonPress && button == Qt::MiddleButton)
    {
      if (m_currentTool != Tool::HAND)
      {
        m_previousTool = m_currentTool;
      }
      m_previousMousePos = mousePos;
      emit hand();
      return;
    }
    if (eventType == QEvent::MouseMove)
    {
      int dx = 1 * static_cast<int>(mousePos.x() - m_previousMousePos.x());
      int dy = 1 * static_cast<int>(mousePos.y() - m_previousMousePos.y());

      m_scrollArea->horizontalScrollBar()->setValue(m_scrollArea->horizontalScrollBar()->value() - dx);
      m_scrollArea->verticalScrollBar()->setValue(m_scrollArea->verticalScrollBar()->value() - dy);

      qDebug() << dy;

      mousePos -= QPointF(dx, dy);

      m_previousMousePos = mousePos;
      return;
    }
    return;
  }

  if (m_currentState == State::SELECTED)
  {
    if (eventType == QEvent::MouseButtonPress)
    {
      using GrabZone = MrDoc::Selection::GrabZone;
      GrabZone grabZone = m_currentSelection.grabZone(pagePos, m_zoom);
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

  /*
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
  */

  if (m_currentState == State::MOVING_SELECTION)
  {
    if (eventType == QEvent::MouseMove)
    {
      continueMovingSelection(mousePos);
      return;
    }
    if (eventType == QEvent::MouseButtonRelease)
    {
      setCurrentState(State::SELECTED);
      setPreviousTool();
    }
  }

  if (m_currentState == State::ROTATING_SELECTION)
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

  if (m_currentState == State::RESIZING_SELECTION)
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

  if (m_currentState == State::SELECTING)
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

  if (m_currentState == State::DRAWING)
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

  if (m_currentState == State::RULING)
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

  if (m_currentState == State::CIRCLING)
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

  if (m_currentState == State::RECTING)
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

  if (m_currentState == State::IDLE)
  {
    if (eventType == QEvent::MouseButtonPress)
    {
      if (pointerType == QTabletEvent::Pen)
      {
        if (m_currentTool == Tool::PEN)
        {
          startDrawing(mousePos, pressure);
          return;
        }
        if (m_currentTool == Tool::RULER)
        {
          startRuling(mousePos);
          return;
        }
        if (m_currentTool == Tool::CIRCLE)
        {
          startCircling(mousePos);
          return;
        }
        if (m_currentTool == Tool::RECT)
        {
          startRecting(mousePos);
          return;
        }
        if (m_currentTool == Tool::ERASER)
        {
          m_undoStack.beginMacro("erase");
          erase(mousePos, invertEraser);
          return;
        }
        if (m_currentTool == Tool::STROKE_ERASER)
        {
          m_undoStack.beginMacro("erase");
          erase(mousePos, !invertEraser);
          return;
        }
        if (m_currentTool == Tool::SELECT || m_currentTool == Tool::RECT_SELECT)
        {
          startSelecting(mousePos);
          return;
        }
        if (m_currentTool == Tool::HAND)
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
      if (pointerType == QTabletEvent::Eraser || m_currentTool == Tool::ERASER)
      {
        erase(mousePos, invertEraser);
      }
      if (m_currentTool == Tool::STROKE_ERASER)
      {
        erase(mousePos, !invertEraser);
      }
      if (m_currentTool == Tool::HAND)
      {
        int dx = 1 * static_cast<int>(mousePos.x() - m_previousMousePos.x());
        int dy = 1 * static_cast<int>(mousePos.y() - m_previousMousePos.y());

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
  //if (inputEnabled())
  {
    event->accept();
    QPointF mousePos = QPointF(event->hiResGlobalX(), event->hiResGlobalY()) - mapToGlobal(QPoint(0, 0));
    qreal pressure = event->pressure();

    QEvent::Type eventType;
    switch (event->type())
    {
    case QTabletEvent::TabletPress:
      eventType = QEvent::MouseButtonPress;
      m_penDown = true;
      break;
    case QTabletEvent::TabletMove:
      eventType = QEvent::MouseMove;
      m_penDown = true;
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
  //else
  //{
  //  event->ignore();
  //}
}

void Widget::mousePressEvent(QMouseEvent *event)
{
  //if (inputEnabled())
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
}

void Widget::mouseMoveEvent(QMouseEvent *event)
{
  //if (inputEnabled())
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
}

void Widget::mouseReleaseEvent(QMouseEvent *event)
{
  //if (inputEnabled())
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
}

void Widget::mouseDoubleClickEvent(QMouseEvent* event)
{
  (void) event;
}

bool Widget::event(QEvent* p_event)
{
  if (p_event->type() == QEvent::TouchBegin)
  {
    qDebug() << "TouchBegin";
    p_event->accept();
    return true;
  }
  else if (p_event->type() == QEvent::TouchUpdate || p_event->type() == QEvent::TouchEnd)
  {
    qDebug() << "TouchUpdate / TouchEnd";
    auto touchEvent = static_cast<QTouchEvent*>(p_event);
    auto touchPoints =  touchEvent->touchPoints();
    if (touchPoints.size() == 1)
    {
      auto pos = touchPoints.at(0).pos();
      auto lastPos = touchPoints.at(0).lastPos();
      auto dx = static_cast<int>(pos.x() - lastPos.x());
      auto dy = static_cast<int>(pos.y() - lastPos.y());
      m_scrollArea->horizontalScrollBar()->setValue(m_scrollArea->horizontalScrollBar()->value() - dx);
      m_scrollArea->verticalScrollBar()->setValue(m_scrollArea->verticalScrollBar()->value() - dy);
    }

    p_event->accept();
    return true;
  }
  //else if (p_event->type() == QEvent::TouchEnd)
  //{
  //  qDebug() << "TouchEnd";
  //  p_event->accept();
  //  return true;
  //}
  else
  {
    return QWidget::event(p_event);
  }
}

void Widget::setPreviousTool()
{
  if (m_previousTool == Tool::PEN)
  {
    emit pen();
  }
  if (m_previousTool == Tool::RULER)
  {
    emit ruler();
  }
  if (m_previousTool == Tool::CIRCLE)
  {
    emit circle();
  }
  if (m_previousTool == Tool::RECT)
  {
    emit rect();
  }
  if (m_previousTool == Tool::ERASER)
  {
    emit eraser();
  }
  if (m_previousTool == Tool::STROKE_ERASER)
  {
    emit strokeEraser();
  }
  if (m_previousTool == Tool::SELECT)
  {
    emit select();
  }
  if (m_previousTool == Tool::RECT_SELECT)
  {
    emit rectSelect();
  }
  if (m_previousTool == Tool::HAND)
  {
    emit hand();
  }

  m_previousTool = Tool::NONE;
}

void Widget::startSelecting(QPointF mousePos)
{
  size_t pageNum = getPageFromMousePos(mousePos);
  QPointF pagePos = getPagePosFromMousePos(mousePos, pageNum);

  MrDoc::Selection newSelection;


  newSelection.setPageNum(pageNum);
  newSelection.setWidth(m_currentDocument.pages[pageNum].width());
  newSelection.setHeight(m_currentDocument.pages[pageNum].height());
  newSelection.appendToSelectionPolygon(pagePos);

  m_firstMousePos = mousePos;

  m_currentSelection = newSelection;

  //    selecting = true;
  m_currentState = State::SELECTING;
  m_selectingOnPage = pageNum;
}

void Widget::continueSelecting(QPointF mousePos)
{
  size_t pageNum = m_selectingOnPage;
  QPointF pagePos = getPagePosFromMousePos(mousePos, pageNum);
  QPointF firstPagePos = getPagePosFromMousePos(m_firstMousePos, pageNum);

  if (m_currentTool == Widget::Tool::SELECT)
  {
    m_currentSelection.appendToSelectionPolygon(pagePos);
  }
  else if (m_currentTool == Widget::Tool::RECT_SELECT)
  {
    QPolygonF selectionPolygon;
    if ( ( pagePos.x() > firstPagePos.x() && pagePos.y() > firstPagePos.y() ) || ( pagePos.x() < firstPagePos.x() && pagePos.y() > firstPagePos.y() ) )
    {
      selectionPolygon.append(QPointF(firstPagePos.x(), firstPagePos.y()));
      selectionPolygon.append(QPointF(firstPagePos.x(), pagePos.y()));
      selectionPolygon.append(QPointF(pagePos.x(), pagePos.y()));
      selectionPolygon.append(QPointF(pagePos.x(), firstPagePos.y()));
    }
    else
    {
      selectionPolygon.append(QPointF(firstPagePos.x(), firstPagePos.y()));
      selectionPolygon.append(QPointF(pagePos.x(), firstPagePos.y()));
      selectionPolygon.append(QPointF(pagePos.x(), pagePos.y()));
      selectionPolygon.append(QPointF(firstPagePos.x(), pagePos.y()));
    }
    m_currentSelection.setSelectionPolygon(selectionPolygon);
  }

  update();

  if (MrWriter::polygonIsClockwise(m_currentSelection.selectionPolygon()))
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
  size_t pageNum = m_selectingOnPage;

  //continueSelecting(mousePos);

  if (m_currentSelection.selectionPolygon().length() < 3) // && currentTool == Widget::tool::SELECT)
  {
    double s = 10.0;
    QPolygonF selectionPolygon;
    selectionPolygon.append(getPagePosFromMousePos(mousePos + QPointF(-s, -s), pageNum));
    selectionPolygon.append(getPagePosFromMousePos(mousePos + QPointF( s, -s), pageNum));
    selectionPolygon.append(getPagePosFromMousePos(mousePos + QPointF( s,  s), pageNum));
    selectionPolygon.append(getPagePosFromMousePos(mousePos + QPointF(-s,  s), pageNum));
    m_currentSelection.setSelectionPolygon(selectionPolygon);
  }

  if (!m_currentDocument.pages[pageNum].getElements(m_currentSelection.selectionPolygon()).empty())
  {
    CreateSelectionCommand *createSelectionCommand = new CreateSelectionCommand(this, pageNum, m_currentSelection);
    m_undoStack.push(createSelectionCommand);

    emit updateGUI();
    update();
  }
  else
  {
    setCurrentState(State::IDLE);
  }
  m_statusText.clear();
  updateGUI();
}

void Widget::letGoSelection()
{
  if (currentState() == State::SELECTED)
  {
    size_t pageNum = m_currentSelection.pageNum();
    ReleaseSelectionCommand *releaseCommand = new ReleaseSelectionCommand(this, pageNum);
    m_undoStack.push(releaseCommand);
    updateAllDirtyBuffers();
    setCurrentState(State::IDLE);
  }
}

QPointF Widget::pagePosToGrid(QPointF pagePos)
{
  QPointF pagePosOnGrid;
  pagePosOnGrid.setX(round(pagePos.x() / (m_gridWidth / 2.0)) * (m_gridWidth / 2.0));
  pagePosOnGrid.setY(round(pagePos.y() / (m_gridWidth / 2.0)) * (m_gridWidth / 2.0));
  return pagePosOnGrid;
}

void Widget::startRuling(QPointF mousePos)
{
  m_currentDocument.setDocumentChanged(true);
  emit modified();

  size_t pageNum = getPageFromMousePos(mousePos);
  QPointF pagePos = getPagePosFromMousePos(mousePos, pageNum);

  if (m_snapToGrid)
  {
    pagePos = pagePosToGrid(pagePos);
  }

  MrDoc::Stroke newStroke;
  newStroke.pattern = m_currentPattern;
  newStroke.points.append(pagePos);
  newStroke.pressures.append(1);
  newStroke.penWidth = m_currentPenWidth;
  newStroke.color = m_currentColor;
  m_currentStroke = newStroke;
  m_currentState = State::RULING;

  m_previousMousePos = mousePos;
  m_firstMousePos = mousePos;
  m_drawingOnPage = pageNum;
}

void Widget::continueRuling(QPointF mousePos)
{
  QPointF pagePos = getPagePosFromMousePos(mousePos, m_drawingOnPage);
  QPointF previousPagePos = getPagePosFromMousePos(m_previousMousePos, m_drawingOnPage);

  QPointF firstPagePos = m_currentStroke.points.at(0);

  if (m_snapToGrid)
  {
    pagePos = pagePosToGrid(pagePos);
    previousPagePos = pagePosToGrid(previousPagePos);
  }

  QPointF oldPagePos = pagePos;

  m_currentDashOffset = 0.0;

  if (m_currentStroke.points.length() > 1)
  {
    oldPagePos = m_currentStroke.points.at(1);
    m_currentStroke.points.removeAt(1);
    m_currentStroke.pressures.removeAt(1);
  }

  m_currentStroke.points.append(pagePos);
  m_currentStroke.pressures.append(1);

  QRect clipRect(m_zoom * firstPagePos.toPoint(), m_zoom * pagePos.toPoint());
  QRect oldClipRect(m_zoom * firstPagePos.toPoint(), m_zoom * previousPagePos.toPoint());
  clipRect = clipRect.normalized().united(oldClipRect.normalized());
  int clipRad = static_cast<int>(m_zoom * m_currentPenWidth / 2.0) + 2;
  clipRect = clipRect.normalized().adjusted(-clipRad, -clipRad, clipRad, clipRad);
  updateBufferRegion(m_drawingOnPage, clipRect);
  drawOnBuffer();

  update();

  m_previousMousePos = mousePos;
}

void Widget::stopRuling(QPointF mousePos)
{
  QPointF pagePos = getPagePosFromMousePos(mousePos, m_drawingOnPage);

  if (m_snapToGrid)
  {
    pagePos = pagePosToGrid(pagePos);
  }

  if (m_currentStroke.points.length() > 1)
  {
    m_currentStroke.points.removeAt(1);
    m_currentStroke.pressures.removeAt(1);
  }

  m_currentStroke.points.append(pagePos);
  m_currentStroke.pressures.append(1);

  AddElementCommand *addCommand = new AddElementCommand(this, m_drawingOnPage, m_currentStroke.clone());
  m_undoStack.push(addCommand);

  m_currentState = State::IDLE;

  QTransform scaleTrans;
  scaleTrans = scaleTrans.scale(m_zoom, m_zoom);

  QRect clipRect = scaleTrans.mapRect(m_currentStroke.points.boundingRect()).toRect();
  int clipRad = static_cast<int>(m_zoom * m_currentPenWidth / 2) + 2;
  clipRect = clipRect.normalized().adjusted(-clipRad, -clipRad, clipRad, clipRad);
  updateBufferRegion(m_drawingOnPage, clipRect);
  update();
}

void Widget::startCircling(QPointF mousePos)
{
  m_currentDocument.setDocumentChanged(true);
  emit modified();

  size_t pageNum = getPageFromMousePos(mousePos);

  MrDoc::Stroke newStroke;
  newStroke.pattern = m_currentPattern;
  newStroke.penWidth = m_currentPenWidth;
  newStroke.color = m_currentColor;
  m_currentStroke = newStroke;
  m_currentState = State::CIRCLING;

  m_previousMousePos = mousePos;
  m_firstMousePos = mousePos;
  m_drawingOnPage = pageNum;
}

void Widget::continueCircling(QPointF mousePos)
{
  QPointF pagePos = getPagePosFromMousePos(mousePos, m_drawingOnPage);
  QPointF firstPagePos = getPagePosFromMousePos(m_firstMousePos, m_drawingOnPage);

  if (m_snapToGrid)
  {
    pagePos = pagePosToGrid(pagePos);
    firstPagePos = pagePosToGrid(firstPagePos);
  }

  m_currentDashOffset = 0.0;

  MrDoc::Stroke oldStroke = m_currentStroke;

  m_currentStroke.points.clear();
  m_currentStroke.pressures.clear();

  qreal radius = QLineF(firstPagePos, pagePos).length();
  qreal phi0 = QLineF(firstPagePos, pagePos).angle() * M_PI / 180.0;

  int N = 100;
  for (int i = 0; i <= N; ++i)
  {
    qreal phi = phi0 + i * (2.0 * M_PI / N);
    qreal x = firstPagePos.x() + radius * cos(phi);
    qreal y = firstPagePos.y() - radius * sin(phi);
    m_currentStroke.points.append(QPointF(x, y));
    m_currentStroke.pressures.append(1.0);
  }

  QTransform scaleTrans;
  scaleTrans = scaleTrans.scale(m_zoom, m_zoom);

  QRect clipRect = scaleTrans.mapRect(m_currentStroke.points.boundingRect()).toRect();
  QRect oldClipRect = scaleTrans.mapRect(oldStroke.points.boundingRect()).toRect();
  clipRect = clipRect.normalized().united(oldClipRect.normalized());
  int clipRad = static_cast<int>(m_zoom * m_currentPenWidth / 2) + 2;
  clipRect = clipRect.normalized().adjusted(-clipRad, -clipRad, clipRad, clipRad);
  updateBufferRegion(m_drawingOnPage, clipRect);

  drawOnBuffer();

  update();

  m_previousMousePos = mousePos;
}

void Widget::stopCircling(QPointF mousePos)
{
  continueCircling(mousePos);

  AddElementCommand *addCommand = new AddElementCommand(this, m_drawingOnPage, m_currentStroke.clone());
  m_undoStack.push(addCommand);

  m_currentState = State::IDLE;

  update();
}

void Widget::startRecting(QPointF mousePos)
{
  m_currentDocument.setDocumentChanged(true);
  emit modified();

  size_t pageNum = getPageFromMousePos(mousePos);

  MrDoc::Stroke newStroke;
  newStroke.pattern = m_currentPattern;
  newStroke.penWidth = m_currentPenWidth;
  newStroke.color = m_currentColor;
  m_currentStroke = newStroke;
  m_currentState = State::RECTING;

  m_previousMousePos = mousePos;
  m_firstMousePos = mousePos;
  m_drawingOnPage = pageNum;
}

void Widget::continueRecting(QPointF mousePos)
{
  QPointF pagePos = getPagePosFromMousePos(mousePos, m_drawingOnPage);
  QPointF firstPagePos = getPagePosFromMousePos(m_firstMousePos, m_drawingOnPage);

  if (m_snapToGrid)
  {
    pagePos = pagePosToGrid(pagePos);
    firstPagePos = pagePosToGrid(firstPagePos);
  }

  m_currentDashOffset = 0.0;

  MrDoc::Stroke oldStroke = m_currentStroke;

  m_currentStroke.points.clear();
  m_currentStroke.pressures.clear();

  m_currentStroke.points.append(QPointF(firstPagePos.x(), firstPagePos.y()));
  m_currentStroke.points.append(QPointF(firstPagePos.x(), pagePos.y()));
  m_currentStroke.points.append(QPointF(pagePos.x(), pagePos.y()));
  m_currentStroke.points.append(QPointF(pagePos.x(), firstPagePos.y()));
  m_currentStroke.points.append(QPointF(firstPagePos.x(), firstPagePos.y()));

  for (int i = 0; i < 5; i++)
  {
    m_currentStroke.pressures.append(1.0);
  }

  QTransform scaleTrans;
  scaleTrans = scaleTrans.scale(m_zoom, m_zoom);

  QRect clipRect = scaleTrans.mapRect(m_currentStroke.points.boundingRect()).toRect();
  QRect oldClipRect = scaleTrans.mapRect(oldStroke.points.boundingRect()).toRect();
  clipRect = clipRect.normalized().united(oldClipRect.normalized());
  int clipRad = static_cast<int>(m_zoom * m_currentPenWidth / 2) + 2;
  clipRect = clipRect.normalized().adjusted(-clipRad, -clipRad, clipRad, clipRad);
  updateBufferRegion(m_drawingOnPage, clipRect);

  drawOnBuffer();

  update();

  m_previousMousePos = mousePos;
}

void Widget::stopRecting(QPointF mousePos)
{
  continueRecting(mousePos);

  AddElementCommand *addCommand = new AddElementCommand(this, m_drawingOnPage, m_currentStroke.clone());
  m_undoStack.push(addCommand);

  m_currentState = State::IDLE;

  update();
}

void Widget::startTexting(QPointF mousePos)
{
  disableInput();

  setCurrentState(State::TEXTING);

  m_firstMousePos = mousePos;

  size_t pageNum = getPageFromMousePos(mousePos);
  QPointF pagePos = getPagePosFromMousePos(m_firstMousePos, pageNum) - QPointF(0.0, m_textEdit->font().pointSizeF() / 2.0 / m_zoom);

  QTransform myTrans;
  myTrans = myTrans.translate(pagePos.x(), pagePos.y());
  m_currentText.m_color = m_currentColor;
  m_currentText.m_transform = myTrans;

  m_currentText.m_font = m_currentFont;

  QFont font = m_currentFont;
  font.setPointSizeF(font.pointSizeF() * m_zoom);
  m_textEdit->setGeometry(static_cast<int>(mousePos.x()), static_cast<int>(mousePos.y() - font.pointSizeF() / 2.0), 100, 100);
  qDebug() << "Word spacing:   " << font.wordSpacing();
  qDebug() << "Letter spacing: " << font.letterSpacing();
  m_textEdit->setFont(font);
  m_textEdit->setPlainText((""));
  m_textEdit->setFocus();
  m_textEdit->show();
}


void Widget::stopTexting(QPointF mousePos)
{
  qDebug() << Q_FUNC_INFO;
  size_t pageNum = getPageFromMousePos(mousePos);
  QPointF pagePos = getPagePosFromMousePos(m_firstMousePos, pageNum) - QPointF(0.0, m_textEdit->font().pointSizeF() / 2.0 / m_zoom);

  m_currentText.m_text = m_textEdit->toPlainText();

  if (m_currentText.m_text.size() != 0)
  {
    auto addElem = new AddElementCommand(this, pageNum, m_currentText.clone());
    m_undoStack.push(addElem);
    update();
    updateGUI();

    m_currentDocument.setDocumentChanged(true);
    emit modified();
  }

  m_undoStack.endMacro();

  m_textEdit->hide();
  setCurrentState(State::IDLE);
  enableInput();
}

void Widget::startEditingText(QPointF mousePos, size_t index)
{
  disableInput();

  setCurrentState(State::TEXTING);

  size_t pageNum = getPageFromMousePos(mousePos);
  auto text = dynamic_cast<MrDoc::Text*>(m_currentDocument.pages.at(pageNum).elements().at(index).get());
  if (!text)
  {
    qDebug() << "nullptr " << Q_FUNC_INFO;
    return;
  }
  m_currentText.m_font = text->m_font;
  m_currentText.m_text = text->m_text;
  m_currentText.m_color = text->m_color;
  m_currentText.m_transform = text->m_transform;
  text = nullptr;

  m_undoStack.beginMacro("Edit text");
  auto remElem = new RemoveElementCommand(this, pageNum, index);
  m_undoStack.push(remElem);

  mousePos = getMousePosFromPagePos(QPointF(m_currentText.m_transform.m31(), m_currentText.m_transform.m32()), pageNum);

  m_textEdit->setGeometry(static_cast<int>(mousePos.x()), static_cast<int>(mousePos.y()), 100, 100);
  QFont font = m_currentText.m_font;
  font.setPointSizeF(font.pointSizeF() * m_zoom);
  //textEdit->document()->setDefaultFont(font);
  m_textEdit->setFont(font);
  m_textEdit->setPlainText(m_currentText.m_text);
  m_textEdit->setFocus();
  m_textEdit->show();

}

void Widget::startDrawing(QPointF mousePos, qreal pressure)
{
  m_updateTimer->start(33); // 33 -> 30 fps

  m_currentDocument.setDocumentChanged(true);
  emit modified();

  m_currentUpdateRect = QRect();

  size_t pageNum = getPageFromMousePos(mousePos);
  QPointF pagePos = getPagePosFromMousePos(mousePos, pageNum);

  m_currentDashOffset = 0.0;

  MrDoc::Stroke newStroke;
  newStroke.pattern = m_currentPattern;
  newStroke.points.append(pagePos);
  newStroke.pressures.append(pressure);
  newStroke.penWidth = m_currentPenWidth;
  newStroke.color = m_currentColor;
  m_currentStroke = newStroke;
  m_currentState = State::DRAWING;

  m_previousMousePos = mousePos;
  m_drawingOnPage = pageNum;
}

void Widget::continueDrawing(QPointF mousePos, qreal pressure)
{
  QPointF pagePos = getPagePosFromMousePos(mousePos, m_drawingOnPage);

  m_currentStroke.points.append(pagePos);
  m_currentStroke.pressures.append(pressure);
  drawOnBuffer(true);

  QRect updateRect(m_previousMousePos.toPoint(), mousePos.toPoint());
  int rad = static_cast<int>(m_currentPenWidth * m_zoom / 2.0) + 2;
  updateRect = updateRect.normalized().adjusted(-rad, -rad, +rad, +rad);

  m_currentUpdateRect = m_currentUpdateRect.united(updateRect);

  m_previousMousePos = mousePos;
}

void Widget::stopDrawing(QPointF mousePos, qreal pressure)
{
  m_updateTimer->stop();

  QPointF pagePos = getPagePosFromMousePos(mousePos, m_drawingOnPage);

  m_currentStroke.points.append(pagePos);
  m_currentStroke.pressures.append(pressure);
  m_currentStroke.finalize();

  drawOnBuffer();

  AddElementCommand *addCommand = new AddElementCommand(this, m_drawingOnPage, m_currentStroke.clone(), true, 0, false, true);
  m_undoStack.push(addCommand);

  //  currentState = state::IDLE;
  setCurrentState(State::IDLE);

  update();
}

void Widget::erase(QPointF mousePos, bool lineEraser)
{
  size_t pageNum = getPageFromMousePos(mousePos);
  QPointF pagePos = getPagePosFromMousePos(mousePos, pageNum);

  const std::vector<std::unique_ptr<MrDoc::Element>> &elements = m_currentDocument.pages[pageNum].elements();

  qreal eraserWidth = 10 * devicePixelRatio();

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
                m_undoStack.push(removeElementCommand);
                AddElementCommand *addElementCommand = new AddElementCommand(this, pageNum, splitStrokeB.clone(), false, i, false, false);
                m_undoStack.push(addElementCommand);
                addElementCommand = new AddElementCommand(this, pageNum, splitStrokeA.clone(), false, i, false, false);
                m_undoStack.push(addElementCommand);
                i += 2;
                break;
              }
            }
          }
        }
      }
    }
  }

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
    m_currentDocument.setDocumentChanged(true);
    emit modified();

    std::sort(strokesToDelete.begin(), strokesToDelete.end(), std::greater<int>());
    for (size_t i = 0; i < strokesToDelete.size(); ++i)
    {
      RemoveElementCommand *removeCommand = new RemoveElementCommand(this, pageNum, strokesToDelete[i]);
      m_undoStack.push(removeCommand);
    }
  }
  updateAllDirtyBuffers();
}

void Widget::startMovingSelection(QPointF mousePos)
{
  m_currentDocument.setDocumentChanged(true);
  emit modified();

  size_t pageNum = getPageFromMousePos(mousePos);
  m_previousPagePos = getPagePosFromMousePos(mousePos, pageNum);
  if (m_snapToGrid)
  {
    m_previousPagePos = pagePosToGrid(m_previousPagePos);
  }

  setCurrentState(State::MOVING_SELECTION);
}

void Widget::continueMovingSelection(QPointF mousePos)
{
  size_t pageNum = getPageFromMousePos(mousePos);
  QPointF pagePos = getPagePosFromMousePos(mousePos, pageNum);

  if (m_snapToGrid)
  {
    pagePos = pagePosToGrid(pagePos);
  }

  QPointF delta = (pagePos - m_previousPagePos);

  QTransform transform;
  transform.translate(delta.x(), delta.y());

  TransformSelectionCommand *transSelectCommand = new TransformSelectionCommand(this, pageNum, transform);
  m_undoStack.push(transSelectCommand);

  m_previousPagePos = pagePos;
}

void Widget::startRotatingSelection(QPointF mousePos)
{
  m_currentDocument.setDocumentChanged(true);
  emit modified();

  m_currentAngle = 0.0;

  size_t pageNum = m_currentSelection.pageNum();
  m_previousPagePos = getPagePosFromMousePos(mousePos, pageNum);
  setCurrentState(State::ROTATING_SELECTION);
}

void Widget::continueRotatingSelection(QPointF mousePos)
{
  size_t pageNum = m_currentSelection.pageNum();
  QPointF pagePos = getPagePosFromMousePos(mousePos, pageNum);

  m_currentAngle = QLineF(m_currentSelection.boundingRect().center(), pagePos).angleTo(QLineF(m_currentSelection.boundingRect().center(), m_previousPagePos));
  if (m_snapToGrid)
  {
    m_currentAngle = floor(m_currentAngle / 5.0) * 5.0;
  }
  m_currentSelection.setAngle(m_currentAngle);
  m_statusText = QString("%1°").arg(m_currentAngle > 180.0 ? 360.0 - m_currentAngle : - m_currentAngle);
  updateGUI();
}

void Widget::stopRotatingSelection(QPointF mousePos)
{
  continueRotatingSelection(mousePos);
  size_t pageNum = m_currentSelection.pageNum();

  QTransform transform;
  transform.translate(m_currentSelection.boundingRect().center().x(), m_currentSelection.boundingRect().center().y());
  transform.rotate(m_currentAngle);
  transform.translate(-m_currentSelection.boundingRect().center().x(), -m_currentSelection.boundingRect().center().y());

  m_currentSelection.setAngle(0.0);

  TransformSelectionCommand *transSelectCommand = new TransformSelectionCommand(this, pageNum, transform);
  m_undoStack.push(transSelectCommand);

  m_currentSelection.finalize();
  m_currentSelection.updateBuffer(m_zoom);
  setCurrentState(State::SELECTED);
  m_statusText = "";
  updateGUI();
}

void Widget::startResizingSelection(QPointF mousePos, MrDoc::Selection::GrabZone grabZone)
{
  m_currentDocument.setDocumentChanged(true);
  emit modified();

  m_grabZone = grabZone;

  size_t pageNum = m_currentSelection.pageNum();
  m_previousPagePos = getPagePosFromMousePos(mousePos, pageNum);
  setCurrentState(State::RESIZING_SELECTION);
}

void Widget::continueResizingSelection(QPointF mousePos)
{
  size_t pageNum = m_currentSelection.pageNum();
  QPointF pagePos = getPagePosFromMousePos(mousePos, pageNum);
  QPointF delta = (pagePos - m_previousPagePos);

  QTransform transform;

  qreal moveX = m_currentSelection.boundingRect().x();
  qreal moveY = m_currentSelection.boundingRect().y();

  qreal sx = 0.0;
  qreal sy = 0.0;

  qreal moveBackX = 0.0;
  qreal moveBackY = 0.0;

  using GrabZone = MrDoc::Selection::GrabZone;
  if (m_grabZone == GrabZone::Top)
  {
    sx = 1.0;
    sy = (m_currentSelection.boundingRect().height() - delta.y()) / m_currentSelection.boundingRect().height();
    moveBackX = -moveX;
    moveBackY = -moveY + delta.y() / sy;
  }
  else if (m_grabZone == GrabZone::Bottom)
  {
    sx = 1.0;
    sy = (m_currentSelection.boundingRect().height() + delta.y()) / m_currentSelection.boundingRect().height();
    moveBackX = -moveX;
    moveBackY = -moveY;
  }
  else if (m_grabZone == GrabZone::Left)
  {
    sx = (m_currentSelection.boundingRect().width() - delta.x()) / m_currentSelection.boundingRect().width();
    sy = 1.0;
    moveBackX = -moveX + delta.x() / sx;
    moveBackY = -moveY;
  }
  else if (m_grabZone == GrabZone::Right)
  {
    sx = (m_currentSelection.boundingRect().width() + delta.x()) / m_currentSelection.boundingRect().width();
    sy = 1.0;
    moveBackX = -moveX;
    moveBackY = -moveY;
  }
  else if (m_grabZone == GrabZone::TopLeft)
  {
    sx = (m_currentSelection.boundingRect().width() - delta.x()) / m_currentSelection.boundingRect().width();
    sy = (m_currentSelection.boundingRect().height() - delta.y()) / m_currentSelection.boundingRect().height();
    moveBackX = -moveX + delta.x() / sx;
    moveBackY = -moveY + delta.y() / sy;
  }
  else if (m_grabZone == GrabZone::TopRight)
  {
    sx = (m_currentSelection.boundingRect().width() + delta.x()) / m_currentSelection.boundingRect().width();
    sy = (m_currentSelection.boundingRect().height() - delta.y()) / m_currentSelection.boundingRect().height();
    moveBackX = -moveX;
    moveBackY = -moveY + delta.y() / sy;
  }
  else if (m_grabZone == GrabZone::BottomLeft)
  {
    sx = (m_currentSelection.boundingRect().width() - delta.x()) / m_currentSelection.boundingRect().width();
    sy = (m_currentSelection.boundingRect().height() + delta.y()) / m_currentSelection.boundingRect().height();
    moveBackX = -moveX + delta.x() / sx;
    moveBackY = -moveY;
  }
  else if (m_grabZone == GrabZone::BottomRight)
  {
    sx = (m_currentSelection.boundingRect().width() + delta.x()) / m_currentSelection.boundingRect().width();
    sy = (m_currentSelection.boundingRect().height() + delta.y()) / m_currentSelection.boundingRect().height();
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

  m_currentSelection.finalize();
  m_currentSelection.updateBuffer(m_zoom);
  setCurrentState(State::SELECTED);
}

size_t Widget::getPageFromMousePos(QPointF mousePos) const
{
  qreal y = mousePos.y(); // - currentCOSPos.y();
  size_t pageNum = 0;
  while (y > (floor(m_currentDocument.pages[pageNum].pixelHeight(m_zoom, 1))) + PAGE_GAP)
  {
    y -= (floor(m_currentDocument.pages[pageNum].pixelHeight(m_zoom, 1))) + PAGE_GAP;
    pageNum += 1;
    if (pageNum >= m_currentDocument.pages.size())
    {
      pageNum = m_currentDocument.pages.size() - 1;
      break;
    }
  }
  return pageNum;
}

size_t Widget::currentPage() const
{
  QPoint globalMousePos = parentWidget()->mapToGlobal(QPoint(0, 0)) + QPoint(parentWidget()->size().width() / 2, parentWidget()->size().height() / 2);
  QPoint pos = this->mapFromGlobal(globalMousePos);
  size_t pageNum = this->getPageFromMousePos(pos);

  return pageNum;
}

QPointF Widget::getPagePosFromMousePos(QPointF mousePos, size_t pageNum) const
{
  qreal x = mousePos.x();
  qreal y = mousePos.y();
  for (size_t i = 0; i < pageNum; ++i)
  {
    y -= (m_currentDocument.pages[pageNum].pixelHeight(m_zoom) + PAGE_GAP);
  }

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
    y += (m_currentDocument.pages[pageNum].pixelHeight(m_zoom, devicePixelRatio()) + PAGE_GAP);
  }
  y *= m_zoom;

  pagePos.setY(y + pagePos.y());

  return pagePos;
}

QPointF Widget::getMousePosFromPagePos(QPointF pagePos, size_t pageNum) const
{
  qreal y = 0.0;
  for (size_t i = 0; i < pageNum; ++i)
  {
    y += (m_currentDocument.pages[pageNum].pixelHeight(m_zoom) + PAGE_GAP);
  }
  QPointF mousePos(pagePos.x() * m_zoom, pagePos.y() * m_zoom + y);

  return mousePos;
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
  m_currentSelection.updateBuffer(m_zoom);
  setGeometry(getWidgetGeometry());

  int newHMax = m_scrollArea->horizontalScrollBar()->maximum();
  int newVMax = m_scrollArea->verticalScrollBar()->maximum();

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

  m_scrollArea->horizontalScrollBar()->setValue(newH);
  m_scrollArea->verticalScrollBar()->setValue(newV);

  updateAllPageBuffers();
  update();
}

void Widget::zoomFitWidth()
{
  size_t pageNum = currentPage();

  QSize widgetSize = this->parentWidget()->size();
  qreal newZoom = widgetSize.width() / m_currentDocument.pages[pageNum].width();

  zoomTo(newZoom);
}

void Widget::zoomFitHeight()
{
  size_t pageNum = currentPage();

  QSize widgetSize = this->parentWidget()->size();
  qreal newZoom = widgetSize.height() / m_currentDocument.pages[pageNum].height();

  zoomTo(newZoom);
}

void Widget::toggleGrid()
{
  m_showGrid = !m_showGrid;
  updateAllPageBuffers(true);
  update();
}

void Widget::toggleSnapToGrid()
{
  m_snapToGrid = !m_snapToGrid;
  update();
}

bool Widget::showingGrid()
{
  return m_showGrid;
}

bool Widget::snappingToGrid()
{
  return m_snapToGrid;
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
  size_t pageNum = currentPage();
  if (pageNum > 0) pageNum--;
  scrollDocumentToPageNum(pageNum);
}

void Widget::pageDown()
{
  size_t pageNum = currentPage();
  pageNum++;

  if (pageNum >= m_currentDocument.pages.size())
  {
    pageAddEnd();
  }

  scrollDocumentToPageNum(pageNum);
}

void Widget::pageAddBefore()
{
  size_t pageNum = currentPage();
  AddPageCommand *addPageCommand = new AddPageCommand(this, pageNum);
  m_undoStack.push(addPageCommand);
  setGeometry(getWidgetGeometry());
  update();
  m_currentDocument.setDocumentChanged(true);
  emit modified();
}

void Widget::pageAddAfter()
{
  size_t pageNum = currentPage() + 1;
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
    size_t pageNum = currentPage();
    RemovePageCommand *removePageCommand = new RemovePageCommand(this, pageNum);
    m_undoStack.push(removePageCommand);
    setGeometry(getWidgetGeometry());
    update();
    m_currentDocument.setDocumentChanged(true);
    emit modified();
  }
}

void Widget::scrollDocumentToPageNum(size_t pageNum)
{
  if (pageNum >= m_currentDocument.pages.size())
  {
    return; // page doesn't exist
  }
  qreal y = 0.0;
  for (size_t i = 0; i < pageNum; ++i)
  {
    y += (m_currentDocument.pages[i].height()) * m_zoom + PAGE_GAP;
  }

  m_scrollArea->verticalScrollBar()->setValue(static_cast<int>(y));
}

void Widget::setCurrentTool(Tool p_tool)
{
  if (m_currentState == State::IDLE || m_currentState == State::SELECTED || m_currentState == State::SELECTING)
  {
    if (p_tool == Tool::SELECT && m_currentState == State::SELECTED)
    {
      letGoSelection();
    }
    m_currentTool = p_tool;
    if (p_tool == Tool::PEN)
      setCursor(m_penCursor);
    if (p_tool == Tool::RULER)
      setCursor(m_rulerCursor);
    if (p_tool == Tool::CIRCLE)
      setCursor(m_circleCursor);
    if (p_tool == Tool::RECT)
      setCursor(m_penCursor); // todo rect cursor
    if (p_tool == Tool::TEXT)
      setCursor(Qt::CursorShape::IBeamCursor); // todo rect cursor
    if (p_tool == Tool::ERASER)
      setCursor(m_eraserCursor);
    if (p_tool == Tool::STROKE_ERASER)
      setCursor(m_strokeEraserCursor);
    if (p_tool == Tool::SELECT)
      setCursor(Qt::CrossCursor);
    if (p_tool == Tool::HAND)
      setCursor(Qt::OpenHandCursor);
  }
}

void Widget::setPenCursor(const QString& p_resourceName)
{
  if (m_currentState == State::IDLE || m_currentState == State::SELECTED)
  {
    // setup cursor
    QPixmap penCursorBitmap = QPixmap(p_resourceName);
    penCursorBitmap.setMask(penCursorBitmap.mask());
    m_penCursor = QCursor(penCursorBitmap, -1, -1);
    setCursor(m_penCursor);
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
  if (m_currentState == State::TEXTING)
  {
    return;
  }

  if (m_currentState == State::SELECTED)
  {
    letGoSelection();
  }

  size_t pageNum = currentPage();
  QRectF selectRect;
  for (auto &element : m_currentDocument.pages[pageNum].elements())
  {
    selectRect = selectRect.united(element->boundingRect());
  }
  QPolygonF selectionPolygon = QPolygonF(selectRect);

  MrDoc::Selection selection;
  selection.setPageNum(pageNum);
  selection.setSelectionPolygon(selectionPolygon);

  if (!m_currentDocument.pages[pageNum].getElements(selection.selectionPolygon()).empty())
  {
    m_currentSelection = selection;
    CreateSelectionCommand *createSelectionCommand = new CreateSelectionCommand(this, pageNum, selection);
    m_undoStack.push(createSelectionCommand);

    emit updateGUI();
    update();
  }
  else
  {
    setCurrentState(State::IDLE);
  }
}

void Widget::copy()
{
  if (m_currentState == State::SELECTED )
  {
    m_clipboard = m_currentSelection;

    QImage selectionBuffer = m_currentSelection.buffer();
    QImage clipboardImage = selectionBuffer;
    clipboardImage.fill(QColor(255,255,255));
    QPainter painter(&clipboardImage);
    painter.drawImage(0, 0, selectionBuffer);

    QClipboard *clipboard = QGuiApplication::clipboard();
    clipboard->setImage(clipboardImage);
  }
  update();
  emit updateGUI();
}

void Widget::paste()
{
  if (! m_clipboard.empty() && m_currentState != State::TEXTING)
  {
    MrDoc::Selection tmpSelection = m_clipboard;
    tmpSelection.setPageNum(currentPage());

    QPoint globalMousePos = parentWidget()->mapToGlobal(QPoint(0, 0)) + QPoint(parentWidget()->size().width() / 2, parentWidget()->size().height() / 2);
    QPoint mousePos = this->mapFromGlobal(globalMousePos);
    QPointF selectionPos = getPagePosFromMousePos(mousePos, currentPage()) - tmpSelection.boundingRect().center();

    if (m_snapToGrid)
    {
      selectionPos = pagePosToGrid(selectionPos);
    }

    QTransform myTrans;
    myTrans = myTrans.translate(selectionPos.x(), selectionPos.y());

    tmpSelection.transform(myTrans, tmpSelection.pageNum());

    tmpSelection.finalize();
    tmpSelection.updateBuffer(m_zoom);

    m_undoStack.beginMacro("Paste");
    if (m_currentState == State::SELECTED)
    {
      letGoSelection();
    }
    PasteCommand *pasteCommand = new PasteCommand(this, tmpSelection);
    m_undoStack.push(pasteCommand);
    m_undoStack.endMacro();

    m_currentDocument.setDocumentChanged(true);
    emit modified();
    emit updateGUI();
  }
}

void Widget::pasteImage()
{
  const QClipboard *clipboard = QApplication::clipboard();
  const QMimeData *mimeData = clipboard->mimeData();

  if (mimeData->hasImage())
  {
    MrDoc::Image image(QPointF(0,0));

    image.m_image = qvariant_cast<QImage>(mimeData->imageData());

    QTransform transform;
    transform.scale(0.5, 0.5);
    image.transform(transform);

    MrDoc::Selection selection;
    selection.appendElement(image.clone());
    selection.finalize();
    MrDoc::Selection prevClipboard = this->m_clipboard;
    this->m_clipboard = selection;
    paste();
    this->m_clipboard = prevClipboard;
  }
}

void Widget::pasteText()
{
  const QClipboard *clipboard = QApplication::clipboard();
  const QMimeData *mimeData = clipboard->mimeData();

  if (mimeData->hasText())
  {
    MrDoc::Text text;

    text.m_text = mimeData->text();

    MrDoc::Selection selection;
    selection.appendElement(text.clone());
    selection.finalize();
    MrDoc::Selection prevClipboard = this->m_clipboard;
    this->m_clipboard = selection;
    paste();
    this->m_clipboard = prevClipboard;
  }
}

void Widget::cut()
{
  CutCommand *cutCommand = new CutCommand(this);
  m_undoStack.push(cutCommand);
  emit updateGUI();
}

void Widget::deleteSlot()
{
  DeleteCommand *deleteCommand = new DeleteCommand(this);
  m_undoStack.push(deleteCommand);
  emit updateGUI();
}

void Widget::toTheBack()
{
  if (m_currentState == State::SELECTED)
  {
    auto toTheBackCommand = new ReleaseSelectionCommand(this, m_currentSelection.pageNum(), true);
    m_undoStack.push(toTheBackCommand);
    emit updateGUI();
  }
}

void Widget::undo()
{
  if (m_undoStack.canUndo() && (m_currentState == State::IDLE || m_currentState == State::SELECTED))
  {
    m_undoStack.undo();
    m_currentSelection.updateBuffer(m_zoom);
    updateAllDirtyBuffers();
  }
}

void Widget::redo()
{
  if (m_undoStack.canRedo() && (m_currentState == State::IDLE || m_currentState == State::SELECTED))
  {
    m_undoStack.redo();
    m_currentSelection.updateBuffer(m_zoom);
    updateAllDirtyBuffers();
  }
}

void Widget::setCurrentState(State newState)
{
  qDebug() << Q_FUNC_INFO << " newState: " << (int)newState;
  m_currentState = newState;
  if (m_currentState == State::IDLE || m_currentState == State::SELECTED)
  {
    updateAllDirtyBuffers();
    m_updateDirtyTimer->stop();
  }
  else
  {
    m_updateDirtyTimer->start();
  }
}

Widget::State Widget::currentState()
{
  return m_currentState;
}

void Widget::setCurrentFont(QFont font)
{
  m_currentFont = font;
  if (m_currentState == State::SELECTED)
  {
    auto changeFontCommand = new ChangeFontOfSelectionCommand(this, font);
    m_undoStack.push(changeFontCommand);
    m_currentSelection.finalize();
    m_currentSelection.updateBuffer(m_zoom);
    update();
  }
}

void Widget::setCurrentColor(QColor newColor)
{
  m_currentColor = newColor;
  if (m_currentState == State::SELECTED)
  {
    auto changeColorCommand = new ChangeColorOfSelectionCommand(this, newColor);
    m_undoStack.push(changeColorCommand);
    m_currentSelection.updateBuffer(m_zoom);
    update();
  }
}

QColor Widget::currentColor()
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

void Widget::setPencilCursorIcon()
{
  m_currentPenCursor = Widget::Cursor::PENCIL;
  setPenCursor(":/images/penCursor3.png");
  emit updateGUI();
}

void Widget::setDotCursorIcon()
{
  m_currentPenCursor = Widget::Cursor::DOT;
  setPenCursor(":/images/dotCursor.png");
  emit updateGUI();
}

void Widget::rotateSelection(qreal angle)
{
  QTransform rotateTrans;

  qreal dx = m_currentSelection.boundingRect().center().x();
  qreal dy = m_currentSelection.boundingRect().center().y();

  rotateTrans = rotateTrans.translate(dx, dy).rotate(-angle).translate(-dx, -dy);
  TransformSelectionCommand *transCommand = new TransformSelectionCommand(this, m_currentSelection.pageNum(), rotateTrans);
  m_undoStack.push(transCommand);
  m_currentSelection.finalize();
  m_currentSelection.updateBuffer(m_zoom);
  update();
}

void Widget::setCurrentPattern(QVector<qreal> newPattern)
{
  m_currentPattern = newPattern;
  emit updateGUI();
  if (m_currentState == State::SELECTED)
  {
    ChangePatternOfSelectionCommand *changePatternCommand = new ChangePatternOfSelectionCommand(this, newPattern);
    m_undoStack.push(changePatternCommand);
    m_currentSelection.updateBuffer(m_zoom);
    update();
  }
}

QVector<qreal> Widget::currentPattern()
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

void Widget::setCurrentPenWidth(qreal p_penWidth)
{
  m_currentPenWidth = p_penWidth;
  if (m_currentState == State::SELECTED)
  {
    ChangePenWidthOfSelectionCommand *changePenWidthCommand = new ChangePenWidthOfSelectionCommand(this, p_penWidth);
    m_undoStack.push(changePenWidthCommand);
    m_currentSelection.updateBuffer(m_zoom);
    update();
  }
}

void Widget::setCurrentPenCursor(Widget::Cursor p_cursor)
{
    switch (p_cursor) {
    case Widget::Cursor::PENCIL:
        setPencilCursorIcon();
        break;
    case Widget::Cursor::DOT:
        setDotCursorIcon();
        break;
    }
}

void Widget::dragEnterEvent(QDragEnterEvent* event)
{
  qDebug() << Q_FUNC_INFO;
  if (event->mimeData()->hasImage())
  {
    event->acceptProposedAction();
  }
  if (event->mimeData()->hasUrls() && event->mimeData()->urls().size() == 1)
  {
    QString filename = event->mimeData()->urls().at(0).toLocalFile();
    QFileInfo fileInfo(filename);
    auto supportedImageFormats = QImageReader::supportedImageFormats();
    auto suffix = fileInfo.suffix();
    bool imageFormatSupported = false;
    for (auto imageFormat : supportedImageFormats)
    {
      if (suffix.compare(imageFormat, Qt::CaseInsensitive) == 0)
      {
        imageFormatSupported = true;
      }
    }
    if (imageFormatSupported)
    {
      qDebug() << "Image format supported!";
      event->acceptProposedAction();
    }
  }
}


void Widget::dragMoveEvent(QDragMoveEvent* event)
{
}


void Widget::dragLeaveEvent(QDragLeaveEvent* event)
{
}


void Widget::dropEvent(QDropEvent* event)
{
  qDebug() << Q_FUNC_INFO;
  if (event->mimeData()->hasImage() || (event->mimeData()->hasUrls() && event->mimeData()->urls().size() == 1))
  {
    letGoSelection();

    auto mousePos = event->pos();
    auto pageNum = getPageFromMousePos(mousePos);
    auto pagePos = getPagePosFromMousePos(mousePos, pageNum);

    MrDoc::Image image(pagePos);

    if (event->mimeData()->hasImage())
    {
      image.m_image = qvariant_cast<QImage>(event->mimeData()->imageData());
    }

    if (event->mimeData()->hasUrls() && event->mimeData()->urls().size() == 1)
    {
      QString filename = event->mimeData()->urls().at(0).toLocalFile();
      image.m_image = QImage(filename);
    }

    // add image as selection
    MrDoc::Selection tmpSelection;
    tmpSelection.appendElement(image.clone());
    tmpSelection.setPageNum(pageNum);

    // center the selection
    pagePos -= QPointF(image.m_image.width(), image.m_image.height()) / 2.0;


    if (m_snapToGrid)
    {
      pagePos = pagePosToGrid(pagePos);
    }

    QTransform myTrans;
    myTrans.scale(0.5, 0.5);
    myTrans = myTrans.translate(pagePos.x(), pagePos.y());

    tmpSelection.transform(myTrans, tmpSelection.pageNum());

    tmpSelection.finalize();
    tmpSelection.updateBuffer(m_zoom);

    m_undoStack.beginMacro("Drop image");
    if (m_currentState == State::SELECTED)
    {
      letGoSelection();
    }
    PasteCommand *pasteCommand = new PasteCommand(this, tmpSelection);
    m_undoStack.push(pasteCommand);
    m_undoStack.endMacro();

    m_currentDocument.setDocumentChanged(true);
    emit modified();
    emit updateGUI();

    update();

    event->acceptProposedAction();
  }
}


