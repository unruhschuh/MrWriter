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
#include <QSettings>
#include <QTimer>
#include <qmath.h>

#define PAGE_GAP 10.0
#define ZOOM_STEP 1.2

#include <QPainter>
#include <QRectF>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
// Widget::Widget(QWidget *parent) : QOpenGLWidget(parent)
{
    textBox = new TextBox(this);
    connect(this, &Widget::setText, textBox, &TextBox::applyText);
    connect(textBox, &TextBox::updatePage, this, &Widget::updatePageAfterText);
    textBox->setWordWrapMode(QTextOption::WordWrap);
    textBox->setWindowFlags(Qt::SubWindow);
    QSizeGrip* sizeGrip = new QSizeGrip(textBox);
    QGridLayout* layout = new QGridLayout(textBox);
    layout->addWidget(sizeGrip, 0,0,1,1, Qt::AlignBottom|Qt::AlignRight);
    textBox->hide();
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

  currentTool = tool::PEN;
  previousTool = tool::NONE;
  realEraser = false;
  setCursor(penCursorBitmap);

  currentDocument = MrDoc::Document();

  currentPenWidth = 1.41;
  currentColor = QColor(0, 0, 0);
  zoom = 1.0;

  currentCOSPos.setX(0.0);
  currentCOSPos.setY(0.0);
  updateAllPageBuffers();
  setGeometry(getWidgetGeometry());

  parent->updateGeometry();
  parent->update();

  updateTimer = new QTimer(this);
  connect(updateTimer, SIGNAL(timeout()), this, SLOT(updateWhileDrawing()));

  updateDirtyTimer = new QTimer(this);
  connect(updateDirtyTimer, SIGNAL(timeout()), this, SLOT(updateAllDirtyBuffers()));
  updateDirtyTimer->setInterval(15);

  scrollTimer = new QTimer(this);
  connect(scrollTimer, &QTimer::timeout, this, &Widget::updatePageAfterScrollTimer);
  //scrollTimer->setInterval(30);
}

void Widget::updateAllPageBuffers()
{
  /*QVector<QFuture<void>> future;
  pageImageBuffer.clear();

  for (int buffNum = 0; buffNum < currentDocument.pages.size(); ++buffNum)
  {
    pageImageBuffer.append(QImage());
  }

  for (int buffNum = 0; buffNum < currentDocument.pages.size(); ++buffNum)
  {
    future.append(QtConcurrent::run(this, &Widget::updateImageBuffer, buffNum));
  }

  pageBuffer.clear();
  for (int buffNum = 0; buffNum < currentDocument.pages.size(); ++buffNum)
  {
    future[buffNum].waitForFinished();
    pageBuffer.append(QPixmap::fromImage(pageImageBuffer.at(buffNum)));

    // safe some memory
    pageImageBufferMutex.lock();
    pageImageBuffer[buffNum] = QImage();
    pageImageBufferMutex.unlock();
  }
  pageImageBuffer.clear();*/

  //new
  /*QVector<QFuture<void>> future;
  pageBuffer.clear();

  for (int buffNum = 0; buffNum < currentDocument.pages.size(); ++buffNum)
  {
    pageBuffer.append(QPixmap());
  }

  for (int buffNum = 0; buffNum < currentDocument.pages.size(); ++buffNum)
  {
    future.append(QtConcurrent::run(this, &Widget::updateBuffer, buffNum));
  }

  for (int buffNum = 0; buffNum < currentDocument.pages.size(); ++buffNum)
  {
    future[buffNum].waitForFinished();
    //pageBuffer.append(QPixmap::fromImage(pageImageBuffer.at(buffNum)));

    // safe some memory
    //pageImageBufferMutex.lock();
    //pageImageBuffer[buffNum] = QImage();
    //pageImageBufferMutex.unlock();
  }
  //pageImageBuffer.clear();

  */

    if(prevZoom != zoom || pageBufferPtr.size() != currentDocument.pages.size() || pageBufferPtr.isEmpty()){
        qDebug() << "All Pages";
        qDebug() << prevZoom << "vs" << zoom;

        QVector<QFuture<void>> future;
        pageBufferPtr.clear();

        for (int buffNum = 0; buffNum < currentDocument.pages.size(); ++buffNum)
        {
            pageBufferPtr.append(std::make_shared<QPixmap>());
        }

        for (int buffNum = 0; buffNum < currentDocument.pages.size(); ++buffNum)
        {
            future.append(QtConcurrent::run(this, &Widget::updateBuffer, buffNum));
        }

        for (int buffNum = 0; buffNum < currentDocument.pages.size(); ++buffNum)
        {
            future[buffNum].waitForFinished();
        }
        prevZoom = zoom;
    }
    else{
        updateNecessaryPagesBuffer();
    }


  /*pageBufferPtr.clear();
  for (int buffNum = 0; buffNum < currentDocument.pages.size(); ++buffNum)
  {
    pageBufferPtr.append(std::make_shared<QPixmap>());
  }
  for(int buffNum = 0; buffNum < currentDocument.pages.size(); ++buffNum){
      updateBuffer(buffNum);
  }*/
}

void Widget::updateNecessaryPagesBuffer(){
    qDebug() << "Only Necessary";
    QVector<QFuture<void>> future;

    int startingPage = std::max(0, getCurrentPage()-6);
    int endPage = std::min(currentDocument.pages.size(), getCurrentPage()+6);
    for(int buffNum = startingPage; buffNum < endPage; ++buffNum){
        pageBufferPtr.replace(buffNum, std::make_shared<QPixmap>());
    }
    for(int buffNum = startingPage; buffNum < endPage; ++buffNum){
        future.append(QtConcurrent::run(this, &Widget::updateBuffer, buffNum));
    }
    for(int buffNum = 0; buffNum < future.size(); ++buffNum){
        future[buffNum].waitForFinished();
    }
}

void Widget::updateImageBuffer(int buffNum)
{
    qDebug() << "updateImageBuffer";
  MrDoc::Page const &page = currentDocument.pages.at(buffNum);
  int pixelWidth = zoom * page.width() * devicePixelRatio();
  int pixelHeight = zoom * page.height() * devicePixelRatio();
  QImage image(pixelWidth, pixelHeight, QImage::Format_ARGB32_Premultiplied);
  image.setDevicePixelRatio(devicePixelRatio());

  image.fill(page.backgroundColor());

  QPainter painter;
  painter.begin(&image);
  painter.setRenderHint(QPainter::Antialiasing, true);

  if(abs(buffNum-getCurrentPage()) < 5)
    currentDocument.pages[buffNum].paint(painter, zoom);

  painter.end();

  pageImageBufferMutex.lock();
  pageImageBuffer.replace(buffNum, image);
  pageImageBufferMutex.unlock();
}

void Widget::updateBuffer(int buffNum)
{
    //qDebug() << "updateBuffer";

  /*MrDoc::Page const &page = currentDocument.pages.at(buffNum);
  int pixelWidth = zoom * page.width() * devicePixelRatio();
  int pixelHeight = zoom * page.height() * devicePixelRatio();
  QPixmap pixmap(pixelWidth, pixelHeight);
  pixmap.setDevicePixelRatio(devicePixelRatio());

  pixmap.fill(page.backgroundColor());

  QPainter painter;
  painter.begin(&pixmap);
  painter.setRenderHint(QPainter::Antialiasing, true);

  currentDocument.pages[buffNum].paint(painter, zoom);

  painter.end();

  pageBuffer.replace(buffNum, pixmap);*/

  MrDoc::Page const &page = currentDocument.pages.at(buffNum);
  int pixelWidth = zoom * page.width() * devicePixelRatio();
  int pixelHeight = zoom * page.height() * devicePixelRatio();

  //qDebug() << "visible Pages: " << getVisiblePages();
  if(abs(buffNum - getCurrentPage()) < 2*getVisiblePages()){
      std::shared_ptr<QPixmap> newPixmap = std::make_shared<QPixmap>(pixelWidth, pixelHeight);
      newPixmap->setDevicePixelRatio(devicePixelRatio());
      newPixmap->fill(page.backgroundColor());
      QPainter painter;
      painter.begin(newPixmap.get());
      painter.setRenderHint(QPainter::Antialiasing, true);

      currentDocument.pages[buffNum].paint(painter, zoom);

      painter.end();
      pageBufferPtr.replace(buffNum, newPixmap);
  }
  else{
      //TODO: use lockguard
      basePixmapMutex.lock();
      if(basePixmap->height() != pixelHeight && basePixmap->width() != pixelWidth){
          basePixmap = std::make_shared<QPixmap>(pixelWidth, pixelHeight);
      }
      basePixmap->setDevicePixelRatio(devicePixelRatio());
      basePixmap->fill(page.backgroundColor());

      QPainter painter;

      painter.begin(basePixmap.get());
      painter.end();
      pageBufferPtr.replace(buffNum, basePixmap);

      basePixmapMutex.unlock();
  }
}

void Widget::updateBufferRegion(int buffNum, QRectF const &clipRect)
{
    //qDebug() << "updateBufferRegion";
  /*QPainter painter;
  painter.begin(&pageBuffer[buffNum]);
  painter.setRenderHint(QPainter::Antialiasing, true);
  painter.setClipRect(clipRect);
  painter.setClipping(true);

  painter.fillRect(clipRect, currentDocument.pages.at(buffNum).backgroundColor());
  //    painter.fillRect(clipRect, Qt::red);

  QRectF paintRect = QRectF(clipRect.topLeft() / zoom, clipRect.bottomRight() / zoom);
  currentDocument.pages[buffNum].paint(painter, zoom, paintRect);

  painter.end();*/

  QPainter painter;
  painter.begin(pageBufferPtr[buffNum].get());
  painter.setRenderHint(QPainter::Antialiasing, true);
  painter.setClipRect(clipRect);
  painter.setClipping(true);

  painter.fillRect(clipRect, currentDocument.pages.at(buffNum).backgroundColor());

  QRectF paintRect = QRectF(clipRect.topLeft() / zoom, clipRect.bottomRight() / zoom);
  currentDocument.pages[buffNum].paint(painter, zoom, paintRect);

  painter.end();
}

void Widget::updateAllDirtyBuffers()
{
  for (int buffNum = 0; buffNum < currentDocument.pages.size(); ++buffNum)
  {
    QRectF const &dirtyRect = currentDocument.pages.at(buffNum).dirtyRect();
    if (!dirtyRect.isNull())
    {
      QRectF dirtyBufferRect = QRectF(dirtyRect.topLeft() * zoom, dirtyRect.bottomRight() * zoom);
      updateBufferRegion(buffNum, dirtyBufferRect);
      currentDocument.pages[buffNum].clearDirtyRect();
    }
  }
  update();
}

void Widget::updatePageAfterText(int i){
    updateBuffer(i);
    emit modified();
}

void Widget::updatePageAfterScrolling(int value){
    if(abs(value-previousValueRendered) > scrollArea->verticalScrollBar()->maximum()/currentDocument.pages.size()){
        //qDebug() << "updatePageAfterScrolling" << value;
        //QFuture<void> future(QtConcurrent::run(this, &Widget::updateAllPageBuffers));
        //updateAllPageBuffers();
        scrollTimer->start(15);
        previousValueMaybeRendered = value;
    }
    //scrollTimer->start(10*zoom);
}

void Widget::updatePageAfterScrollTimer(){
    qDebug() << "scrollTimer: " << scrollArea->verticalScrollBar()->isSliderDown();
    qDebug() << getCurrentPage();
    if(!scrollArea->verticalScrollBar()->isSliderDown()){
        updateAllPageBuffers();
        scrollTimer->stop();
        update();
        previousValueRendered = previousValueMaybeRendered;
    }
}

void Widget::drawOnBuffer(bool last)
{
    /*QPainter painter;
    painter.begin(&pageBuffer[drawingOnPage]);
    painter.setRenderHint(QPainter::Antialiasing, true);

    currentStroke.paint(painter, zoom, last);*/

    QPainter painter;
    painter.begin(pageBufferPtr[drawingOnPage].get());
    painter.setRenderHint(QPainter::Antialiasing, true);

    currentStroke.paint(painter, zoom, last);
}

QRect Widget::getWidgetGeometry()
{
  /*int width = 0;
  int height = 0;
  for (int i = 0; i < pageBuffer.size(); ++i)
  {
    height += pageBuffer[i].height()/devicePixelRatio() + PAGE_GAP;
    if (pageBuffer[i].width()/devicePixelRatio() > width)
      width = pageBuffer[i].width()/devicePixelRatio();
  }
  height -= PAGE_GAP;
  return QRect(0, 0, width, height);*/

  int width = 0;
  int height = 0;
  for (int i = 0; i < pageBufferPtr.size(); ++i)
  {
    height += pageBufferPtr[i]->height()/devicePixelRatio() + PAGE_GAP;
    if (pageBufferPtr[i]->width()/devicePixelRatio() > width)
      width = pageBufferPtr[i]->width()/devicePixelRatio();
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
        for (int i = 0; i < drawingOnPage; ++i)
        {
            trans = trans.translate(0, -(pageBufferPtr.at(i)->height() + PAGE_GAP*devicePixelRatio()));
        }
        trans = trans.scale(devicePixelRatio(),devicePixelRatio());
        rectSource = trans.mapRect(event->rect());

        //        QPixmap tmp = QPixmap::fromImage(pageBuffer.at(drawingOnPage));
        painter.drawPixmap(event->rect(), *pageBufferPtr[drawingOnPage], rectSource);

        //        painter.drawImage(event->rect(), pageBuffer.at(drawingOnPage), rectSource);
        return;
    }

    //    painter.setRenderHint(QPainter::Antialiasing, true);

    for (int i = 0; i < pageBufferPtr.size(); ++i)
    {
        QRectF rectSource;
        rectSource.setTopLeft(QPointF(0.0, 0.0));
        rectSource.setWidth(pageBufferPtr.at(i)->width());
        rectSource.setHeight(pageBufferPtr.at(i)->height());

        QRectF rectTarget;
        //        rectTarget.setTopLeft(QPointF(0.0, currentYPos));
        rectTarget.setTopLeft(QPointF(0.0, 0.0));
        rectTarget.setWidth(pageBufferPtr.at(i)->width()/devicePixelRatio());
        rectTarget.setHeight(pageBufferPtr.at(i)->height()/devicePixelRatio());

        painter.drawPixmap(rectTarget, *pageBufferPtr.at(i), rectSource);

        if ((currentState == state::SELECTING || currentState == state::SELECTED || currentState == state::MOVING_SELECTION ||
             currentState == state::RESIZING_SELECTION || currentState == state::ROTATING_SELECTION) &&
                i == currentSelection.pageNum())
        {
            currentSelection.paint(painter, zoom);
        }

        painter.translate(QPointF(0.0, rectSource.height()/devicePixelRatio() + PAGE_GAP));
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

  // bencmark
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
      int dx = 1 * (mousePos.x() - previousMousePos.x());
      int dy = 1 * (mousePos.y() - previousMousePos.y());

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
      GrabZone grabZone = currentSelection.grabZone(pagePos, zoom);
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
        int dx = 1 * (mousePos.x() - previousMousePos.x());
        int dy = 1 * (mousePos.y() - previousMousePos.y());

        scrollArea->horizontalScrollBar()->setValue(scrollArea->horizontalScrollBar()->value() - dx);
        scrollArea->verticalScrollBar()->setValue(scrollArea->verticalScrollBar()->value() - dy);

        mousePos -= QPointF(dx, dy);

        previousMousePos = mousePos;
      }
    }
    /*//Test for finger is moving the sheet
    if (eventType == QEvent::TouchUpdate){
        int dx = 1 * (mousePos.x() - previousMousePos.x());
        int dy = 1 * (mousePos.y() - previousMousePos.y());

        scrollArea->horizontalScrollBar()->setValue(scrollArea->horizontalScrollBar()->value() - dx);
        scrollArea->verticalScrollBar()->setValue(scrollArea->verticalScrollBar()->value() - dy);

        mousePos -= QPointF(dx, dy);

        previousMousePos = mousePos;
    }*/
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

  mouseAndTabletEvent(mousePos, event->button(), event->buttons(), keyboardModifiers, event->pointerType(), eventType, pressure, true);
}

void Widget::mousePressEvent(QMouseEvent *event)
{
    if(currentTool == tool::TEXT){
        QPointF point = getPagePosFromMousePos(event->pos(), getCurrentPage());
        int textIndex = currentDocument.pages[getCurrentPage()].textIndexFromMouseClick(point.x(), point.y());
        if(textBoxOpen){
            closeTextBox();
        }
        else if(textIndex != -1){ //clicked on text
            textBox->setPage(&(currentDocument.pages[getCurrentPage()]));
            textBox->setPageNum(getCurrentPage());
            textBox->setTextIndex(textIndex);
            textBox->setGeometry(currentDocument.pages[getCurrentPage()].textRectByIndex(textIndex).toAlignedRect().adjusted(0,0,50,50));
            textBox->setText(currentDocument.pages[getCurrentPage()].textByIndex(textIndex));
            //qDebug() << "Text clicked";
            textBox->setPrevText(textBox->toPlainText());
            textBox->setPrevColor(currentDocument.pages[getCurrentPage()].textColorByIndex(textIndex));
            textBox->setPrevFont(currentDocument.pages[getCurrentPage()].textFontByIndex(textIndex));
            textBox->show();
            textBoxOpen = true;
            textChanged = true;
            setCurrentColor(currentDocument.pages[getCurrentPage()].textColorByIndex(textIndex));
        }
        else if(textIndex == -1){ //clicked not on text
            textBox->setGeometry(event->x(), event->y(), 250,100);
            textBox->setPageNum(getCurrentPage());
            textBox->setPage(&(currentDocument.pages[getCurrentPage()]));
            textBox->setTextIndex(-1);
            textBox->setTextX(point.x());
            textBox->setTextY(point.y());
            textBox->setTextColor(getCurrentColor());
            textBox->setTextFont(QFont("Sans", 12));
            textBox->show();
            textBoxOpen = true;
        }
        setCurrentState(state::WRITING);
    }
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

void Widget::keyPressEvent(QKeyEvent *event){
    if(textBoxOpen){
        if(event->key() == Qt::Key_Escape){
            closeTextBox();
        }
        else{
            QWidget::keyPressEvent(event);
        }
    }
    else{
        QWidget::keyPressEvent(event);
    }
}

void Widget::closeTextBox(){
    if(textBoxOpen){
        textBox->setTextColor(getCurrentColor());
        if(textChanged){
            ChangeTextCommand* changeTextCommand = new ChangeTextCommand(textBox->getPage(), textBox->getTextIndex(), textBox->getPrevColor(),
                                                                         getCurrentColor(), textBox->getPrevFont(), textBox->getFont(),
                                                                         textBox->getPrevText(), textBox->toPlainText());
            undoStack.push(changeTextCommand);
            //update();
            textChanged = false;
            emit setText(true); //onlyHide
        }
        else{
            TextCommand* textCommand = new TextCommand(textBox->getPage(), QRectF(textBox->getTextX(), textBox->getTextY(), 0, 0), getCurrentColor(), textBox->getFont(), textBox->toPlainText());
            undoStack.push(textCommand);
            emit setText(true); //onlyHide, because the textSetting is done by textCommand
        }
        //emit setText(false); //text get's really set
        //updateBuffer(getCurrentPage());
        textBoxOpen = false;
        currentDocument.setDocumentChanged(true);
        //emit modified();
    }
    setCurrentState(state::IDLE);
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

  newSelection.setPageNum(pageNum);
  newSelection.setWidth(currentDocument.pages[pageNum].width());
  newSelection.setHeight(currentDocument.pages[pageNum].height());
  newSelection.appendToSelectionPolygon(pagePos);

  currentSelection = newSelection;

  //    selecting = true;
  currentState = state::SELECTING;
  selectingOnPage = pageNum;
}

void Widget::continueSelecting(QPointF mousePos)
{
  int pageNum = selectingOnPage;
  QPointF pagePos = getPagePosFromMousePos(mousePos, pageNum);

  currentSelection.appendToSelectionPolygon(pagePos);

  update();
}

void Widget::stopSelecting(QPointF mousePos)
{
  int pageNum = selectingOnPage;
  QPointF pagePos = getPagePosFromMousePos(mousePos, pageNum);

  currentSelection.appendToSelectionPolygon(pagePos);

  if (!currentDocument.pages[pageNum].getStrokes(currentSelection.selectionPolygon()).isEmpty())
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
}

void Widget::letGoSelection()
{
  if (getCurrentState() == state::SELECTED)
  {
    int pageNum = currentSelection.pageNum();
    ReleaseSelectionCommand *releaseCommand = new ReleaseSelectionCommand(this, pageNum);
    undoStack.push(releaseCommand);
    updateAllDirtyBuffers();
    setCurrentState(state::IDLE);
  }
}

void Widget::startRuling(QPointF mousePos)
{
  currentDocument.setDocumentChanged(true);
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

  QRect clipRect(zoom * firstPagePos.toPoint(), zoom * pagePos.toPoint());
  QRect oldClipRect(zoom * firstPagePos.toPoint(), zoom * previousPagePos.toPoint());
  clipRect = clipRect.normalized().united(oldClipRect.normalized());
  int clipRad = zoom * currentPenWidth / 2 + 2;
  clipRect = clipRect.normalized().adjusted(-clipRad, -clipRad, clipRad, clipRad);
  updateBufferRegion(drawingOnPage, clipRect);
  drawOnBuffer();

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

  AddStrokeCommand *addCommand = new AddStrokeCommand(this, drawingOnPage, currentStroke);
  undoStack.push(addCommand);

  currentState = state::IDLE;

  update();
}

void Widget::startCircling(QPointF mousePos)
{
  currentDocument.setDocumentChanged(true);
  emit modified();

  int pageNum = getPageFromMousePos(mousePos);

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

  currentDashOffset = 0.0;

  MrDoc::Stroke oldStroke = currentStroke;

  currentStroke.points.clear();
  currentStroke.pressures.clear();

  qreal radius = QLineF(firstPagePos, pagePos).length();
  qreal phi0 = QLineF(firstPagePos, pagePos).angle() * M_PI / 180.0;

  int N = 100;
  for (int i = 0; i < N; ++i)
  {
    qreal phi = phi0 + i * (2.0 * M_PI / (N - 1));
    qreal x = firstPagePos.x() + radius * cos(phi);
    qreal y = firstPagePos.y() - radius * sin(phi);
    currentStroke.points.append(QPointF(x, y));
    currentStroke.pressures.append(1.0);
  }

  QTransform scaleTrans;
  scaleTrans = scaleTrans.scale(zoom, zoom);

  QRect clipRect = scaleTrans.mapRect(currentStroke.points.boundingRect()).toRect();
  QRect oldClipRect = scaleTrans.mapRect(oldStroke.points.boundingRect()).toRect();
  clipRect = clipRect.normalized().united(oldClipRect.normalized());
  int clipRad = zoom * currentPenWidth / 2 + 2;
  clipRect = clipRect.normalized().adjusted(-clipRad, -clipRad, clipRad, clipRad);
  updateBufferRegion(drawingOnPage, clipRect);

  drawOnBuffer();

  update();

  previousMousePos = mousePos;
}

void Widget::stopCircling(QPointF mousePos)
{
  continueCircling(mousePos);

  AddStrokeCommand *addCommand = new AddStrokeCommand(this, drawingOnPage, currentStroke);
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

  currentStroke.points.append(pagePos);
  currentStroke.pressures.append(pressure);
  drawOnBuffer(true);

  QRect updateRect(previousMousePos.toPoint(), mousePos.toPoint());
  int rad = currentPenWidth * zoom / 2 + 2;
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

  AddStrokeCommand *addCommand = new AddStrokeCommand(this, drawingOnPage, currentStroke, -1, false, true);
  undoStack.push(addCommand);

  //  currentState = state::IDLE;
  setCurrentState(state::IDLE);

  update();
}

void Widget::erase(QPointF mousePos, bool invertEraser)
{
  int pageNum = getPageFromMousePos(mousePos);
  QPointF pagePos = getPagePosFromMousePos(mousePos, pageNum);

  const QVector<MrDoc::Stroke> &strokes = currentDocument.pages[pageNum].strokes();

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

  if (realEraser || (!realEraser && invertEraser))
  {
    for (int i = strokes.size() - 1; i >= 0; --i)
    {
      MrDoc::Stroke stroke = strokes.at(i);
      if (rectE.intersects(stroke.points.boundingRect()) || !stroke.points.boundingRect().isValid()) // this is done for speed
      {
        for (int j = 0; j < stroke.points.length() - 1; ++j)
        {
          QLineF line = QLineF(stroke.points.at(j), stroke.points.at(j + 1));
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
              splitStroke.points = splitStroke.points.mid(0, j + 1);
              splitStroke.points.append(iPoint);
              splitStroke.pressures = splitStroke.pressures.mid(0, j + 1);
              qreal lastPressure = splitStroke.pressures.last();
              splitStroke.pressures.append(lastPressure);

              stroke.points = stroke.points.mid(j + 1);
              stroke.points.prepend(iPoint);
              stroke.pressures = stroke.pressures.mid(j + 1);
              qreal firstPressure = stroke.pressures.first();
              stroke.pressures.prepend(firstPressure);

              RemoveStrokeCommand *removeStrokeCommand = new RemoveStrokeCommand(this, pageNum, i, false);
              undoStack.push(removeStrokeCommand);
              AddStrokeCommand *addStrokeCommand = new AddStrokeCommand(this, pageNum, stroke, i, false, false);
              undoStack.push(addStrokeCommand);
              addStrokeCommand = new AddStrokeCommand(this, pageNum, splitStroke, i, false, false);
              undoStack.push(addStrokeCommand);
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
    if (rectE.intersects(stroke.points.boundingRect()) || !stroke.points.boundingRect().isValid()) // this is done for speed
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
        for (int j = 0; j < stroke.points.length() - 1; ++j)
        {
          QLineF line = QLineF(stroke.points.at(j), stroke.points.at(j + 1));
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
    currentDocument.setDocumentChanged(true);
    emit modified();

    //    QRect updateRect;
    std::sort(strokesToDelete.begin(), strokesToDelete.end(), std::greater<int>());
    for (int i = 0; i < strokesToDelete.size(); ++i)
    {
      //      updateRect = updateRect.united(currentDocument.pages[pageNum].m_strokes.at(strokesToDelete.at(i)).points.boundingRect().toRect());
      RemoveStrokeCommand *removeCommand = new RemoveStrokeCommand(this, pageNum, strokesToDelete[i]);
      undoStack.push(removeCommand);
    }
  }
  updateAllDirtyBuffers();
}

void Widget::startMovingSelection(QPointF mousePos)
{
  currentDocument.setDocumentChanged(true);
  emit modified();

  int pageNum = getPageFromMousePos(mousePos);
  previousPagePos = getPagePosFromMousePos(mousePos, pageNum);
  setCurrentState(state::MOVING_SELECTION);
}

void Widget::continueMovingSelection(QPointF mousePos)
{
  int pageNum = getPageFromMousePos(mousePos);
  QPointF pagePos = getPagePosFromMousePos(mousePos, pageNum);
  //    currentSelection.move(1 * (pagePos - previousPagePos));

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

  int pageNum = currentSelection.pageNum();
  previousPagePos = getPagePosFromMousePos(mousePos, pageNum);
  setCurrentState(state::ROTATING_SELECTION);
}

void Widget::continueRotatingSelection(QPointF mousePos)
{
  int pageNum = currentSelection.pageNum();
  QPointF pagePos = getPagePosFromMousePos(mousePos, pageNum);

  m_currentAngle = QLineF(currentSelection.boundingRect().center(), pagePos).angleTo(QLineF(currentSelection.boundingRect().center(), previousPagePos));
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
  undoStack.push(transSelectCommand);

  currentSelection.finalize();
  currentSelection.updateBuffer(zoom);
  setCurrentState(state::SELECTED);
}

void Widget::startResizingSelection(QPointF mousePos, MrDoc::Selection::GrabZone grabZone)
{
  currentDocument.setDocumentChanged(true);
  emit modified();

  m_grabZone = grabZone;

  int pageNum = currentSelection.pageNum();
  previousPagePos = getPagePosFromMousePos(mousePos, pageNum);
  setCurrentState(state::RESIZING_SELECTION);
}

void Widget::continueResizingSelection(QPointF mousePos)
{
  int pageNum = currentSelection.pageNum();
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
  currentSelection.updateBuffer(zoom);
  setCurrentState(state::SELECTED);
}

int Widget::getPageFromMousePos(QPointF mousePos)
{
  qreal y = mousePos.y(); // - currentCOSPos.y();
  int pageNum = 0;
  while (y > (floor(currentDocument.pages[pageNum].height() * zoom)) + PAGE_GAP)
  {
    y -= (floor(currentDocument.pages[pageNum].height() * zoom)) + PAGE_GAP;
    pageNum += 1;
    if (pageNum >= currentDocument.pages.size())
    {
      pageNum = currentDocument.pages.size() - 1;
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

int Widget::getVisiblePages(){
    //qDebug() << "Page Height: " << currentDocument.pages.at(getCurrentPage()).height();
    //qDebug() << "parentWidget()->size().height(): " << parentWidget()->size().height();
    //qDebug() << "Zoom: " << zoom;
    int visiblePages = currentDocument.pages.at(getCurrentPage()).height()/parentWidget()->size().height()/zoom + 1;
    return visiblePages;
}

QPointF Widget::getPagePosFromMousePos(QPointF mousePos, int pageNum)
{
  qreal x = mousePos.x();
  qreal y = mousePos.y();
  for (int i = 0; i < pageNum; ++i)
  {
    //        y -= (currentDocument.pages[i].height() * zoom + PAGE_GAP); // THIS DOESN'T WORK PROPERLY (should be floor(...height(), or just use
    //        pageBuffer[i].height()/devicePixelRatio())
    y -= (pageBufferPtr[i]->height()/devicePixelRatio() + PAGE_GAP);
  }
  //    y -= (pageNum) * (currentDocument.pages[0].height() * zoom + PAGE_GAP);

  QPointF pagePos = (QPointF(x, y)) / zoom;

  return pagePos;
}

QPointF Widget::getAbsolutePagePosFromMousePos(QPointF mousePos)
{
  int pageNum = getPageFromMousePos(mousePos);
  QPointF pagePos = getPagePosFromMousePos(mousePos, pageNum);

  qreal y = 0.0;
  for (int i = 0; i < pageNum; ++i)
  {
    y += (pageBufferPtr[i]->height()/devicePixelRatio() + PAGE_GAP);
  }
  y *= zoom;

  pagePos.setY(y + pagePos.y());

  return pagePos;
}

void Widget::newFile()
{
  letGoSelection();

  currentDocument = MrDoc::Document();
  pageBufferPtr.clear();
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

  prevZoom = zoom;
  zoom = newZoom;

  int prevHMax = scrollArea->horizontalScrollBar()->maximum();
  int prevVMax = scrollArea->verticalScrollBar()->maximum();

  int prevH = scrollArea->horizontalScrollBar()->value();
  int prevV = scrollArea->verticalScrollBar()->value();

  updateAllPageBuffers();
  currentSelection.updateBuffer(zoom);
  setGeometry(getWidgetGeometry());

  int newHMax = scrollArea->horizontalScrollBar()->maximum();
  int newVMax = scrollArea->verticalScrollBar()->maximum();

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

  scrollArea->horizontalScrollBar()->setValue(newH);
  scrollArea->verticalScrollBar()->setValue(newV);

  //scrollArea->verticalScrollBar()->setTracking(false);
  connect(scrollArea->verticalScrollBar(), &QScrollBar::valueChanged, this, &Widget::updatePageAfterScrolling, Qt::UniqueConnection);
  update();
}

void Widget::zoomFitWidth()
{
  int pageNum = getCurrentPage();

  QSize widgetSize = this->parentWidget()->size();
  qreal newZoom = widgetSize.width() / currentDocument.pages[pageNum].width();

  zoomTo(newZoom);
}

void Widget::zoomFitHeight()
{
  int pageNum = getCurrentPage();

  QSize widgetSize = this->parentWidget()->size();
  qreal newZoom = widgetSize.height() / currentDocument.pages[pageNum].height();

  zoomTo(newZoom);
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

  if (pageNum >= currentDocument.pages.size())
  {
    pageAddEnd();
  }

  scrollDocumentToPageNum(pageNum);
}

void Widget::pageAddBefore()
{
  int pageNum = getCurrentPage();
  AddPageCommand *addPageCommand = new AddPageCommand(this, pageNum);
  undoStack.push(addPageCommand);
  setGeometry(getWidgetGeometry());
  update();
  currentDocument.setDocumentChanged(true);
  emit modified();
}

void Widget::pageAddAfter()
{
  int pageNum = getCurrentPage() + 1;
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
    int pageNum = getCurrentPage();
    RemovePageCommand *removePageCommand = new RemovePageCommand(this, pageNum);
    undoStack.push(removePageCommand);
    setGeometry(getWidgetGeometry());
    update();
    currentDocument.setDocumentChanged(true);
    emit modified();
  }
}

void Widget::scrollDocumentToPageNum(int pageNum)
{
  if (pageNum >= currentDocument.pages.size())
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
    y += (currentDocument.pages[i].height()) * zoom + PAGE_GAP;
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
    if (toolID == tool::TEXT)
        setCursor(Qt::ArrowCursor);
  }
}

void Widget::setDocument(const MrDoc::Document &newDocument)
{
  currentDocument = newDocument;
  undoStack.clear();
  pageBufferPtr.clear();
  prevZoom = -1.0;  //this is a workaround, that all pages get rendered and updateNecessaryPagesBuffer is not called
  zoom = 0.0; // otherwise zoomTo() doesn't do anything if zoom == newZoom
  zoomFitWidth();
  pageFirst();
}

void Widget::selectAll()
{

  if (currentState == state::SELECTED)
  {
    letGoSelection();
  }

  int pageNum = getCurrentPage();
  QRectF selectRect;
  for (auto &stroke : currentDocument.pages[pageNum].strokes())
  {
    selectRect = selectRect.united(stroke.boundingRect());
  }
  QPolygonF selectionPolygon = QPolygonF(selectRect);

  MrDoc::Selection selection;
  selection.setPageNum(pageNum);
  selection.setSelectionPolygon(selectionPolygon);

  if (!currentDocument.pages[pageNum].getStrokes(selection.selectionPolygon()).isEmpty())
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
  tmpSelection.updateBuffer(zoom);

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

void Widget::cut()
{
  CutCommand *cutCommand = new CutCommand(this);
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
    currentSelection.updateBuffer(zoom);
    updateAllDirtyBuffers();
  }
}

void Widget::redo()
{
  if (undoStack.canRedo() && (currentState == state::IDLE || currentState == state::SELECTED))
  {
    undoStack.redo();
    currentSelection.updateBuffer(zoom);
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
    currentSelection.updateBuffer(zoom);
    update();
  }
  emit updateGUI();
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

  qreal dx = currentSelection.boundingRect().center().x();
  qreal dy = currentSelection.boundingRect().center().y();

  rotateTrans = rotateTrans.translate(dx, dy).rotate(-angle).translate(-dx, -dy);
  TransformSelectionCommand *transCommand = new TransformSelectionCommand(this, currentSelection.pageNum(), rotateTrans);
  undoStack.push(transCommand);
  currentSelection.finalize();
  currentSelection.updateBuffer(zoom);
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
    currentSelection.updateBuffer(zoom);
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
    currentSelection.updateBuffer(zoom);
    update();
  }
}
