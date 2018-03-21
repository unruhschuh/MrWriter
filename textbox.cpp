#include"textbox.h"

TextBox::TextBox(QWidget* parent)
    : QTextEdit(parent) {
}

/*TextBox::TextBox(MrDoc::Page &page, int textIndex, QWidget *parent)
    : QTextEdit(parent),
      m_page {page},
      m_textIndex {textIndex}{
}

TextBox::TextBox(MrDoc::Page &page, int textIndex, int _x, int _y, QWidget *parent)
    : QTextEdit(parent),
      m_page {page},
      m_textIndex {textIndex},
      x {_x},
      y {_y} {
}*/

void TextBox::setTextX(int _x){
    x = _x;
}

void TextBox::setTextY(int _y){
    y = _y;
}

void TextBox::setPage(MrDoc::Page* page){
    m_page = page;
}

void TextBox::setPageNum(int pageNum){
    m_pageNum = pageNum;
}

void TextBox::setPrevText(const QString &text){
    m_prevText = text;
}

void TextBox::setTextIndex(int index){
    m_textIndex = index;
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

MrDoc::Page* TextBox::getPage(){
    return m_page;
}

int TextBox::getTextIndex() const {
    return m_textIndex;
}

const QString& TextBox::getPrevText() const{
    return m_prevText;
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

int TextBox::getTextX() const {
    return x;
}

int TextBox::getTextY() const {
    return y;
}

void TextBox::applyText(bool onlyHide){
    if(!onlyHide){
        if(m_textIndex == -1){ //it's a new text
            if(!toPlainText().isEmpty()){
                m_page->appendText(QRectF(x, y, 0, 0), QFont("Sans", 12), m_color, toPlainText());
            }
        }
        else{
            m_page->setText(m_textIndex, m_font, m_color, toPlainText());
        }
    }
    hide();
    setText(QString(""));
    emit updatePage(m_pageNum);
}
