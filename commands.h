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
  Widget::view m_view;
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
  ChangePenWidthOfSelectionCommand(Widget *newWidget, qreal penWidth, QUndoCommand *parent = 0);
  void undo() Q_DECL_OVERRIDE;
  void redo() Q_DECL_OVERRIDE;

private:
  Widget *widget;
  MrDoc::Selection selection;
  qreal m_penWidth;
};


class CreateMarkdownSelection : public QUndoCommand{
public:
    CreateMarkdownSelection(Widget* widget, int pageNum, int markdownIndex, MrDoc::MarkdownSelection selection, QUndoCommand* parent = nullptr);
    void undo() override;
    void redo() override;

private:
    Widget* m_widget;
    int m_pageNum;
    int m_markdownIndex;
    MrDoc::MarkdownSelection m_selection;
};

class ReleaseMarkdownSelectionCommand : public QUndoCommand{
public:
    ReleaseMarkdownSelectionCommand(Widget* widget, int pageNum, QUndoCommand *parent = nullptr);
    void undo() override;
    void redo() override;

private:
    Widget* m_widget;
    int m_pageNum;
    MrDoc::MarkdownSelection m_selection;
    int m_markdownIndex;
};

class MoveMarkdownCommand : public QUndoCommand {
public:
    MoveMarkdownCommand(Widget* widget, int oldPagenum, int newPageNum, QPointF oldPos, QPointF newPos, QPointF delta, QUndoCommand* parent = nullptr);
    void undo() override;
    void redo() override;
    int id() const override {
        return 1;
    }

    bool mergeWith(const QUndoCommand *other) override;

private:
    Widget* m_widget;
    MrDoc::MarkdownSelection m_selection;
    int m_oldPageNum;
    int m_newPageNum;
    QPointF m_oldPos;
    QPointF m_newPos;
    QPointF m_delta;
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
  ChangePageSettingsCommand(Widget *newWidget, int newPageNum, QSizeF newSize, QColor newBackgroundColor, MrDoc::Page::backgroundType newBackgroundType, QUndoCommand *parent = 0);
  void undo() Q_DECL_OVERRIDE;
  void redo() Q_DECL_OVERRIDE;

private:
  Widget *widget;
  int pageNum;
  QSizeF prevSize;
  QSizeF size;
  QColor prevBackgroundColor;
  QColor backgroundColor;
  MrDoc::Page::backgroundType backgroundType;
  MrDoc::Page::backgroundType prevBackgroundType;
};

class ChangeTextCommand : public QUndoCommand{
public:
    ChangeTextCommand(Widget *widget, int pageNum, MrDoc::Page* page, int textIndex, const QColor& prevColor,
                      const QColor& color, const QFont& prevFont, const QFont& font, const QString& prevText, const QString& text, QUndoCommand *parent = nullptr);
    void undo() override;
    void redo() override;
private:
    Widget* m_widget;
    int m_pageNum;
    MrDoc::Page* m_page;
    int m_textIndex;
    QString m_prevText;
    QString m_text;
    QColor m_prevColor;
    QColor m_color;
    QFont m_prevFont;
    QFont m_font;
};

/**
 * @brief The TextCommand class is for inserting text into an MrDoc::Page
 */
class TextCommand : public QUndoCommand{
public:
    TextCommand(Widget *widget, int pageNum, MrDoc::Page* page, const QRectF &rect, const QColor& color, const QFont& font, const QString& text, QUndoCommand* parent = nullptr);
    void undo() override;
    void redo() override;
private:
    Widget* m_widget;
    int m_pageNum;
    MrDoc::Page* m_page;
    int m_textIndex;
    QRectF m_rect;
    QColor m_color;
    QFont m_font;
    QString m_text;
};

class ChangeMarkdownCommand : public QUndoCommand{
public:
    ChangeMarkdownCommand(Widget* widget, int pageNum, MrDoc::Page* page, int markdownIndex, const QString& prevText, const QString& text, const QRectF &rect, QUndoCommand* parent = nullptr);
    void undo() override;
    void redo() override;
private:
    Widget* m_widget;
    int m_pageNum;
    MrDoc::Page* m_page;
    int m_markdownIndex;
    QString m_prevText;
    QString m_text;
    QRectF m_rect;
};

class MarkdownCommand : public QUndoCommand{
public:
    MarkdownCommand(Widget* widget, int pageNum, MrDoc::Page* page, const QPointF &upperLeft, const QString& text, QUndoCommand* parent = nullptr);
    void undo() override;
    void redo() override;
private:
    Widget* m_widget;
    int m_pageNum;
    MrDoc::Page* m_page;
    int m_markdowIndex;
    QPointF m_upperLeft;
    QString m_text;
};

#endif
