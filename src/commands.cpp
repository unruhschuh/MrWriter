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
AddElementCommand::AddElementCommand(Widget *newWidget, size_t newPageNum, std::unique_ptr<MrDoc::Element> newElement, bool newAtEnd, size_t newElementNum, bool newUpdate, bool newUpdateSuccessive, QUndoCommand *parent)
    : QUndoCommand(parent)
{
  setText(MainWindow::tr("Add Element"));
  pageNum = newPageNum;
  widget = newWidget;
  element = std::move(newElement);
  elementNum = newElementNum;
  update = newUpdate;
  updateSuccessive = newUpdateSuccessive;
  atEnd = newAtEnd;

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
  if (atEnd)
  {
    widget->m_currentDocument.pages[pageNum].removeElementAt(widget->m_currentDocument.pages[pageNum].elements().size() - 1);
  }
  else
  {
    widget->m_currentDocument.pages[pageNum].removeElementAt(elementNum);
  }
}

void AddElementCommand::redo()
{
  if (atEnd)
  {
    widget->m_currentDocument.pages[pageNum].appendElement(element->clone());
  }
  else
  {
    widget->m_currentDocument.pages[pageNum].insertElement(elementNum, element->clone());
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
RemoveElementCommand::RemoveElementCommand(Widget *newWidget, size_t newPageNum, size_t newElementNum, bool newUpdate, QUndoCommand *parent) : QUndoCommand(parent)
{
  setText(MainWindow::tr("Remove Element"));
  pageNum = newPageNum;
  widget = newWidget;
  elementNum = newElementNum;
  element = widget->m_currentDocument.pages[pageNum].elements()[elementNum]->clone();
  update = newUpdate;
}

void RemoveElementCommand::undo()
{
  widget->m_currentDocument.pages[pageNum].insertElement(elementNum, element->clone());

  qreal zoom = widget->m_zoom;
  QRect updateRect = element->boundingRect().toRect();
  updateRect = QRect(zoom * updateRect.topLeft(), zoom * updateRect.bottomRight());
  int delta = static_cast<int>(zoom * 10.0);
  updateRect.adjust(-delta, -delta, delta, delta);
}

void RemoveElementCommand::redo()
{
  widget->m_currentDocument.pages[pageNum].removeElementAt(elementNum);

  qreal zoom = widget->m_zoom;
  QRect updateRect = element->boundingRect().toRect();
  updateRect = QRect(zoom * updateRect.topLeft(), zoom * updateRect.bottomRight());
  int delta = static_cast<int>(zoom * 10.0);
  updateRect.adjust(-delta, -delta, delta, delta);
}

/******************************************************************************
** CreateSelectionCommand
*/

CreateSelectionCommand::CreateSelectionCommand(Widget *widget, size_t pageNum, const MrDoc::Selection & selection, QUndoCommand *parent) : QUndoCommand(parent)
{
  setText(MainWindow::tr("Create Selection"));
  m_widget = widget;
  m_pageNum = pageNum;
  m_selection = selection;
  m_selectionPolygon = selection.selectionPolygon();

  m_elementsAndPositions = widget->m_currentDocument.pages[pageNum].getElements(m_selectionPolygon);

  for (auto &sAndP : m_elementsAndPositions)
  {
    m_selection.prependElement(sAndP.first->clone());
  }
  m_selection.finalize();
  m_selection.updateBuffer(m_widget->m_zoom);
}

void CreateSelectionCommand::undo()
{
  m_widget->m_currentDocument.pages[m_pageNum].insertElements(m_elementsAndPositions);

  m_widget->setCurrentState(Widget::state::IDLE);
}

void CreateSelectionCommand::redo()
{
  for (auto &sAndP : m_elementsAndPositions)
  {
    m_widget->m_currentDocument.pages[m_pageNum].removeElementAt(sAndP.second);
  }
  m_widget->m_currentSelection = m_selection;
  m_widget->setCurrentState(Widget::state::SELECTED);
}

/******************************************************************************
** ReleaseSelectionCommand
*/

ReleaseSelectionCommand::ReleaseSelectionCommand(Widget *newWidget, size_t newPageNum, bool toTheBack, QUndoCommand *parent) : QUndoCommand(parent)
{
  setText(MainWindow::tr("Release Selection"));

  widget = newWidget;
  selection = widget->m_currentSelection;
  pageNum = newPageNum;
  m_toTheBack = toTheBack;
}

void ReleaseSelectionCommand::undo()
{
  widget->m_currentSelection = selection;
  for (size_t i = 0; i < widget->m_currentSelection.elements().size(); ++i)
  {
    if (m_toTheBack)
    {
      widget->m_currentDocument.pages[pageNum].removeFirstElement();
    }
    else
    {
      widget->m_currentDocument.pages[pageNum].removeLastElement();
    }
  }
  widget->setCurrentState(Widget::state::SELECTED);
}

void ReleaseSelectionCommand::redo()
{
  size_t pageNum = widget->m_currentSelection.pageNum();
  if (m_toTheBack)
  {
    widget->m_currentDocument.pages[pageNum].prependElements(widget->m_currentSelection.elements());
  }
  else
  {
    widget->m_currentDocument.pages[pageNum].appendElements(widget->m_currentSelection.elements());
  }
  widget->setCurrentState(Widget::state::IDLE);
}

/******************************************************************************
** TransformSelectionCommand
*/

TransformSelectionCommand::TransformSelectionCommand(Widget *newWidget, size_t newPageNum, QTransform newTransform, QUndoCommand *parent) : QUndoCommand(parent)
{
  setText(MainWindow::tr("Transform Selection"));

  widget = newWidget;
  pageNum = newPageNum;
  selection = widget->m_currentSelection;
  transform = newTransform;
}

void TransformSelectionCommand::undo()
{
  widget->m_currentSelection = selection;
}

void TransformSelectionCommand::redo()
{
  widget->m_currentSelection.transform(transform, pageNum);
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
** ChangeFontOfSelectionCommand
*/

ChangeFontOfSelectionCommand::ChangeFontOfSelectionCommand(Widget *widget, QFont font, QUndoCommand *parent) : QUndoCommand(parent)
{
  setText(MainWindow::tr("Change Font"));

  m_widget = widget;
  m_selection = m_widget->m_currentSelection;
  m_font = font;
}

void ChangeFontOfSelectionCommand::undo()
{
  m_widget->m_currentSelection = m_selection;
}

void ChangeFontOfSelectionCommand::redo()
{
  for (size_t i = 0; i < m_widget->m_currentSelection.elements().size(); ++i)
  {
    m_widget->m_currentSelection.changeFont(i, m_font);
  }
}

/******************************************************************************
** ChangeColorOfSelectionCommand
*/

ChangeColorOfSelectionCommand::ChangeColorOfSelectionCommand(Widget *widget, QColor color, QUndoCommand *parent) : QUndoCommand(parent)
{
  setText(MainWindow::tr("Change Color"));

  m_widget = widget;
  m_selection = m_widget->m_currentSelection;
  m_color = color;
}

void ChangeColorOfSelectionCommand::undo()
{
  m_widget->m_currentSelection = m_selection;
}

void ChangeColorOfSelectionCommand::redo()
{
  for (size_t i = 0; i < m_widget->m_currentSelection.elements().size(); ++i)
  {
    m_widget->m_currentSelection.changeStrokeColor(i, m_color);
  }
}

/******************************************************************************
** ChangePatternOfSelectionCommand
*/

ChangePatternOfSelectionCommand::ChangePatternOfSelectionCommand(Widget *widget, QVector<qreal> pattern, QUndoCommand *parent) : QUndoCommand(parent)
{
  setText(MainWindow::tr("Change Pattern"));

  m_widget = widget;
  m_selection = m_widget->m_currentSelection;
  m_pattern = pattern;
}

void ChangePatternOfSelectionCommand::undo()
{
  m_widget->m_currentSelection = m_selection;
}

void ChangePatternOfSelectionCommand::redo()
{
  for (size_t i = 0; i < m_widget->m_currentSelection.elements().size(); ++i)
  {
    m_widget->m_currentSelection.changeStrokePattern(i, m_pattern);
  }
}

/******************************************************************************
** ChangePenWidthOfSelection
*/

ChangePenWidthOfSelectionCommand::ChangePenWidthOfSelectionCommand(Widget *newWidget, qreal penWidth, QUndoCommand *parent) : QUndoCommand(parent)
{
  setText(MainWindow::tr("Change Pen Width"));

  widget = newWidget;
  selection = widget->m_currentSelection;
  m_penWidth = penWidth;
}

void ChangePenWidthOfSelectionCommand::undo()
{
  widget->m_currentSelection = selection;
}

void ChangePenWidthOfSelectionCommand::redo()
{
  for (size_t i = 0; i < widget->m_currentSelection.elements().size(); ++i)
  {
    widget->m_currentSelection.changePenWidth(i, m_penWidth);
  }
}

/******************************************************************************
** AddPageCommand
*/

AddPageCommand::AddPageCommand(Widget *newWidget, size_t newPageNum, QUndoCommand *parent) : QUndoCommand(parent)
{
  setText(MainWindow::tr("Add Page"));
  widget = newWidget;
  pageNum = newPageNum;
}

void AddPageCommand::undo()
{
  widget->m_currentDocument.pages.erase(widget->m_currentDocument.pages.begin() + static_cast<long>(pageNum));
  widget->m_pageBuffer.erase(widget->m_pageBuffer.begin() + static_cast<long>(pageNum));
  widget->update();
}

void AddPageCommand::redo()
{
  page = MrDoc::Page();
  size_t pageNumForSettings;

  if (pageNum == 0)
    pageNumForSettings = pageNum;
  else
    pageNumForSettings = pageNum - 1;

  page.setWidth(widget->m_currentDocument.pages[pageNumForSettings].width());
  page.setHeight(widget->m_currentDocument.pages[pageNumForSettings].height());
  page.setBackgroundColor(widget->m_currentDocument.pages[pageNumForSettings].backgroundColor());

  widget->m_currentDocument.pages.insert(widget->m_currentDocument.pages.begin() + static_cast<long>(pageNum), page);
  widget->m_pageBuffer.insert(widget->m_pageBuffer.begin() + static_cast<long>(pageNum), QPixmap());
  widget->updateBuffer(pageNum);
  widget->update();
}

/******************************************************************************
** RemovePageCommand
*/

RemovePageCommand::RemovePageCommand(Widget *newWidget, size_t newPageNum, QUndoCommand *parent) : QUndoCommand(parent)
{
  setText(MainWindow::tr("Remove Page"));
  widget = newWidget;
  pageNum = newPageNum;
  page = widget->m_currentDocument.pages[pageNum];
}

void RemovePageCommand::undo()
{
  widget->m_currentDocument.pages.insert(widget->m_currentDocument.pages.begin() + static_cast<long>(pageNum), page);
  widget->m_pageBuffer.insert(widget->m_pageBuffer.begin() + static_cast<long>(pageNum), QPixmap());
  widget->updateBuffer(pageNum);
  widget->update();
}

void RemovePageCommand::redo()
{
  widget->m_currentDocument.pages.erase(widget->m_currentDocument.pages.begin() + static_cast<long>(pageNum));
  widget->m_pageBuffer.erase(widget->m_pageBuffer.begin() + static_cast<long>(pageNum));
  widget->update();
}

/******************************************************************************
** RemovePageCommand
*/

PasteCommand::PasteCommand(Widget *newWidget, const MrDoc::Selection & newSelection, QUndoCommand *parent) : QUndoCommand(parent)
{
  setText(MainWindow::tr("Paste"));
  widget = newWidget;
  pasteSelection = newSelection;
  previousSelection = widget->m_currentSelection;
  previousState = widget->getCurrentState();
}

void PasteCommand::undo()
{
  widget->m_currentSelection = previousSelection;
  widget->setCurrentState(previousState);
  widget->update();
}

void PasteCommand::redo()
{
  widget->m_currentSelection = pasteSelection;
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
  previousSelection = widget->m_currentSelection;
  previousState = widget->getCurrentState();
}

void CutCommand::undo()
{
  widget->m_currentSelection = previousSelection;
  widget->setCurrentState(previousState);
}

void CutCommand::redo()
{
  widget->clipboard = previousSelection;
  widget->m_currentSelection = MrDoc::Selection();
  widget->setCurrentState(Widget::state::IDLE);
}

/******************************************************************************
** DeleteCommand
*/

DeleteCommand::DeleteCommand(Widget *newWidget, QUndoCommand *parent) : QUndoCommand(parent)
{
  setText(MainWindow::tr("Delete"));
  widget = newWidget;
  previousSelection = widget->m_currentSelection;
  previousState = widget->getCurrentState();
}

void DeleteCommand::undo()
{
  widget->m_currentSelection = previousSelection;
  widget->setCurrentState(previousState);
}

void DeleteCommand::redo()
{
  widget->m_currentSelection = MrDoc::Selection();
  widget->setCurrentState(Widget::state::IDLE);
}

/******************************************************************************
** ChangePageSettingsCommand
*/

ChangePageSettingsCommand::ChangePageSettingsCommand(Widget *newWidget, size_t newPageNum, QSizeF newSize, QColor newBackgroundColor, QUndoCommand *parent)
    : QUndoCommand(parent)
{
  widget = newWidget;
  pageNum = newPageNum;
  prevSize = QSizeF(widget->m_currentDocument.pages[pageNum].width(), widget->m_currentDocument.pages[pageNum].height());
  size = newSize;
  prevBackgroundColor = widget->m_currentDocument.pages[pageNum].backgroundColor();
  backgroundColor = newBackgroundColor;
}

void ChangePageSettingsCommand::undo()
{
  qreal width = prevSize.width();
  qreal height = prevSize.height();
  widget->m_currentDocument.pages[pageNum].setWidth(width);
  widget->m_currentDocument.pages[pageNum].setHeight(height);
  widget->m_currentDocument.pages[pageNum].setBackgroundColor(prevBackgroundColor);
  widget->updateBuffer(pageNum);
  widget->setGeometry(widget->getWidgetGeometry());
}

void ChangePageSettingsCommand::redo()
{
  qreal width = size.width();
  qreal height = size.height();
  widget->m_currentDocument.pages[pageNum].setWidth(width);
  widget->m_currentDocument.pages[pageNum].setHeight(height);
  widget->m_currentDocument.pages[pageNum].setBackgroundColor(backgroundColor);
  widget->updateBuffer(pageNum);
  widget->setGeometry(widget->getWidgetGeometry());
}
