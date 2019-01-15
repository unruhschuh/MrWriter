#ifndef COMMANDS_H
#define COMMANDS_H

#include <QUndoCommand>
#include <memory>
#include "widget.h"
#include "mrdoc.h"
#include "page.h"

class AddElementCommand : public QUndoCommand
{
public:
  AddElementCommand(Widget *newWidget, int newPageNum, std::unique_ptr<MrDoc::Element> newElement, int newElementNum = -1, bool newUpdate = true,
                   bool newUpdateSuccessive = true, QUndoCommand *parent = nullptr);
  void undo() Q_DECL_OVERRIDE;
  void redo() Q_DECL_OVERRIDE;

private:
  Widget *widget;
  std::unique_ptr<MrDoc::Element> element;
  int elementNum;
  int pageNum;
  bool update;
  bool updateSuccessive;
};

class RemoveElementCommand : public QUndoCommand
{
public:
  RemoveElementCommand(Widget *newWidget, int newPageNum, int newElementNum, bool newUpdate = true, QUndoCommand *parent = nullptr);
  void undo() Q_DECL_OVERRIDE;
  void redo() Q_DECL_OVERRIDE;

private:
  Widget *widget;
  std::unique_ptr<MrDoc::Element> element;
  int elementNum;
  int pageNum;
  bool update;
};

class CreateSelectionCommand : public QUndoCommand
{
public:
  CreateSelectionCommand(Widget *widget, int pageNum, MrDoc::Selection selection, QUndoCommand *parent = nullptr);
  void undo() Q_DECL_OVERRIDE;
  void redo() Q_DECL_OVERRIDE;

private:
  Widget *m_widget;
  QPolygonF m_selectionPolygon;
  std::vector<QPair<std::unique_ptr<MrDoc::Element>, size_t>> m_elementsAndPositions;
  MrDoc::Selection m_selection;
  int m_pageNum;
};

class ReleaseSelectionCommand : public QUndoCommand
{
public:
  ReleaseSelectionCommand(Widget *newWidget, int newPageNum, QUndoCommand *parent = nullptr);
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
  TransformSelectionCommand(Widget *newWidget, int newPageNum, QTransform newTransform, QUndoCommand *parent = nullptr);
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
  ChangeColorOfSelectionCommand(Widget *widget, QColor color, QUndoCommand *parent = nullptr);
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
  ChangePatternOfSelectionCommand(Widget *widget, QVector<qreal> pattern, QUndoCommand *parent = nullptr);
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
  ChangePenWidthOfSelectionCommand(Widget *newWidget, qreal penWidth, QUndoCommand *parent = nullptr);
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
  AddPageCommand(Widget *newWidget, int newPageNum, QUndoCommand *parent = nullptr);
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
  RemovePageCommand(Widget *newWidget, int newPageNum, QUndoCommand *parent = nullptr);
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
  PasteCommand(Widget *newWidget, MrDoc::Selection newSelection, QUndoCommand *parent = nullptr);
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
  CutCommand(Widget *newWidget, QUndoCommand *parent = nullptr);
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
  ChangePageSettingsCommand(Widget *newWidget, int newPageNum, QSizeF newSize, QColor newBackgroundColor, QUndoCommand *parent = nullptr);
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
