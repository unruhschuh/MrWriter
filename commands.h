#ifndef COMMANDS_H
#define COMMANDS_H

#include <QUndoCommand>
#include "widget.h"
#include "mrdoc.h"
#include "page.h"

class AddStrokeCommand : public QUndoCommand
{
public:
  AddStrokeCommand(Widget *widget, int pageNum, const MrDoc::Stroke &stroke, int strokeNum = -1, bool update = true,
                   bool updateSuccessive = true, QUndoCommand *parent = 0);
  void undo() Q_DECL_OVERRIDE;
  void redo() Q_DECL_OVERRIDE;

private:
  Widget *m_widget;
  MrDoc::Stroke m_stroke;
  int m_strokeNum;
  int m_pageNum;
  bool m_update;
  bool m_updateSuccessive;
};

class RemoveStrokeCommand : public QUndoCommand
{
public:
  RemoveStrokeCommand(Widget *widget, int pageNum, int strokeNum, bool update = true, QUndoCommand *parent = 0);
  void undo() Q_DECL_OVERRIDE;
  void redo() Q_DECL_OVERRIDE;

private:
  Widget *m_widget;
  MrDoc::Stroke m_stroke;
  int m_strokeNum;
  int m_pageNum;
  bool m_update;
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
  ReleaseSelectionCommand(Widget *widget, int pageNum, QUndoCommand *parent = 0);
  void undo() Q_DECL_OVERRIDE;
  void redo() Q_DECL_OVERRIDE;

private:
  Widget *m_widget;
  MrDoc::Selection m_selection;
  int m_pageNum;
};

class TransformSelectionCommand : public QUndoCommand
{
public:
  TransformSelectionCommand(Widget *widget, int pageNum, QTransform transform, QUndoCommand *parent = 0);
  void undo() Q_DECL_OVERRIDE;
  void redo() Q_DECL_OVERRIDE;
  int id() const Q_DECL_OVERRIDE
  {
    return 1;
  }
  bool mergeWith(const QUndoCommand *other) Q_DECL_OVERRIDE;

private:
  Widget *m_widget;
  MrDoc::Selection m_selection;
  QTransform m_transform;
  int m_pageNum;
};

class ChangeColorOfSelectionCommand : public QUndoCommand
{
public:
  ChangeColorOfSelectionCommand(Widget *widget, QColor color, QUndoCommand *parent = 0);
  void undo() Q_DECL_OVERRIDE;
  void redo() Q_DECL_OVERRIDE;

private:
  Widget *m_widget;
  MrDoc::Selection m_selection;
  QColor m_color;
};

class ChangePatternOfSelectionCommand : public QUndoCommand
{
public:
  ChangePatternOfSelectionCommand(Widget *widget, QVector<qreal> pattern, QUndoCommand *parent = 0);
  void undo() Q_DECL_OVERRIDE;
  void redo() Q_DECL_OVERRIDE;

private:
  Widget *m_widget;
  MrDoc::Selection m_selection;
  QVector<qreal> m_pattern;
};

class ChangePenWidthOfSelectionCommand : public QUndoCommand
{
public:
  ChangePenWidthOfSelectionCommand(Widget *widget, qreal penWidth, QUndoCommand *parent = 0);
  void undo() Q_DECL_OVERRIDE;
  void redo() Q_DECL_OVERRIDE;

private:
  Widget *m_widget;
  MrDoc::Selection m_selection;
  qreal m_penWidth;
};

class AddPageCommand : public QUndoCommand
{
public:
  AddPageCommand(Widget *widget, int pageNum, QUndoCommand *parent = 0);
  void undo() Q_DECL_OVERRIDE;
  void redo() Q_DECL_OVERRIDE;

private:
  Widget *m_widget;
  MrDoc::Page m_page;
  int m_pageNum;
};

class RemovePageCommand : public QUndoCommand
{
public:
  RemovePageCommand(Widget *widget, int pageNum, QUndoCommand *parent = 0);
  void undo() Q_DECL_OVERRIDE;
  void redo() Q_DECL_OVERRIDE;

private:
  Widget *m_widget;
  MrDoc::Page m_page;
  int m_pageNum;
};

class PasteCommand : public QUndoCommand
{
public:
  PasteCommand(Widget *widget, MrDoc::Selection selection, QUndoCommand *parent = 0);
  void undo() Q_DECL_OVERRIDE;
  void redo() Q_DECL_OVERRIDE;

private:
  Widget *m_widget;
  MrDoc::Selection m_pasteSelection;
  MrDoc::Selection m_previousSelection;
  Widget::state m_previousState;
};

class CutCommand : public QUndoCommand
{
public:
  CutCommand(Widget *widget, QUndoCommand *parent = 0);
  void undo() Q_DECL_OVERRIDE;
  void redo() Q_DECL_OVERRIDE;

private:
  Widget *m_widget;
  MrDoc::Selection m_previousSelection;
  Widget::state m_previousState;
};

class ChangePageSettingsCommand : public QUndoCommand
{
public:
  ChangePageSettingsCommand(Widget *widget, int pageNum, QSizeF size, QColor backgroundColor, QUndoCommand *parent = 0);
  void undo() Q_DECL_OVERRIDE;
  void redo() Q_DECL_OVERRIDE;

private:
  Widget *m_widget;
  int m_pageNum;
  QSizeF m_prevSize;
  QSizeF m_size;
  QColor m_prevBackgroundColor;
  QColor m_backgroundColor;
};

#endif
