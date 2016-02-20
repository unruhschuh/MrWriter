#include "commands.h"
#include "mainwindow.h"
#include <QDebug>

/******************************************************************************
** AddStrokeCommand
*/

/**
 * @brief AddStrokeCommand::AddStrokeCommand
 * @param widget
 * @param pageNum
 * @param stroke
 * @param strokeNum
 * @param update
 * @param updateSuccessive
 * @param parent
 * @todo remove parameter update
 */
AddStrokeCommand::AddStrokeCommand(Widget *widget, int pageNum, const MrDoc::Stroke &stroke, int strokeNum, bool update,
                                   bool updateSuccessive, QUndoCommand *parent)
    : QUndoCommand(parent)
{
  setText(MainWindow::tr("Add Stroke"));
  m_pageNum = pageNum;
  m_widget = widget;
  m_stroke = stroke;
  m_strokeNum = strokeNum;
  m_update = update;
  m_updateSuccessive = updateSuccessive;

  // delete duplicate points
  for (int i = m_stroke.m_points.length() - 1; i > 0; --i)
  {
    if (m_stroke.m_points.at(i) == m_stroke.m_points.at(i - 1))
    {
      m_stroke.m_points.removeAt(i);
      m_stroke.m_pressures.removeAt(i);
    }
  }
}

void AddStrokeCommand::undo()
{
  if (m_stroke.m_points.length() > 0)
  {
    if (m_strokeNum == -1)
    {
      m_widget->m_currentDocument.pages[m_pageNum].removeStrokeAt(m_widget->m_currentDocument.pages[m_pageNum].strokes().size() - 1);
    }
    else
    {
      m_widget->m_currentDocument.pages[m_pageNum].removeStrokeAt(m_strokeNum);
    }
  }
}

void AddStrokeCommand::redo()
{
  if (m_stroke.m_points.length() > 0)
  {
    if (m_strokeNum == -1)
    {
      m_widget->m_currentDocument.pages[m_pageNum].appendStroke(m_stroke);
    }
    else
    {
      m_widget->m_currentDocument.pages[m_pageNum].insertStroke(m_strokeNum, m_stroke);
    }
  }
}

/******************************************************************************
** RemoveStrokeCommand
*/

/**
 * @brief RemoveStrokeCommand::RemoveStrokeCommand
 * @param widget
 * @param pageNum
 * @param strokeNum
 * @param update
 * @param parent
 * @todo remove parameter update
 */
RemoveStrokeCommand::RemoveStrokeCommand(Widget *widget, int pageNum, int strokeNum, bool update, QUndoCommand *parent) : QUndoCommand(parent)
{
  setText(MainWindow::tr("Remove Stroke"));
  m_pageNum = pageNum;
  m_widget = widget;
  m_strokeNum = strokeNum;
  m_stroke = m_widget->m_currentDocument.pages[m_pageNum].strokes()[m_strokeNum];
  m_update = update;
}

void RemoveStrokeCommand::undo()
{
  m_widget->m_currentDocument.pages[m_pageNum].insertStroke(m_strokeNum, m_stroke);

  qreal zoom = m_widget->m_zoom;
  QRect updateRect = m_stroke.m_points.boundingRect().toRect();
  updateRect = QRect(zoom * updateRect.topLeft(), zoom * updateRect.bottomRight());
  int delta = zoom * 10;
  updateRect.adjust(-delta, -delta, delta, delta);
}

void RemoveStrokeCommand::redo()
{
  m_widget->m_currentDocument.pages[m_pageNum].removeStrokeAt(m_strokeNum);

  qreal zoom = m_widget->m_zoom;
  QRect updateRect = m_stroke.m_points.boundingRect().toRect();
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

  m_strokesAndPositions = widget->m_currentDocument.pages[pageNum].getStrokes(m_selectionPolygon);

  for (auto sAndP : m_strokesAndPositions)
  {
    m_selection.prependStroke(sAndP.first);
  }
  m_selection.finalize();
  m_selection.updateBuffer(m_widget->m_zoom);
}

void CreateSelectionCommand::undo()
{
  m_widget->m_currentDocument.pages[m_pageNum].insertStrokes(m_strokesAndPositions);

  m_widget->setCurrentState(Widget::state::IDLE);
}

void CreateSelectionCommand::redo()
{
  for (auto &sAndP : m_strokesAndPositions)
  {
    m_widget->m_currentDocument.pages[m_pageNum].removeStrokeAt(sAndP.second);
  }
  m_widget->currentSelection = m_selection;
  m_widget->setCurrentState(Widget::state::SELECTED);
}

/******************************************************************************
** ReleaseSelectionCommand
*/

ReleaseSelectionCommand::ReleaseSelectionCommand(Widget *widget, int pageNum, QUndoCommand *parent) : QUndoCommand(parent)
{
  setText(MainWindow::tr("Release Selection"));

  m_widget = widget;
  m_selection = m_widget->currentSelection;
  m_pageNum = pageNum;
}

void ReleaseSelectionCommand::undo()
{
  m_widget->currentSelection = m_selection;
  for (int i = 0; i < m_widget->currentSelection.strokes().size(); ++i)
  {
    m_widget->m_currentDocument.pages[m_pageNum].removeLastStroke();
  }
  m_widget->setCurrentState(Widget::state::SELECTED);
}

void ReleaseSelectionCommand::redo()
{
  int pageNum = m_widget->currentSelection.pageNum();
  m_widget->m_currentDocument.pages[pageNum].appendStrokes(m_widget->currentSelection.strokes());
  m_widget->setCurrentState(Widget::state::IDLE);
}

/******************************************************************************
** TransformSelectionCommand
*/

TransformSelectionCommand::TransformSelectionCommand(Widget *widget, int pageNum, QTransform transform, QUndoCommand *parent) : QUndoCommand(parent)
{
  setText(MainWindow::tr("Transform Selection"));

  m_widget = widget;
  m_pageNum = pageNum;
  m_selection = m_widget->currentSelection;
  m_transform = transform;
}

void TransformSelectionCommand::undo()
{
  m_widget->currentSelection = m_selection;
}

void TransformSelectionCommand::redo()
{
  m_widget->currentSelection.transform(m_transform, m_pageNum);
}

bool TransformSelectionCommand::mergeWith(const QUndoCommand *other)
{
  if (other->id() != id())
    return false;
  m_transform *= static_cast<const TransformSelectionCommand *>(other)->m_transform;
  m_pageNum = static_cast<const TransformSelectionCommand *>(other)->m_pageNum;

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

ChangePenWidthOfSelectionCommand::ChangePenWidthOfSelectionCommand(Widget *widget, qreal penWidth, QUndoCommand *parent) : QUndoCommand(parent)
{
  setText(MainWindow::tr("Change Pen Width"));

  m_widget = widget;
  m_selection = m_widget->currentSelection;
  m_penWidth = penWidth;
}

void ChangePenWidthOfSelectionCommand::undo()
{
  m_widget->currentSelection = m_selection;
}

void ChangePenWidthOfSelectionCommand::redo()
{
  for (int i = 0; i < m_widget->currentSelection.strokes().size(); ++i)
  {
    m_widget->currentSelection.changePenWidth(i, m_penWidth);
  }
}

/******************************************************************************
** AddPageCommand
*/

AddPageCommand::AddPageCommand(Widget *widget, int pageNum, QUndoCommand *parent) : QUndoCommand(parent)
{
  setText(MainWindow::tr("Add Page"));
  m_widget = widget;
  m_pageNum = pageNum;
}

void AddPageCommand::undo()
{
  m_widget->m_currentDocument.pages.removeAt(m_pageNum);
  m_widget->m_pageBuffer.removeAt(m_pageNum);
  m_widget->update();
}

void AddPageCommand::redo()
{
  m_page = MrDoc::Page();
  int pageNumForSettings;

  if (m_pageNum == 0)
    pageNumForSettings = m_pageNum;
  else
    pageNumForSettings = m_pageNum - 1;

  m_page.setWidth(m_widget->m_currentDocument.pages[pageNumForSettings].width());
  m_page.setHeight(m_widget->m_currentDocument.pages[pageNumForSettings].height());
  m_page.setBackgroundColor(m_widget->m_currentDocument.pages[pageNumForSettings].backgroundColor());

  m_widget->m_currentDocument.pages.insert(m_pageNum, m_page);
  m_widget->m_pageBuffer.insert(m_pageNum, QPixmap());
  m_widget->updateBuffer(m_pageNum);
  m_widget->update();
}

/******************************************************************************
** RemovePageCommand
*/

RemovePageCommand::RemovePageCommand(Widget *widget, int pageNum, QUndoCommand *parent) : QUndoCommand(parent)
{
  setText(MainWindow::tr("Remove Page"));
  m_widget = widget;
  m_pageNum = pageNum;
  m_page = m_widget->m_currentDocument.pages[m_pageNum];
}

void RemovePageCommand::undo()
{
  m_widget->m_currentDocument.pages.insert(m_pageNum, m_page);
  m_widget->m_pageBuffer.insert(m_pageNum, QPixmap());
  m_widget->updateBuffer(m_pageNum);
  m_widget->update();
}

void RemovePageCommand::redo()
{
  m_widget->m_currentDocument.pages.removeAt(m_pageNum);
  m_widget->m_pageBuffer.removeAt(m_pageNum);
  m_widget->update();
}

/******************************************************************************
** RemovePageCommand
*/

PasteCommand::PasteCommand(Widget *widget, MrDoc::Selection selection, QUndoCommand *parent) : QUndoCommand(parent)
{
  setText(MainWindow::tr("Paste"));
  m_widget = widget;
  m_pasteSelection = selection;
  m_previousSelection = m_widget->currentSelection;
  m_previousState = m_widget->getCurrentState();
}

void PasteCommand::undo()
{
  m_widget->currentSelection = m_previousSelection;
  m_widget->setCurrentState(m_previousState);
  m_widget->update();
}

void PasteCommand::redo()
{
  m_widget->currentSelection = m_pasteSelection;
  m_widget->setCurrentState(Widget::state::SELECTED);
  m_widget->update();
}

/******************************************************************************
** CutCommand
*/

CutCommand::CutCommand(Widget *widget, QUndoCommand *parent) : QUndoCommand(parent)
{
  setText(MainWindow::tr("Cut"));
  m_widget = widget;
  m_previousSelection = m_widget->currentSelection;
  m_previousState = m_widget->getCurrentState();
}

void CutCommand::undo()
{
  m_widget->currentSelection = m_previousSelection;
  m_widget->setCurrentState(m_previousState);
}

void CutCommand::redo()
{
  m_widget->clipboard = m_previousSelection;
  m_widget->currentSelection = MrDoc::Selection();
  m_widget->setCurrentState(Widget::state::IDLE);
}

/******************************************************************************
** ChangePageSettingsCommand
*/

ChangePageSettingsCommand::ChangePageSettingsCommand(Widget *widget, int pageNum, QSizeF size, QColor backgroundColor, QUndoCommand *parent)
    : QUndoCommand(parent)
{
  m_widget = widget;
  m_pageNum = pageNum;
  m_prevSize = QSizeF(m_widget->m_currentDocument.pages[m_pageNum].width(), m_widget->m_currentDocument.pages[m_pageNum].height());
  m_size = size;
  m_prevBackgroundColor = m_widget->m_currentDocument.pages[m_pageNum].backgroundColor();
  m_backgroundColor = backgroundColor;
}

void ChangePageSettingsCommand::undo()
{
  qreal width = m_prevSize.width();
  qreal height = m_prevSize.height();
  m_widget->m_currentDocument.pages[m_pageNum].setWidth(width);
  m_widget->m_currentDocument.pages[m_pageNum].setHeight(height);
  m_widget->m_currentDocument.pages[m_pageNum].setBackgroundColor(m_prevBackgroundColor);
  m_widget->updateBuffer(m_pageNum);
  m_widget->setGeometry(m_widget->getWidgetGeometry());
}

void ChangePageSettingsCommand::redo()
{
  qreal width = m_size.width();
  qreal height = m_size.height();
  m_widget->m_currentDocument.pages[m_pageNum].setWidth(width);
  m_widget->m_currentDocument.pages[m_pageNum].setHeight(height);
  m_widget->m_currentDocument.pages[m_pageNum].setBackgroundColor(m_backgroundColor);
  m_widget->updateBuffer(m_pageNum);
  m_widget->setGeometry(m_widget->getWidgetGeometry());
}
