#ifndef ABSTRACTTEXTBOX_H
#define ABSTRACTTEXTBOX_H
#include <QTextEdit>
#include "page.h"

class AbstractTextBox : public QTextEdit {
    Q_OBJECT
public:
    AbstractTextBox(QWidget* parent = nullptr);

    void setBoundingRect(const QRectF& rect);
    void setPage(MrDoc::Page *page);
    void setPageNum(int pageNum);
    void setPrevText(const QString& text);
    void setTextIndex(int index);

    MrDoc::Page* getPage();
    int getPageNum();
    int getTextIndex() const;
    const QString& getPrevText() const;
    int getTextX() const;
    int getTextY() const;
    const QRectF& getBoundingRect() const;

public slots:
    /**
     * @brief applyText hides the widget (and emits signal @ref updatePage);
     */
    virtual void applyText() = 0;

protected:
    MrDoc::Page* m_page;
    int m_pageNum;
    QString m_prevText;
    int m_textIndex;
    //int x;  //x-Coordinate of the text, not of the textBox
    //int y;
    QRectF m_textRect;

};

#endif // ABSTRACTTEXTBOX_H
