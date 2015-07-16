#ifndef COMMANDS_H
#define COMMANDS_H

#include <QUndoCommand>
#include "document.h"
#include "widget.h"
#include "page.h"
#include "curve.h"

class AddCurveCommand : public QUndoCommand
{
public:
    AddCurveCommand(Widget *newWidget, int newPageNum, const Curve &newCurve, int newCurveNum = -1, bool newUpdate = true, bool newUpdateSuccessive = true, QUndoCommand *parent = 0);
    void undo() Q_DECL_OVERRIDE;
    void redo() Q_DECL_OVERRIDE;

private:
    Widget *widget;
    Curve curve;
    int curveNum;
    int pageNum;
    bool update;
    bool updateSuccessive;
};

class RemoveCurveCommand : public QUndoCommand
{
public:
    RemoveCurveCommand(Widget *newWidget, int newPageNum, int newCurveNum, bool newUpdate = true, QUndoCommand *parent = 0);
    void undo() Q_DECL_OVERRIDE;
    void redo() Q_DECL_OVERRIDE;

private:
    Widget *widget;
    Curve curve;
    int curveNum;
    int pageNum;
    bool update;
};

class CreateSelectionCommand : public QUndoCommand
{
public:
    CreateSelectionCommand(Widget *newWidget, int newPageNum, QVector<int> newCurvesInSelection, QUndoCommand *parent = 0);
    void undo() Q_DECL_OVERRIDE;
    void redo() Q_DECL_OVERRIDE;

private:
    Widget* widget;
    QVector<int> curvesInSelection;
    Selection selection;
    int pageNum;
};

class ReleaseSelectionCommand : public QUndoCommand
{
public:
    ReleaseSelectionCommand(Widget *newWidget, int newPageNum, QUndoCommand *parent = 0);
    void undo() Q_DECL_OVERRIDE;
    void redo() Q_DECL_OVERRIDE;

private:
    Widget* widget;
    Selection selection;
    int pageNum;
};

class TransformSelectionCommand : public QUndoCommand
{
public:
    TransformSelectionCommand(Widget* newWidget, int newPageNum, QTransform newTransform, QUndoCommand *parent = 0);
    void undo() Q_DECL_OVERRIDE;
    void redo() Q_DECL_OVERRIDE;
    int id() const Q_DECL_OVERRIDE { return 1; }
    bool mergeWith(const QUndoCommand *other) Q_DECL_OVERRIDE;

private:
    Widget* widget;
    Selection selection;
    QTransform transform;
    int pageNum;
};

class ChangeColorOfSelectionCommand : public QUndoCommand
{
public:
    ChangeColorOfSelectionCommand(Widget* newWidget, QColor newColor, QUndoCommand *parent = 0);
    void undo() Q_DECL_OVERRIDE;
    void redo() Q_DECL_OVERRIDE;

private:
    Widget* widget;
    Selection selection;
    QColor color;
};

class AddPageCommand : public QUndoCommand
{
public:
    AddPageCommand(Widget* newWidget, int newPageNum, QUndoCommand *parent = 0);
    void undo() Q_DECL_OVERRIDE;
    void redo() Q_DECL_OVERRIDE;

private:
    Widget* widget;
    Page page;
    int pageNum;
};

class RemovePageCommand : public QUndoCommand
{
public:
    RemovePageCommand(Widget* newWidget, int newPageNum, QUndoCommand *parent = 0);
    void undo() Q_DECL_OVERRIDE;
    void redo() Q_DECL_OVERRIDE;

private:
    Widget* widget;
    Page page;
    int pageNum;
};

class PasteCommand : public QUndoCommand
{
public:
    PasteCommand(Widget* newWidget, Selection newSelection, QUndoCommand *parent = 0);
    void undo() Q_DECL_OVERRIDE;
    void redo() Q_DECL_OVERRIDE;

private:
    Widget* widget;
    Selection pasteSelection;
    Selection previousSelection;
    Widget::state previousState;
};

class CutCommand : public QUndoCommand
{
public:
    CutCommand(Widget* newWidget, QUndoCommand *parent = 0);
    void undo() Q_DECL_OVERRIDE;
    void redo() Q_DECL_OVERRIDE;

private:
    Widget* widget;
    Selection previousSelection;
    Widget::state previousState;
};

#endif
