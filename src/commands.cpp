#include "commands.h"
#include "mainwindow.h"
#include <QDebug>
#include <memory>
#include "element.h"
#include "tools.h"

/******************************************************************************
** AddElementCommand
*/

/**
 * @brief AddElementCommand::AddElementCommand
 * @param newWidget
 * @param newPageNum
 * @param newElement
 * @param newElementNum
 * @param newUpdate
 * @param newUpdateSuccessive
 * @param parent
 * @todo remove parameter newUpdate
 */
AddElementCommand::AddElementCommand(Widget *newWidget, int newPageNum, std::unique_ptr<MrDoc::Element> newElement, int newElementNum, bool newUpdate,
                                   bool newUpdateSuccessive, QUndoCommand *parent)
    : QUndoCommand(parent)
{
  setText(MainWindow::tr("Add Element"));
  pageNum = newPageNum;
  widget = newWidget;
  element = std::move(newElement);
  elementNum = newElementNum;
  update = newUpdate;
  updateSuccessive = newUpdateSuccessive;

  // delete duplicate points
  /*
  for (int i = element.points.length() - 1; i > 0; --i)
  {
    if (element.points.at(i) == element.points.at(i - 1))
    {
      element.points.removeAt(i);
      element.pressures.removeAt(i);
    }
  }
  */
}

void AddElementCommand::undo()
{
  if (elementNum == -1)
  {
    widget->currentDocument.pages[pageNum].removeElementAt(widget->currentDocument.pages[pageNum].elements().size() - 1);
  }
  else
  {
    widget->currentDocument.pages[pageNum].removeElementAt(elementNum);
  }
}

void AddElementCommand::redo()
{
  if (elementNum == -1)
  {
    widget->currentDocument.pages[pageNum].appendElement(element->clone());
  }
  else
  {
    widget->currentDocument.pages[pageNum].insertElement(elementNum, element->clone());
  }
}

/******************************************************************************
** RemoveElementCommand
*/

/**
 * @brief RemoveElementCommand::RemoveElementCommand
 * @param newWidget
 * @param newPageNum
 * @param newElementNum
 * @param newUpdate
 * @param parent
 * @todo remove parameter newUpdate
 */
RemoveElementCommand::RemoveElementCommand(Widget *newWidget, int newPageNum, int newElementNum, bool newUpdate, QUndoCommand *parent) : QUndoCommand(parent)
{
  setText(MainWindow::tr("Remove Element"));
  pageNum = newPageNum;
  widget = newWidget;
  elementNum = newElementNum;
  element = widget->currentDocument.pages[pageNum].elements()[elementNum]->clone();
  update = newUpdate;
}

void RemoveElementCommand::undo()
{
  widget->currentDocument.pages[pageNum].insertElement(elementNum, element->clone());

  qreal zoom = widget->m_zoom;
  QRect updateRect = element->boundingRect().toRect();
  updateRect = QRect(zoom * updateRect.topLeft(), zoom * updateRect.bottomRight());
  int delta = zoom * 10;
  updateRect.adjust(-delta, -delta, delta, delta);
}

void RemoveElementCommand::redo()
{
  widget->currentDocument.pages[pageNum].removeElementAt(elementNum);

  qreal zoom = widget->m_zoom;
  QRect updateRect = element->boundingRect().toRect();
  updateRect = QRect(zoom * updateRect.topLeft(), zoom * updateRect.bottomRight());
  int delta = zoom * 10;
  updateRect.adjust(-delta, -delta, delta, delta);
}

/******************************************************************************
** CreateSelectionCommand
*/

CreateSelectionCommand::CreateSelectionCommand(Widget *widget, int pageNum, const MrDoc::Selection & selection, QUndoCommand *parent) : QUndoCommand(parent)
{
  setText(MainWindow::tr("Create Selection"));
  m_widget = widget;
  m_pageNum = pageNum;
  m_selection = selection;
  m_selectionPolygon = selection.selectionPolygon();

  m_elementsAndPositions = widget->currentDocument.pages[pageNum].getElements(m_selectionPolygon);

  for (auto &sAndP : m_elementsAndPositions)
  {
    m_selection.prependElement(sAndP.first->clone());
  }
  m_selection.finalize();
  m_selection.updateBuffer(m_widget->m_zoom);
}

void CreateSelectionCommand::undo()
{
  m_widget->currentDocument.pages[m_pageNum].insertElements(m_elementsAndPositions);

  m_widget->setCurrentState(Widget::state::IDLE);
}

void CreateSelectionCommand::redo()
{
  for (auto &sAndP : m_elementsAndPositions)
  {
    m_widget->currentDocument.pages[m_pageNum].removeElementAt(sAndP.second);
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
  for (int i = 0; i < widget->currentSelection.elements().size(); ++i)
  {
    widget->currentDocument.pages[pageNum].removeLastElement();
  }
  widget->setCurrentState(Widget::state::SELECTED);
}

void ReleaseSelectionCommand::redo()
{
  int pageNum = widget->currentSelection.pageNum();
  widget->currentDocument.pages[pageNum].appendElements(widget->currentSelection.elements());
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
  for (int i = 0; i < m_widget->currentSelection.elements().size(); ++i)
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
  for (int i = 0; i < m_widget->currentSelection.elements().size(); ++i)
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
  for (int i = 0; i < widget->currentSelection.elements().size(); ++i)
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
  widget->currentDocument.pages.erase(widget->currentDocument.pages.begin() + pageNum);
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

  widget->currentDocument.pages.insert(widget->currentDocument.pages.begin() + pageNum, page);
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
  widget->currentDocument.pages.insert(widget->currentDocument.pages.begin() + pageNum, page);
  widget->pageBuffer.insert(pageNum, QPixmap());
  widget->updateBuffer(pageNum);
  widget->update();
}

void RemovePageCommand::redo()
{
  widget->currentDocument.pages.erase(widget->currentDocument.pages.begin() + pageNum);
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
