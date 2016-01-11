#ifndef COMMANDS_H
#define COMMANDS_H

#include <QUndoCommand>
#include "widget.h"
#include "mrdoc.h"
#include "page.h"

class AddStrokeCommand : public QUndoCommand
{
public:
  AddStrokeCommand(Widget *newWidget, int newPageNum, const MrDoc::Stroke &newStroke, int newStrokeNum = -1, bool newUpdate = true,
                   bool newUpdateSuccessive = true, QUndoCommand *parent = 0);
  void undo() Q_DECL_OVERRIDE;
  void redo() Q_DECL_OVERRIDE;

private:
  Widget *widget;
  MrDoc::Stroke stroke;
  int strokeNum;
  int pageNum;
  bool update;
  bool updateSuccessive;
};

class RemoveStrokeCommand : public QUndoCommand
{
public:
  RemoveStrokeCommand(Widget *newWidget, int newPageNum, int newStrokeNum, bool newUpdate = true, QUndoCommand *parent = 0);
  void undo() Q_DECL_OVERRIDE;
  void redo() Q_DECL_OVERRIDE;

private:
  Widget *widget;
  MrDoc::Stroke stroke;
  int strokeNum;
  int pageNum;
  bool update;
};

class CreateSelectionCommand : public QUndoCommand
{
public:
  CreateSelectionCommand(Widget *widget, int pageNum, MrDoc::Selection selection, QUndoCommand *parent = 0);
  void undo() Q_DECL_OVERRIDE;
  void redo() Q_DECL_OVERRIDE;

private:
  Widget *m_widget;
  QPolygonF m_selectionPolygon;
  QVector<QPair<MrDoc::Stroke, int>> m_strokesAndPositions;
  MrDoc::Selection m_selection;
  int m_pageNum;
};

class ReleaseSelectionCommand : public QUndoCommand
{
public:
  ReleaseSelectionCommand(Widget *newWidget, int newPageNum, QUndoCommand *parent = 0);
  void undo() Q_DECL_OVERRIDE;
  void redo() Q_DECL_OVERRIDE;

private:
  Widget *widget;
  MrDoc::Selection selection;
  int pageNum;
};

class TransformSelectionCommand : public QUndoCommand
{
public:
  TransformSelectionCommand(Widget *newWidget, int newPageNum, QTransform newTransform, QUndoCommand *parent = 0);
  void undo() Q_DECL_OVERRIDE;
  void redo() Q_DECL_OVERRIDE;
  int id() const Q_DECL_OVERRIDE
  {
    return 1;
  }
  bool mergeWith(const QUndoCommand *other) Q_DECL_OVERRIDE;

private:
  Widget *widget;
  MrDoc::Selection selection;
  QTransform transform;
  int pageNum;
};

class ChangeColorOfSelectionCommand : public QUndoCommand
{
public:
  ChangeColorOfSelectionCommand(Widget *newWidget, QColor newColor, QUndoCommand *parent = 0);
  void undo() Q_DECL_OVERRIDE;
  void redo() Q_DECL_OVERRIDE;

private:
  Widget *widget;
  MrDoc::Selection selection;
  QColor color;
};

class ChangePenWidthOfSelectionCommand : public QUndoCommand
{
public:
  ChangePenWidthOfSelectionCommand(Widget *newWidget, qreal penWidth, QUndoCommand *parent = 0);
  void undo() Q_DECL_OVERRIDE;
  void redo() Q_DECL_OVERRIDE;

private:
  Widget *widget;
  MrDoc::Selection selection;
  qreal m_penWidth;
};


class AddPageCommand : public QUndoCommand
{
public:
  AddPageCommand(Widget *newWidget, int newPageNum, QUndoCommand *parent = 0);
  void undo() Q_DECL_OVERRIDE;
  void redo() Q_DECL_OVERRIDE;

private:
  Widget *widget;
  MrDoc::Page page;
  int pageNum;
};

class RemovePageCommand : public QUndoCommand
{
public:
  RemovePageCommand(Widget *newWidget, int newPageNum, QUndoCommand *parent = 0);
  void undo() Q_DECL_OVERRIDE;
  void redo() Q_DECL_OVERRIDE;

private:
  Widget *widget;
  MrDoc::Page page;
  int pageNum;
};

class PasteCommand : public QUndoCommand
{
public:
  PasteCommand(Widget *newWidget, MrDoc::Selection newSelection, QUndoCommand *parent = 0);
  void undo() Q_DECL_OVERRIDE;
  void redo() Q_DECL_OVERRIDE;

private:
  Widget *widget;
  MrDoc::Selection pasteSelection;
  MrDoc::Selection previousSelection;
  Widget::state previousState;
};

class CutCommand : public QUndoCommand
{
public:
  CutCommand(Widget *newWidget, QUndoCommand *parent = 0);
  void undo() Q_DECL_OVERRIDE;
  void redo() Q_DECL_OVERRIDE;

private:
  Widget *widget;
  MrDoc::Selection previousSelection;
  Widget::state previousState;
};

class ChangePageSettingsCommand : public QUndoCommand
{
public:
  ChangePageSettingsCommand(Widget *newWidget, int newPageNum, QSizeF newSize, QColor newBackgroundColor, QUndoCommand *parent = 0);
  void undo() Q_DECL_OVERRIDE;
  void redo() Q_DECL_OVERRIDE;

private:
  Widget *widget;
  int pageNum;
  QSizeF prevSize;
  QSizeF size;
  QColor prevBackgroundColor;
  QColor backgroundColor;
};

#endif
