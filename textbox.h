#ifndef TEXTBOX_H
#define TEXTBOX_H

#include <QTextEdit>
#include "page.h"
#include "abstracttextbox.h"

/**
 * @brief The TextBox class is the widget for text insertion. It contains also the information for previous text
 * display in the text box, when changing it.
 */
class TextBox : public AbstractTextBox {
    Q_OBJECT
public:
    TextBox(QWidget* parent = nullptr);

    void setPrevColor(const QColor& color);
    void setTextColor(const QColor& color);
    void setPrevFont(const QFont& font);
    void setTextFont(const QFont& font);

    const QColor& getPrevColor() const;
    const QFont& getFont() const;
    const QFont& getPrevFont() const;

signals:
    void updatePageSimpleText(int pageNum);

public slots:
    virtual void applyText();

private:
    QColor m_prevColor;
    QColor m_color;
    QFont m_prevFont;
    QFont m_font;
};

#endif // TEXTBOX_H
