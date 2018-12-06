#ifndef MARKDOWNSELECTION_H
#define MARKDOWNSELECTION_H
#include<QPainter>
#include<QRect>
#include<QString>
#include<QTextDocument>

#include<tuple>

#include "page.h"

namespace MrDoc {

class MarkdownSelection {
public:
    enum class GrabZone{
        None,
        Move,
        Top,
        Bottom,
        Left,
        Right,
        TopLeft,
        TopRight,
        BottomLeft,
        BottomRight
    };
    MarkdownSelection(std::tuple<QRectF, QString> markdown);

    void setPageNum(int pageNum);
    int pageNum() const;

    const QString& text() const;
    const QRectF& boundingRect() const;

    void moveTo(QPointF newPos, QPointF delta, int pageNum);

    void paint(QPainter& painter, qreal zoom);

    GrabZone grabZone(QPointF pagePos);

private:
    std::tuple<QRectF, QString> m_markdown;

    int m_pageNum;
};

}

#endif // MARKDOWNSELECTION_H
