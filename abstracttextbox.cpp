#include "abstracttextbox.h"

AbstractTextBox::AbstractTextBox(QWidget *parent)
    : QTextEdit(parent) {
}

void AbstractTextBox::setBoundingRect(const QRectF &rect){
    m_textRect = rect;
}

void AbstractTextBox::setPage(MrDoc::Page* page){
    m_page = page;
}

void AbstractTextBox::setPageNum(int pageNum){
    m_pageNum = pageNum;
}

int AbstractTextBox::getPageNum(){
    return m_pageNum;
}

void AbstractTextBox::setPrevText(const QString &text){
    m_prevText = text;
}

void AbstractTextBox::setTextIndex(int index){
    m_textIndex = index;
}

MrDoc::Page* AbstractTextBox::getPage(){
    return m_page;
}

int AbstractTextBox::getTextIndex() const {
    return m_textIndex;
}

const QString& AbstractTextBox::getPrevText() const{
    return m_prevText;
}

int AbstractTextBox::getTextX() const {
    return m_textRect.x();
}

int AbstractTextBox::getTextY() const {
    return m_textRect.y();
}

const QRectF& AbstractTextBox::getBoundingRect() const {
    return m_textRect;
}
