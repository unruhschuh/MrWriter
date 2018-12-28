#include "commands.h"
#include "mainwindow.h"
#include <QDebug>

/******************************************************************************
** AddStrokeCommand
*/

/**
 * @brief AddStrokeCommand::AddStrokeCommand
 * @param newWidget
 * @param newPageNum
 * @param newStroke
 * @param newStrokeNum
 * @param newUpdate
 * @param newUpdateSuccessive
 * @param parent
 * @todo remove parameter newUpdate
 */
AddStrokeCommand::AddStrokeCommand(Widget *newWidget, int newPageNum, const MrDoc::Stroke &newStroke, int newStrokeNum, bool newUpdate,
                                   bool newUpdateSuccessive, QUndoCommand *parent)
    : QUndoCommand(parent)
{
  setText(MainWindow::tr("Add Stroke"));
  pageNum = newPageNum;
  widget = newWidget;
  stroke = newStroke;
  strokeNum = newStrokeNum;
  update = newUpdate;
  updateSuccessive = newUpdateSuccessive;

  // delete duplicate points
  for (int i = stroke.points.length() - 1; i > 0; --i)
  {
    if (stroke.points.at(i) == stroke.points.at(i - 1))
    {
      stroke.points.removeAt(i);
      stroke.pressures.removeAt(i);
    }
  }
}

void AddStrokeCommand::undo()
{
  if (stroke.points.length() > 0)
  {
    if (strokeNum == -1)
    {
      widget->currentDocument.pages[pageNum].removeStrokeAt(widget->currentDocument.pages[pageNum].strokes().size() - 1);
    }
    else
    {
      widget->currentDocument.pages[pageNum].removeStrokeAt(strokeNum);
    }
  }
}

void AddStrokeCommand::redo()
{
  if (stroke.points.length() > 0)
  {
    if (strokeNum == -1)
    {
      widget->currentDocument.pages[pageNum].appendStroke(stroke);
    }
    else
    {
      widget->currentDocument.pages[pageNum].insertStroke(strokeNum, stroke);
    }
  }
}

/******************************************************************************
** RemoveStrokeCommand
*/

/**
 * @brief RemoveStrokeCommand::RemoveStrokeCommand
 * @param newWidget
 * @param newPageNum
 * @param newStrokeNum
 * @param newUpdate
 * @param parent
 * @todo remove parameter newUpdate
 */
RemoveStrokeCommand::RemoveStrokeCommand(Widget *newWidget, int newPageNum, int newStrokeNum, bool newUpdate, QUndoCommand *parent) : QUndoCommand(parent)
{
  setText(MainWindow::tr("Remove Stroke"));
  pageNum = newPageNum;
  widget = newWidget;
  strokeNum = newStrokeNum;
  stroke = widget->currentDocument.pages[pageNum].strokes()[strokeNum];
  update = newUpdate;
}

void RemoveStrokeCommand::undo()
{
  widget->currentDocument.pages[pageNum].insertStroke(strokeNum, stroke);

  qreal zoom = widget->zoom;
  QRect updateRect = stroke.points.boundingRect().toRect();
  updateRect = QRect(zoom * updateRect.topLeft(), zoom * updateRect.bottomRight());
  int delta = zoom * 10;
  updateRect.adjust(-delta, -delta, delta, delta);
}

void RemoveStrokeCommand::redo()
{
  widget->currentDocument.pages[pageNum].removeStrokeAt(strokeNum);

  qreal zoom = widget->zoom;
  QRect updateRect = stroke.points.boundingRect().toRect();
  updateRect = QRect(zoom * updateRect.topLeft(), zoom * updateRect.bottomRight());
  int delta = zoom * 10;
  updateRect.adjust(-delta, -delta, delta, delta);
}

/******************************************************************************
** CreateSelectionCommand
*/

CreateSelectionCommand::CreateSelectionCommand(Widget *widget, int pageNum, MrDoc::Selection selection, QUndoCommand *parent) : QUndoCommand(parent)
{
  setText(MainWindow::tr("Create Selection"));
  m_widget = widget;
  m_pageNum = pageNum;
  m_selection = selection;
  m_selectionPolygon = selection.selectionPolygon();

  m_strokesAndPositions = widget->currentDocument.pages[pageNum].getStrokes(m_selectionPolygon);

  for (auto sAndP : m_strokesAndPositions)
  {
    m_selection.prependStroke(sAndP.first);
  }
  m_selection.finalize();
  m_selection.updateBuffer(m_widget->zoom);
}

void CreateSelectionCommand::undo()
{
  m_widget->currentDocument.pages[m_pageNum].insertStrokes(m_strokesAndPositions);

  m_widget->setCurrentState(Widget::state::IDLE);
}

void CreateSelectionCommand::redo()
{
  for (auto &sAndP : m_strokesAndPositions)
  {
    m_widget->currentDocument.pages[m_pageNum].removeStrokeAt(sAndP.second);
  }
  m_widget->currentSelection = m_selection;
  m_widget->setCurrentState(Widget::state::SELECTED);
}

/******************************************************************************
** ReleaseSelectionCommand
*/

ReleaseSelectionCommand::ReleaseSelectionCommand(Widget *newWidget, int newPageNum, QUndoCommand *parent) : QUndoCommand(parent)
{
  setText(MainWindow::tr("Release Selection"));

  widget = newWidget;
  selection = widget->currentSelection;
  pageNum = newPageNum;
}

void ReleaseSelectionCommand::undo()
{
  widget->currentSelection = selection;
  for (int i = 0; i < widget->currentSelection.strokes().size(); ++i)
  {
    widget->currentDocument.pages[pageNum].removeLastStroke();
  }
  widget->setCurrentState(Widget::state::SELECTED);
}

void ReleaseSelectionCommand::redo()
{
  int pageNum = widget->currentSelection.pageNum();
  widget->currentDocument.pages[pageNum].appendStrokes(widget->currentSelection.strokes());
  widget->setCurrentState(Widget::state::IDLE);
}

/******************************************************************************
** TransformSelectionCommand
*/

TransformSelectionCommand::TransformSelectionCommand(Widget *newWidget, int newPageNum, QTransform newTransform, QUndoCommand *parent) : QUndoCommand(parent)
{
  setText(MainWindow::tr("Transform Selection"));

  widget = newWidget;
  pageNum = newPageNum;
  selection = widget->currentSelection;
  transform = newTransform;
}

void TransformSelectionCommand::undo()
{
  widget->currentSelection = selection;
}

void TransformSelectionCommand::redo()
{
  widget->currentSelection.transform(transform, pageNum);
}

bool TransformSelectionCommand::mergeWith(const QUndoCommand *other)
{
  if (other->id() != id())
    return false;
  transform *= static_cast<const TransformSelectionCommand *>(other)->transform;
  pageNum = static_cast<const TransformSelectionCommand *>(other)->pageNum;

  return true;
}

/******************************************************************************
** ChangeColorOfSelectionCommand
*/

ChangeColorOfSelectionCommand::ChangeColorOfSelectionCommand(Widget *widget, QColor color, QUndoCommand *parent) : QUndoCommand(parent)
{
  setText(MainWindow::tr("Change Color"));

  m_widget = widget;
  m_selection = m_widget->currentSelection;
  m_color = color;
}

void ChangeColorOfSelectionCommand::undo()
{
  m_widget->currentSelection = m_selection;
}

void ChangeColorOfSelectionCommand::redo()
{
  for (int i = 0; i < m_widget->currentSelection.strokes().size(); ++i)
  {
    m_widget->currentSelection.changeStrokeColor(i, m_color);
  }
}

/******************************************************************************
** ChangePatternOfSelectionCommand
*/

ChangePatternOfSelectionCommand::ChangePatternOfSelectionCommand(Widget *widget, QVector<qreal> pattern, QUndoCommand *parent) : QUndoCommand(parent)
{
  setText(MainWindow::tr("Change Pattern"));

  m_widget = widget;
  m_selection = m_widget->currentSelection;
  m_pattern = pattern;
}

void ChangePatternOfSelectionCommand::undo()
{
  m_widget->currentSelection = m_selection;
}

void ChangePatternOfSelectionCommand::redo()
{
  for (int i = 0; i < m_widget->currentSelection.strokes().size(); ++i)
  {
    m_widget->currentSelection.changeStrokePattern(i, m_pattern);
  }
}

/******************************************************************************
** ChangePenWidthOfSelection
*/

ChangePenWidthOfSelectionCommand::ChangePenWidthOfSelectionCommand(Widget *newWidget, qreal penWidth, QUndoCommand *parent) : QUndoCommand(parent)
{
  setText(MainWindow::tr("Change Pen Width"));

  widget = newWidget;
  selection = widget->currentSelection;
  m_penWidth = penWidth;
}

void ChangePenWidthOfSelectionCommand::undo()
{
  widget->currentSelection = selection;
}

void ChangePenWidthOfSelectionCommand::redo()
{
  for (int i = 0; i < widget->currentSelection.strokes().size(); ++i)
  {
    widget->currentSelection.changePenWidth(i, m_penWidth);
  }
}

/******************************************************************************
** AddPageCommand
*/

AddPageCommand::AddPageCommand(Widget *newWidget, int newPageNum, QUndoCommand *parent) : QUndoCommand(parent)
{
  setText(MainWindow::tr("Add Page"));
  widget = newWidget;
  pageNum = newPageNum;
}

void AddPageCommand::undo()
{
  widget->currentDocument.pages.removeAt(pageNum);
  widget->pageBuffer.removeAt(pageNum);
  widget->update();
}

void AddPageCommand::redo()
{
  page = MrDoc::Page();
  int pageNumForSettings;

  if (pageNum == 0)
    pageNumForSettings = pageNum;
  else
    pageNumForSettings = pageNum - 1;

  page.setWidth(widget->currentDocument.pages[pageNumForSettings].width());
  page.setHeight(widget->currentDocument.pages[pageNumForSettings].height());
  page.setBackgroundColor(widget->currentDocument.pages[pageNumForSettings].backgroundColor());

  widget->currentDocument.pages.insert(pageNum, page);
  widget->pageBuffer.insert(pageNum, QPixmap());
  widget->updateBuffer(pageNum);
  widget->update();
}

/******************************************************************************
** RemovePageCommand
*/

RemovePageCommand::RemovePageCommand(Widget *newWidget, int newPageNum, QUndoCommand *parent) : QUndoCommand(parent)
{
  setText(MainWindow::tr("Remove Page"));
  widget = newWidget;
  pageNum = newPageNum;
  page = widget->currentDocument.pages[pageNum];
}

void RemovePageCommand::undo()
{
  widget->currentDocument.pages.insert(pageNum, page);
  widget->pageBuffer.insert(pageNum, QPixmap());
  widget->updateBuffer(pageNum);
  widget->update();
}

void RemovePageCommand::redo()
{
  widget->currentDocument.pages.removeAt(pageNum);
  widget->pageBuffer.removeAt(pageNum);
  widget->update();
}

/******************************************************************************
** RemovePageCommand
*/

PasteCommand::PasteCommand(Widget *newWidget, MrDoc::Selection newSelection, QUndoCommand *parent) : QUndoCommand(parent)
{
  setText(MainWindow::tr("Paste"));
  widget = newWidget;
  pasteSelection = newSelection;
  previousSelection = widget->currentSelection;
  previousState = widget->getCurrentState();
}

void PasteCommand::undo()
{
  widget->currentSelection = previousSelection;
  widget->setCurrentState(previousState);
  widget->update();
}

void PasteCommand::redo()
{
  widget->currentSelection = pasteSelection;
  widget->setCurrentState(Widget::state::SELECTED);
  widget->update();
}

/******************************************************************************
** CutCommand
*/

CutCommand::CutCommand(Widget *newWidget, QUndoCommand *parent) : QUndoCommand(parent)
{
  setText(MainWindow::tr("Cut"));
  widget = newWidget;
  previousSelection = widget->currentSelection;
  previousState = widget->getCurrentState();
}

void CutCommand::undo()
{
  widget->currentSelection = previousSelection;
  widget->setCurrentState(previousState);
}

void CutCommand::redo()
{
  widget->clipboard = previousSelection;
  widget->currentSelection = MrDoc::Selection();
  widget->setCurrentState(Widget::state::IDLE);
}

/******************************************************************************
** ChangePageSettingsCommand
*/

ChangePageSettingsCommand::ChangePageSettingsCommand(Widget *newWidget, int newPageNum, QSizeF newSize, QColor newBackgroundColor, QUndoCommand *parent)
    : QUndoCommand(parent)
{
  widget = newWidget;
  pageNum = newPageNum;
  prevSize = QSizeF(widget->currentDocument.pages[pageNum].width(), widget->currentDocument.pages[pageNum].height());
  size = newSize;
  prevBackgroundColor = widget->currentDocument.pages[pageNum].backgroundColor();
  backgroundColor = newBackgroundColor;
}

void ChangePageSettingsCommand::undo()
{
  qreal width = prevSize.width();
  qreal height = prevSize.height();
  widget->currentDocument.pages[pageNum].setWidth(width);
  widget->currentDocument.pages[pageNum].setHeight(height);
  widget->currentDocument.pages[pageNum].setBackgroundColor(prevBackgroundColor);
  widget->updateBuffer(pageNum);
  widget->setGeometry(widget->getWidgetGeometry());
}

void ChangePageSettingsCommand::redo()
{
  qreal width = size.width();
  qreal height = size.height();
  widget->currentDocument.pages[pageNum].setWidth(width);
  widget->currentDocument.pages[pageNum].setHeight(height);
  widget->currentDocument.pages[pageNum].setBackgroundColor(backgroundColor);
  widget->updateBuffer(pageNum);
  widget->setGeometry(widget->getWidgetGeometry());
}
