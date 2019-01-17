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
  AddElementCommand(Widget *newWidget, size_t newPageNum, std::unique_ptr<MrDoc::Element> newElement, bool newAtEnd = true, size_t newElementNum = 0, bool newUpdate = true, bool newUpdateSuccessive = true, QUndoCommand *parent = nullptr);
  void undo() Q_DECL_OVERRIDE;
  void redo() Q_DECL_OVERRIDE;

private:
  Widget *widget;
  std::unique_ptr<MrDoc::Element> element;
  size_t elementNum;
  size_t pageNum;
  bool update;
  bool updateSuccessive;
  bool atEnd;
};

class RemoveElementCommand : public QUndoCommand
{
public:
  RemoveElementCommand(Widget *newWidget, size_t newPageNum, size_t newElementNum, bool newUpdate = true, QUndoCommand *parent = nullptr);
  void undo() Q_DECL_OVERRIDE;
  void redo() Q_DECL_OVERRIDE;

private:
  Widget *widget;
  std::unique_ptr<MrDoc::Element> element;
  size_t elementNum;
  size_t pageNum;
  bool update;
};

class CreateSelectionCommand : public QUndoCommand
{
public:
  CreateSelectionCommand(Widget *widget, size_t pageNum, const MrDoc::Selection& selection, QUndoCommand *parent = nullptr);
  void undo() Q_DECL_OVERRIDE;
  void redo() Q_DECL_OVERRIDE;

private:
  Widget *m_widget;
  QPolygonF m_selectionPolygon;
  std::vector<QPair<std::unique_ptr<MrDoc::Element>, size_t>> m_elementsAndPositions;
  MrDoc::Selection m_selection;
  size_t m_pageNum;
};

class ReleaseSelectionCommand : public QUndoCommand
{
public:
  ReleaseSelectionCommand(Widget *newWidget, size_t newPageNum, QUndoCommand *parent = nullptr);
  void undo() Q_DECL_OVERRIDE;
  void redo() Q_DECL_OVERRIDE;

private:
  Widget *widget;
  MrDoc::Selection selection;
  size_t pageNum;
};

class TransformSelectionCommand : public QUndoCommand
{
public:
  TransformSelectionCommand(Widget *newWidget, size_t newPageNum, QTransform newTransform, QUndoCommand *parent = nullptr);
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
  size_t pageNum;
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
  AddPageCommand(Widget *newWidget, size_t newPageNum, QUndoCommand *parent = nullptr);
  void undo() Q_DECL_OVERRIDE;
  void redo() Q_DECL_OVERRIDE;

private:
  Widget *widget;
  MrDoc::Page page;
  size_t pageNum;
};

class RemovePageCommand : public QUndoCommand
{
public:
  RemovePageCommand(Widget *newWidget, size_t newPageNum, QUndoCommand *parent = nullptr);
  void undo() Q_DECL_OVERRIDE;
  void redo() Q_DECL_OVERRIDE;

private:
  Widget *widget;
  MrDoc::Page page;
  size_t pageNum;
};

class PasteCommand : public QUndoCommand
{
public:
  PasteCommand(Widget *newWidget, const MrDoc::Selection& newSelection, QUndoCommand *parent = nullptr);
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
  ChangePageSettingsCommand(Widget *newWidget, size_t newPageNum, QSizeF newSize, QColor newBackgroundColor, QUndoCommand *parent = nullptr);
  void undo() Q_DECL_OVERRIDE;
  void redo() Q_DECL_OVERRIDE;

private:
  Widget *widget;
  size_t pageNum;
  QSizeF prevSize;
  QSizeF size;
  QColor prevBackgroundColor;
  QColor backgroundColor;
};

#endif
