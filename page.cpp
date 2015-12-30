#include "page.h"
#include "mrdoc.h"
#include <QDebug>

namespace MrDoc {

    Page::Page()
    {
        // set up standard page (Letter, white background)
        setWidth(595.0);
        setHeight(842.0);
    //    setWidth(600.0);
    //    setHeight(800.0);
        setBackgroundColor(QColor(255,255,255));
    }


    qreal Page::getHeight()
    {
        return height;
    }

    qreal Page::getWidth()
    {
        return width;
    }

    void Page::setHeight(qreal newHeight)
    {
        if (newHeight > 0)
        {
            height = newHeight;
        }
    }

    void Page::setWidth(qreal newWidth)
    {
        if (newWidth > 0)
        {
            width = newWidth;
        }
    }

    void Page::paint(QPainter &painter, qreal zoom, QRectF region)
    {
        for (int i = 0; i < m_strokes.length(); ++i)
        {
            Stroke &stroke = m_strokes[i];
            if (region.isNull() || stroke.boundingRect().intersects(region))
            {
                stroke.paint(painter, zoom);
            }
        }
    }

    void Page::setBackgroundColor(QColor newBackgroundColor)
    {
        backgroundColor = newBackgroundColor;
    }

    QColor Page::getBackgroundColor()
    {
        return backgroundColor;
    }

    const QRectF & Page::getDirtyRect()
    {
        return dirtyRect;
    }

    void Page::clearDirtyRect()
    {
        dirtyRect = QRectF();
    }

    QVector<QPair<Stroke, int>> Page::getStrokes(QPolygonF &selectionPolygon)
    {
        QVector<QPair<Stroke, int>> removedStrokesAndPositions;

        for (int i = m_strokes.size()-1; i >= 0; --i)
        {
            const MrDoc::Stroke &stroke = m_strokes.at(i);
            bool containsStroke = true;
            for (int j = 0; j < stroke.points.size(); ++j)
            {
                if (!selectionPolygon.containsPoint(stroke.points.at(j), Qt::OddEvenFill)) {
                    containsStroke = false;
                }
            }
            if (containsStroke)
            {
                // add selected strokes and positions to return vector
                removedStrokesAndPositions.append(QPair<Stroke, int>(stroke, i));
            }
        }

        return removedStrokesAndPositions;
    }

    QVector<QPair<Stroke, int>> Page::removeStrokes(QPolygonF &selectionPolygon)
    {
        auto removedStrokesAndPositions = getStrokes(selectionPolygon);

        for (auto sAndP : removedStrokesAndPositions)
        {
            m_strokes.removeAt(sAndP.second);
        }

        return removedStrokesAndPositions;
    }

    void Page::removeStrokeAt(int i)
    {
        dirtyRect = dirtyRect.united(m_strokes[i].boundingRect());
        m_strokes.removeAt(i);
    }

    void Page::insertStrokes(QVector<QPair<Stroke, int>> &strokesAndPositions)
    {
        for (int i = strokesAndPositions.size()-1; i >= 0; --i)
        {
            insertStroke(strokesAndPositions[i].second,
                         strokesAndPositions[i].first);
        }
    }

    void Page::insertStroke(int position, Stroke &stroke)
    {
        dirtyRect = dirtyRect.united(stroke.boundingRect());
        m_strokes.insert(position, stroke);
    }

    void Page::appendStroke(Stroke &stroke)
    {
        dirtyRect = dirtyRect.united(stroke.boundingRect());
        m_strokes.append(stroke);
    }

    void Page::appendStrokes(QVector<Stroke> &strokes)
    {
        for (Stroke &stroke : strokes)
        {
            m_strokes.append(stroke);
        }
    }

    void Page::prependStroke(Stroke &stroke)
    {
        dirtyRect = dirtyRect.united(stroke.boundingRect());
        m_strokes.append(stroke);
    }
}
