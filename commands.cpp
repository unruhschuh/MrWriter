/*
#####################################################################
Copyright (C) 2015 Thomas Leitz (thomas.leitz@web.de)
#####################################################################

LICENSE:

This file is part of MrWriter.

MrWriter is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License 2.0 as published
by the Free Software Foundation.

MrWriter is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with MrWriter.  If not, see <http://www.gnu.org/licenses/>.
#####################################################################
*/

#include "commands.h"
#include "mainwindow.h"

/******************************************************************************
** AddStrokeCommand
*/

AddStrokeCommand::AddStrokeCommand(Widget *newWidget, int newPageNum, const MrDoc::Stroke &newStroke, int newStrokeNum, bool newUpdate, bool newUpdateSuccessive, QUndoCommand *parent) : QUndoCommand(parent)
{
    setText(MainWindow::tr("Add Stroke"));
    pageNum = newPageNum;
    widget = newWidget;
    stroke = newStroke;
    strokeNum = newStrokeNum;
    update = newUpdate;
    updateSuccessive = newUpdateSuccessive;

    // delete duplicate points
    for (int i = stroke.points.length()-1; i > 0; --i)
    {
        if (stroke.points.at(i) == stroke.points.at(i-1))
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
//            widget->currentDocument->pages[pageNum].m_strokes.removeLast();
            widget->currentDocument->pages[pageNum].removeStrokeAt(widget->currentDocument->pages[pageNum].m_strokes.size()-1);
        } else {
//            widget->currentDocument->pages[pageNum].m_strokes.removeAt(strokeNum);
            widget->currentDocument->pages[pageNum].removeStrokeAt(strokeNum);
        }
        if (update)
        {
//            widget->updateBuffer(pageNum);
//            widget->update();
        }
    }
}

void AddStrokeCommand::redo()
{
    if (stroke.points.length() > 0)
    {
        if (strokeNum == -1)
        {
//            widget->currentDocument->pages[pageNum].m_strokes.append(stroke);
            widget->currentDocument->pages[pageNum].appendStroke(stroke);
        } else {
//            widget->currentDocument->pages[pageNum].m_strokes.insert(strokeNum, stroke);
            widget->currentDocument->pages[pageNum].insertStroke(strokeNum, stroke);
        }
        if (update)
        {
//            widget->updateBuffer(pageNum);
//            widget->update();
        } else if (updateSuccessive) {
            update = true; // update is turned off for the first redo
        }
    }
}

/******************************************************************************
** RemoveStrokeCommand
*/

RemoveStrokeCommand::RemoveStrokeCommand(Widget *newWidget, int newPageNum, int newStrokeNum, bool newUpdate, QUndoCommand *parent) : QUndoCommand(parent)
{
    setText(MainWindow::tr("Remove Stroke"));
    pageNum = newPageNum;
    widget = newWidget;
    strokeNum = newStrokeNum;
    stroke = widget->currentDocument->pages[pageNum].m_strokes[strokeNum];
    update = newUpdate;
}

void RemoveStrokeCommand::undo()
{
//    widget->currentDocument->pages[pageNum].m_strokes.insert(strokeNum, stroke);
    widget->currentDocument->pages[pageNum].insertStroke(strokeNum, stroke);

    qreal zoom = widget->zoom;
    QRect updateRect = stroke.points.boundingRect().toRect();
    updateRect = QRect(zoom * updateRect.topLeft(), zoom * updateRect.bottomRight());
    int delta = zoom * 10;
    updateRect.adjust(-delta,-delta, delta, delta);
    if (update)
    {
//        widget->updateBufferRegion(pageNum, updateRect);
//        widget->update();
    }
}

void RemoveStrokeCommand::redo()
{
//    widget->currentDocument->pages[pageNum].m_strokes.removeAt(strokeNum);
    widget->currentDocument->pages[pageNum].removeStrokeAt(strokeNum);

    qreal zoom = widget->zoom;
    QRect updateRect = stroke.points.boundingRect().toRect();
    updateRect = QRect(zoom * updateRect.topLeft(), zoom * updateRect.bottomRight());
    int delta = zoom * 10;
    updateRect.adjust(-delta,-delta, delta, delta);
    if (update)
    {
//        widget->updateBufferRegion(pageNum, updateRect);
//        widget->update();
    }
}

/******************************************************************************
** CreateSelectionCommand
*/

//CreateSelectionCommand::CreateSelectionCommand(Widget *newWidget, int newPageNum, QVector<int> newStrokesInSelection, QUndoCommand *parent) : QUndoCommand(parent)
CreateSelectionCommand::CreateSelectionCommand(Widget *widget, int pageNum, QPolygonF selectionPolygon, QUndoCommand *parent) : QUndoCommand(parent)
{
    setText(MainWindow::tr("Create Selection"));
    m_widget = widget;
    m_pageNum = pageNum;

    m_strokesAndPositions = widget->currentDocument->pages[pageNum].getStrokes(selectionPolygon);

    m_selection = widget->currentSelection;
    for (auto sAndP : m_strokesAndPositions)
    {
        m_selection.prependStroke(sAndP.first);
    }
    m_selection.finalize();
    m_selection.updateBuffer(m_widget->zoom);
}

void CreateSelectionCommand::undo()
{
    m_widget->currentDocument->pages[m_pageNum].insertStrokes(m_strokesAndPositions);

    m_widget->setCurrentState(Widget::state::IDLE);
}

void CreateSelectionCommand::redo()
{
    m_widget->currentDocument->pages[m_pageNum].removeStrokes(m_selection.selectionPolygon);
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
    for (int i = 0; i < widget->currentSelection.m_strokes.size(); ++i)
    {
        widget->currentDocument->pages[pageNum].m_strokes.removeLast();
    }
    widget->setCurrentState(Widget::state::SELECTED);
//    widget->updateBuffer(pageNum);
//    widget->update();
}

void ReleaseSelectionCommand::redo()
{
    int pageNum = widget->currentSelection.pageNum;
//    for (int i = 0; i < widget->currentSelection.m_strokes.size(); ++i)
//    {
//        widget->currentDocument->pages[pageNum].m_strokes.append(widget->currentSelection.m_strokes.at(i));
//    }
    widget->currentDocument->pages[pageNum].appendStrokes(widget->currentSelection.m_strokes);
    widget->setCurrentState(Widget::state::IDLE);
//    widget->updateBuffer(pageNum);
//    widget->update();
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
//    widget->updateBuffer(pageNum);
//    widget->update();
//    widget->updateAllDirtyBuffers();
}

void TransformSelectionCommand::redo()
{
    QRect updateRect = widget->currentSelection.selectionPolygon.boundingRect().toRect();
    widget->currentSelection.transform(transform, pageNum);
//    widget->updateBuffer(pageNum);
    updateRect = updateRect.united(widget->currentSelection.selectionPolygon.boundingRect().toRect());
    QTransform scaleTrans;
    scaleTrans = scaleTrans.scale(widget->zoom, widget->zoom);
    updateRect = scaleTrans.mapRect(updateRect);
    updateRect.adjust(-1, -1, 1, 1);
//    widget->update(updateRect);
//    widget->updateAllDirtyBuffers();
}

bool TransformSelectionCommand::mergeWith(const QUndoCommand *other)
{
    if (other->id() != id())
        return false;
    transform *= static_cast<const TransformSelectionCommand*>(other)->transform;
    pageNum = static_cast<const TransformSelectionCommand*>(other)->pageNum;

    return true;
}


/******************************************************************************
** ChangeColorOfSelection
*/

ChangeColorOfSelectionCommand::ChangeColorOfSelectionCommand(Widget* newWidget, QColor newColor, QUndoCommand *parent) : QUndoCommand(parent)
{
    setText(MainWindow::tr("Change Color"));

    widget = newWidget;
    selection = widget->currentSelection;
    color = newColor;
}

void ChangeColorOfSelectionCommand::undo()
{
    widget->currentSelection = selection;
//    widget->updateBuffer(selection.pageNum);
//    widget->update();
}

void ChangeColorOfSelectionCommand::redo()
{
    for (int i = 0; i < widget->currentSelection.m_strokes.size(); ++i)
    {
        widget->currentSelection.m_strokes[i].color = color;
    }
//    widget->updateBuffer(selection.pageNum);
//    widget->update();
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
    widget->currentDocument->pages.removeAt(pageNum);
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
        pageNumForSettings = pageNum-1;

    page.setWidth(widget->currentDocument->pages[pageNumForSettings].getWidth());
    page.setHeight(widget->currentDocument->pages[pageNumForSettings].getHeight());
    page.backgroundColor = widget->currentDocument->pages[pageNumForSettings].backgroundColor;

    widget->currentDocument->pages.insert(pageNum, page);
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
    page = widget->currentDocument->pages[pageNum];
}

void RemovePageCommand::undo()
{
    widget->currentDocument->pages.insert(pageNum, page);
    widget->pageBuffer.insert(pageNum, QPixmap());
    widget->updateBuffer(pageNum);
    widget->update();
}

void RemovePageCommand::redo()
{
    widget->currentDocument->pages.removeAt(pageNum);
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
** CutPageCommand
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
    widget->update();
}

void CutCommand::redo()
{
    widget->clipboard = previousSelection;
    widget->currentSelection = MrDoc::Selection();
    widget->setCurrentState(Widget::state::IDLE);
    widget->update();
}


/******************************************************************************
** CutPageCommand
*/

ChangePageSettingsCommand::ChangePageSettingsCommand(Widget *newWidget, int newPageNum, QSizeF newSize, QColor newBackgroundColor, QUndoCommand *parent) : QUndoCommand(parent)
{
    widget = newWidget;
    pageNum = newPageNum;
    prevSize = QSizeF(widget->currentDocument->pages[pageNum].getWidth(), widget->currentDocument->pages[pageNum].getHeight());
    size = newSize;
    prevBackgroundColor = widget->currentDocument->pages[pageNum].getBackgroundColor();
    backgroundColor = newBackgroundColor;
}

void ChangePageSettingsCommand::undo()
{
    qreal width = prevSize.width();
    qreal height = prevSize.height();
    widget->currentDocument->pages[pageNum].setWidth(width);
    widget->currentDocument->pages[pageNum].setHeight(height);
    widget->currentDocument->pages[pageNum].setBackgroundColor(prevBackgroundColor);
    widget->updateBuffer(pageNum);
    widget->setGeometry(widget->getWidgetGeometry());
    widget->update();
}

void ChangePageSettingsCommand::redo()
{
    qreal width = size.width();
    qreal height = size.height();
    widget->currentDocument->pages[pageNum].setWidth(width);
    widget->currentDocument->pages[pageNum].setHeight(height);
    widget->currentDocument->pages[pageNum].setBackgroundColor(backgroundColor);
    widget->updateBuffer(pageNum);
    widget->setGeometry(widget->getWidgetGeometry());
    widget->update();
}
