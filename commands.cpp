#include "commands.h"
#include "mainwindow.h"

/******************************************************************************
** AddCurveCommand
*/

AddCurveCommand::AddCurveCommand(Widget *newWidget, int newPageNum, const Curve &newCurve, int newCurveNum, bool newUpdate, bool newUpdateSuccessive, QUndoCommand *parent) : QUndoCommand(parent)
{
    setText(MainWindow::tr("Add Curve"));
    pageNum = newPageNum;
    widget = newWidget;
    curve = newCurve;
    curveNum = newCurveNum;
    update = newUpdate;
    updateSuccessive = newUpdateSuccessive;
}

void AddCurveCommand::undo()
{
    if (curveNum == -1)
    {
        widget->currentDocument->pages[pageNum].curves.removeLast();
    } else {
        widget->currentDocument->pages[pageNum].curves.removeAt(curveNum);
    }
    if (update)
    {
        widget->updateBuffer(pageNum);
        widget->update();
    }
}

void AddCurveCommand::redo()
{
    if (curveNum == -1)
    {
        widget->currentDocument->pages[pageNum].curves.append(curve);
    } else {
        widget->currentDocument->pages[pageNum].curves.insert(curveNum, curve);
    }
    if (update)
    {
        widget->updateBuffer(pageNum);
        widget->update();
    } else if (updateSuccessive) {
        update = true; // update is turned off for the first redo
    }
}

/******************************************************************************
** RemoveCurveCommand
*/

RemoveCurveCommand::RemoveCurveCommand(Widget *newWidget, int newPageNum, int newCurveNum, bool newUpdate, QUndoCommand *parent) : QUndoCommand(parent)
{
    setText(MainWindow::tr("Remove Curve"));
    pageNum = newPageNum;
    widget = newWidget;
    curveNum = newCurveNum;
    curve = widget->currentDocument->pages[pageNum].curves[curveNum];
    update = newUpdate;
}

void RemoveCurveCommand::undo()
{
    widget->currentDocument->pages[pageNum].curves.insert(curveNum, curve);
    qreal zoom = widget->zoom;
    QRect updateRect = curve.points.boundingRect().toRect();
    updateRect = QRect(zoom * updateRect.topLeft(), zoom * updateRect.bottomRight());
    int delta = zoom * 10;
    updateRect.adjust(-delta,-delta, delta, delta);
    if (update)
    {
        widget->updateBufferRegion(pageNum, updateRect);
        widget->update();
    }
}

void RemoveCurveCommand::redo()
{
    widget->currentDocument->pages[pageNum].curves.removeAt(curveNum);

    qreal zoom = widget->zoom;
    QRect updateRect = curve.points.boundingRect().toRect();
    updateRect = QRect(zoom * updateRect.topLeft(), zoom * updateRect.bottomRight());
    int delta = zoom * 10;
    updateRect.adjust(-delta,-delta, delta, delta);
    if (update)
    {
        widget->updateBufferRegion(pageNum, updateRect);
        widget->update();
    }
}

/******************************************************************************
** CreateSelectionCommand
*/

CreateSelectionCommand::CreateSelectionCommand(Widget *newWidget, int newPageNum, QVector<int> newCurvesInSelection, QUndoCommand *parent) : QUndoCommand(parent)
{
    setText(MainWindow::tr("Create Selection"));
    widget = newWidget;
    pageNum = newPageNum;
    curvesInSelection = newCurvesInSelection;
    selection = widget->currentSelection;
}

void CreateSelectionCommand::undo()
{
    for (int i = 0; i < curvesInSelection.size(); ++i)
    {
        widget->currentDocument->pages[pageNum].curves.insert(curvesInSelection[i], selection.curves[i]);
    }
    widget->setCurrentState(Widget::state::IDLE);
    widget->updateBuffer(pageNum);
    widget->update();
}

void CreateSelectionCommand::redo()
{
    for (int i = curvesInSelection.size()-1; i >= 0; --i)
    {
        widget->currentDocument->pages[pageNum].curves.removeAt(curvesInSelection[i]);
    }
    widget->currentSelection = selection;
    widget->setCurrentState(Widget::state::SELECTED);
    widget->updateBuffer(pageNum);
    widget->update();
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
    for (int i = 0; i < widget->currentSelection.curves.size(); ++i)
    {
        widget->currentDocument->pages[pageNum].curves.removeLast();
    }
    widget->setCurrentState(Widget::state::SELECTED);
    widget->updateBuffer(pageNum);
    widget->update();
}

void ReleaseSelectionCommand::redo()
{
    int pageNum = widget->currentSelection.pageNum;
    for (int i = 0; i < widget->currentSelection.curves.size(); ++i)
    {
        widget->currentDocument->pages[pageNum].curves.append(widget->currentSelection.curves.at(i));
    }
    widget->setCurrentState(Widget::state::IDLE);
    widget->updateBuffer(pageNum);
    widget->update();
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
    widget->updateBuffer(pageNum);
    widget->update();
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
    widget->update();
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
    widget->updateBuffer(selection.pageNum);
    widget->update();
}

void ChangeColorOfSelectionCommand::redo()
{
    for (int i = 0; i < widget->currentSelection.curves.size(); ++i)
    {
        widget->currentSelection.curves[i].color = color;
    }
    widget->updateBuffer(selection.pageNum);
    widget->update();
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
    page = Page();
    int pageNumForSettings;

    if (pageNum == 0)
        pageNumForSettings = pageNum;
    else
        pageNumForSettings = pageNum-1;

    page.setWidth(widget->currentDocument->pages[pageNumForSettings].getWidth());
    page.setHeight(widget->currentDocument->pages[pageNumForSettings].getHeight());
    page.backgroundColor = widget->currentDocument->pages[pageNumForSettings].backgroundColor;

    widget->currentDocument->pages.insert(pageNum, page);
    widget->pageBuffer.insert(pageNum, QImage());
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
    widget->pageBuffer.insert(pageNum, QImage());
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

PasteCommand::PasteCommand(Widget *newWidget, Selection newSelection, QUndoCommand *parent) : QUndoCommand(parent)
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
    widget->currentSelection = Selection();
    widget->setCurrentState(Widget::state::IDLE);
    widget->update();
}

