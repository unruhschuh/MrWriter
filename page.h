#ifndef PAGE_H
#define PAGE_H

#include "stroke.h"

class Page
{
public:
    Page();

    qreal getWidth();
    qreal getHeight();

    void setWidth(qreal newWidth);
    void setHeight(qreal newHeight);

    void setBackgroundColor(QColor newBackgroundColor);
    QColor getBackgroundColor(void);


//    virtual void paint(QPainter &painter, qreal zoom);
    virtual void paint(QPainter &painter, qreal zoom, QRectF region = QRect(0,0,0,0), bool pdf=false);

//    QVector<Stroke> strokes;
    QList<Stroke> strokes;
    QColor backgroundColor;

private:
    float width;  // post script units
    float height; // post script units

};

#endif // PAGE_H
