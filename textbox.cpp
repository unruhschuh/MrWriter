#include"textbox.h"

TextBox::TextBox(QWidget* parent)
    : AbstractTextBox(parent) {
}

void TextBox::setPrevColor(const QColor &color){
    m_prevColor = color;
}

void TextBox::setTextColor(const QColor &color){
    m_color = color;
}

void TextBox::setPrevFont(const QFont &font){
    m_prevFont = font;
}

void TextBox::setTextFont(const QFont &font){
    m_font = font;
}

const QColor& TextBox::getPrevColor() const {
    return m_prevColor;
}

const QFont& TextBox::getFont() const {
    return m_font;
}

const QFont& TextBox::getPrevFont() const {
    return m_prevFont;
}

void TextBox::applyText(){
    hide();
    setText(QString(""));
    emit updatePageSimpleText(m_pageNum);
}
