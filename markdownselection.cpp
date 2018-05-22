#include "markdownselection.h"
#include <QDebug>

namespace MrDoc {

MarkdownSelection::MarkdownSelection(std::tuple<QRectF, QString> markdown)
    : m_markdown{markdown} {
}

void MarkdownSelection::setPageNum(int pageNum){
    m_pageNum = pageNum;
}

int MarkdownSelection::pageNum() const {
    return m_pageNum;
}

const QString& MarkdownSelection::text() const {
    return std::get<1>(m_markdown);
}

const QRectF& MarkdownSelection::boundingRect() const {
    return std::get<0>(m_markdown);
}

void MarkdownSelection::moveTo(QPointF newPos, QPointF delta, int pageNum){
    m_pageNum = pageNum;
    m_markdown = std::make_tuple(QRectF(newPos.x()+delta.x(), newPos.y()+delta.y(), std::get<0>(m_markdown).width(), std::get<0>(m_markdown).height()), std::get<1>(m_markdown));
}

MrDoc::MarkdownSelection::GrabZone MarkdownSelection::grabZone(QPointF pagePos){
    qreal dist = 5.0;
    QRectF boundingRect = std::get<0>(m_markdown);
    if( (pagePos-boundingRect.topLeft()).manhattanLength() < dist){
        return GrabZone::TopLeft;
    }
    else if( (pagePos-boundingRect.topRight()).manhattanLength() < dist){
        return GrabZone::TopRight;
    }
    else if( (pagePos-boundingRect.bottomLeft()).manhattanLength() < dist){
        return GrabZone::BottomLeft;
    }
    else if( (pagePos-boundingRect.bottomRight()).manhattanLength() < dist){
        return GrabZone::BottomRight;
    }
    else if( ((pagePos-boundingRect.topLeft())+QPointF(boundingRect.width()/2.0,0)).manhattanLength() < dist ){
        return GrabZone::Top;
    }
    else if( ((pagePos-boundingRect.topLeft())+QPointF(0,boundingRect.height()/2.0)).manhattanLength() < dist){
        return GrabZone::Left;
    }
    else if( ((pagePos-boundingRect.bottomRight())+QPointF(0,-boundingRect.height()/2.0)).manhattanLength() < dist){
        return GrabZone::Right;
    }
    else if( ((pagePos-boundingRect.bottomLeft())+QPointF(-boundingRect.width()/2.0,0)).manhattanLength() < dist){
        return GrabZone::Bottom;
    }
    else if(boundingRect.contains(pagePos)){
        return GrabZone::Move;
    }
    else{
        return GrabZone::None;
    }
}

void MarkdownSelection::paint(QPainter &painter, qreal zoom){

    painter.setRenderHint(QPainter::Antialiasing, true);

    QTextDocument td;
    td.setHtml(Page::compileMarkdown(std::get<1>(m_markdown)));
    painter.scale(zoom,zoom);
    painter.translate(std::get<0>(m_markdown).x(), std::get<0>(m_markdown).y());
    td.setPageSize(QSizeF(std::get<0>(m_markdown).width(), std::get<0>(m_markdown).height()));
    td.drawContents(&painter);
    painter.translate(-std::get<0>(m_markdown).x(), -std::get<0>(m_markdown).y());

    QPen pen;
    pen.setStyle(Qt::SolidLine);
    pen.setWidthF(1);
    pen.setCapStyle(Qt::RoundCap);
    pen.setColor(QColor(50,50,50,255));
    painter.setPen(pen);

    QRectF boundingRect = std::get<0>(m_markdown);
    painter.drawRect(boundingRect);

    qreal radius = 2.5;
    painter.drawEllipse(boundingRect.topLeft(), radius, radius);
    painter.drawEllipse(boundingRect.topRight(), radius, radius);
    painter.drawEllipse(boundingRect.bottomLeft(), radius, radius);
    painter.drawEllipse(boundingRect.bottomRight(), radius, radius);
    painter.drawEllipse(boundingRect.topLeft()+QPointF(boundingRect.width()/2.0,0), radius,radius);
    painter.drawEllipse(boundingRect.topLeft()+QPointF(0,boundingRect.height()/2), radius,radius);
    painter.drawEllipse(boundingRect.bottomRight()+QPointF(0,-boundingRect.height()/2), radius,radius);
    painter.drawEllipse(boundingRect.bottomRight()+QPointF(-boundingRect.width()/2,0), radius,radius);

    painter.scale(1/zoom,1/zoom);
}

}
