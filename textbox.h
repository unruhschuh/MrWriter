#ifndef TEXTBOX_H
#define TEXTBOX_H

#include <QTextEdit>
#include "page.h"

class TextBox : public QTextEdit {
    Q_OBJECT
public:
    TextBox(QWidget* parent = nullptr);
    /*TextBox(MrDoc::Page& page, int textIndex, QWidget* parent = nullptr);
    TextBox(MrDoc::Page& page, int textIndex, int _x, int _y, QWidget* parent = nullptr);*/

    void setTextX(int _x);
    void setTextY(int _y);
    void setPage(MrDoc::Page *page);
    void setPageNum(int pageNum);
    void setPrevText(const QString& text);
    void setTextIndex(int index);
    void setPrevColor(const QColor& color);
    void setTextColor(const QColor& color);
    void setPrevFont(const QFont& font);
    void setTextFont(const QFont& font);

    MrDoc::Page* getPage();
    int getTextIndex() const;
    const QString& getPrevText() const;
    const QColor& getPrevColor() const;
    const QFont& getFont() const;
    const QFont& getPrevFont() const;
    int getTextX() const;
    int getTextY() const;

public slots:
    void applyText(bool onlyHide);

signals:
    void updatePage(int pageNum);

private:
    MrDoc::Page* m_page;
    int m_pageNum;
    QString m_prevText;
    int m_textIndex;
    QColor m_prevColor;
    QColor m_color;
    QFont m_prevFont;
    QFont m_font;
    int x;  //x-Coordinate of the text, not of the textBox
    int y;
};

#endif // TEXTBOX_H
