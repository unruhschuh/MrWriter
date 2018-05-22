#include "page.h"
#include "mrdoc.h"
#include <QDebug>

namespace MrDoc
{

Page::Page(/*const Page &page*/)
{
    // set up standard page (Letter, white background)
    setWidth(595.0);
    setHeight(842.0);
    //    setWidth(600.0);
    //    setHeight(800.0);
    setBackgroundColor(QColor(255, 255, 255));
    setBackgroundType(backgroundType::PLAIN);
}

qreal Page::height() const
{
  return m_height;
}

qreal Page::width() const
{
  return m_width;
}

void Page::setHeight(qreal height)
{
  if (height > 0)
  {
    m_height = height;
  }
}

void Page::setWidth(qreal width)
{
  if (width > 0)
  {
    m_width = width;
  }
}

void Page::paint(QPainter &painter, qreal zoom, QRectF region)
{
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
    if(m_backgroundType != backgroundType::PLAIN){
        QPen pen;
        pen.setColor(QColor(100,100,150));
        painter.setPen(pen);
        if(m_backgroundType == backgroundType::SQUARED){
            for(int i = 0; i < 42*m_width/595.0; ++i){
                painter.drawLine(QPointF(595.0/42*(i+1)*zoom,0), QPointF(595.0/42*(i+1)*zoom, m_height*zoom));
            }
            for(int i = 0; i < 59.4*m_height/842.0; ++i){
                painter.drawLine(QPointF(0, 842.0/59.4*(i+1)*zoom), QPointF(m_width*zoom, 842.0/59.4*(i+1)*zoom));
            }
        }
        else if(m_backgroundType == backgroundType::RULED){
            for(int i = 0; i < 29.7*m_height/842.0; ++i){
                painter.drawLine(QPointF(0, 842.0/29.7*(i+1)*zoom), QPointF(m_width*zoom, 842.0/29.7*(i+1)*zoom));
            }
        }
        pen.setColor(QColor(255,255,255));
        painter.setPen(pen);
    }
    if(m_pdfPointer != nullptr){
        //double eZoom = zoom*(exp(-zoom)+2) > 10 ? 10 : zoom*(exp(-zoom)+2);
        //auto img = m_pdfPointer->renderToImage(72.0*eZoom, 72.0*eZoom, 0,0,int(m_width*eZoom), int(m_height*eZoom));
        //QImage image = m_pdfPointer->renderToImage(72.0*eZoom, 72.0*eZoom, 0,0,int(m_width*eZoom), int(m_height*eZoom));
        //painter.drawImage(0,0, image.scaled(m_width*zoom, m_height*zoom, Qt::KeepAspectRatio, Qt::SmoothTransformation));

        QImage image = m_pdfPointer->renderToImage(72.0*zoom, 72.0*zoom, 0,0, int(m_width*zoom), int(m_height*zoom));
        painter.drawImage(0,0, image);

        /*if(region.isNull()){
            qDebug() << "region is null";
            QImage image = m_pdfPointer->renderToImage(72.0*eZoom, 72.0*eZoom, 0,0,int(m_width*eZoom), int(m_height*eZoom));
            painter.drawImage(0,0, image.scaled(m_width*zoom, m_height*zoom, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        }
        else{
            qDebug() << "region";
            //QImage image = m_pdfPointer->renderToImage(72.0*eZoom, 72.0*eZoom, region.x(), region.y(), region.width(), region.height());
            QImage image = m_pdfPointer->renderToImage(72.0*eZoom, 72.0*eZoom, 0,0,int(m_width*eZoom), int(m_height*eZoom));
            QRectF source(0.0, 0.0, 200, 800);
            painter.drawImage(region, image.scaled(region.width()*zoom, region.height()*zoom, Qt::KeepAspectRatio, Qt::SmoothTransformation), source);
        }*/
    }
    /*if(!m_pdf.isNull()){
        painter.drawImage(0,0, m_pdf.scaled(m_width*zoom, m_height*zoom, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }*/
    if(rectIsPoint){
        for(int i = 0; i < m_texts.length(); ++i){
            QFont font = std::get<1>(m_texts[i]);
            font.setPointSize(font.pointSize()*zoom);
            painter.setFont(font);
            painter.setPen(std::get<2>(m_texts[i]));
            //qDebug() << "rectIsPoint: " << std::get<3>(m_texts[i]);
            painter.drawText(std::get<0>(m_texts[i]).x()*zoom, std::get<0>(m_texts[i]).y()*zoom, m_width, m_height, Qt::TextWordWrap, std::get<3>(m_texts[i]));
            QRectF rect = painter.boundingRect(std::get<0>(m_texts[i]).x(), std::get<0>(m_texts[i]).y(), m_width, m_height, Qt::TextWordWrap, std::get<3>(m_texts[i]));
            m_texts[i] = std::make_tuple(rect, std::get<1>(m_texts[i]), std::get<2>(m_texts[i]), std::get<3>(m_texts[i]));
        }
        rectIsPoint = false;
    }
    else{
        for(auto t : m_texts){
            QFont font = std::get<1>(t);
            font.setPointSize(font.pointSize()*zoom);
            painter.setFont(font);
            painter.setPen(std::get<2>(t));
            //qDebug() << "Paint: " << std::get<3>(t);
            painter.drawText(std::get<0>(t).x()*zoom, std::get<0>(t).y()*zoom, m_width, m_height, Qt::TextWordWrap, std::get<3>(t));
        }
    }
    for (Stroke &stroke : m_strokes)
    {
        if (region.isNull() || stroke.boundingRect().intersects(region))
        {
            //stroke.paint(painter, QRect(stroke.boundingRect().x()*zoom, stroke.boundingRect().y()*zoom, stroke.boundingRect().width()*zoom, stroke.boundingRect().height()*zoom), zoom);
            stroke.paint(painter, zoom);
            //stroke.paint(painter, QRect(0,0, m_width*zoom, m_height*zoom), zoom);
        }
    }
    painter.setCompositionMode(QPainter::CompositionMode_SourceOver);

    QColor halfYellow(255,255,0,128);
    for (QRectF& rect : searchResultRects){
        painter.fillRect(rect.x()*zoom, rect.y()*zoom, rect.width()*zoom, rect.height()*zoom, halfYellow);
    }

    painter.scale(zoom, zoom);
    for(auto t : m_markdownDocs){
        qDebug() << std::get<0>(t);
        QTextDocument td;
        td.setHtml(compileMarkdown(std::get<1>(t)));
        painter.translate(std::get<0>(t).x(), std::get<0>(t).y());
        td.setPageSize(QSizeF(std::get<0>(t).width(), std::get<0>(t).height()));
        td.drawContents(&painter);
        painter.translate(-std::get<0>(t).x(), -std::get<0>(t).y());
    }
}

void Page::paintForPdfExport(QPainter &painter, qreal zoom){

    if(rectIsPoint){
        for(int i = 0; i < m_texts.length(); ++i){
            QFont font = std::get<1>(m_texts[i]);
            font.setPointSize(font.pointSize()*zoom);
            painter.setFont(font);
            painter.setPen(std::get<2>(m_texts[i]));
            painter.drawText(std::get<0>(m_texts[i]).x()*zoom, std::get<0>(m_texts[i]).y()*zoom, m_width, m_height, Qt::TextWordWrap, std::get<3>(m_texts[i]));
            QRectF rect = painter.boundingRect(std::get<0>(m_texts[i]).x(), std::get<0>(m_texts[i]).y(), m_width, m_height, Qt::TextWordWrap, std::get<3>(m_texts[i]));
            m_texts[i] = std::make_tuple(rect, std::get<1>(m_texts[i]), std::get<2>(m_texts[i]), std::get<3>(m_texts[i]));
        }
        //rectIsPoint = false;
    }
    else{
        for(auto t : m_texts){
            QFont font = std::get<1>(t);
            font.setPointSize(font.pointSize()*zoom);
            painter.setFont(font);
            painter.setPen(std::get<2>(t));
            painter.drawText(std::get<0>(t).x()*zoom, std::get<0>(t).y()*zoom, m_width, m_height, Qt::TextWordWrap, std::get<3>(t));
        }
    }


    for (Stroke &stroke : m_strokes)
    {
        stroke.paint(painter, zoom);
    }

    painter.scale(zoom, zoom);
    for(auto t : m_markdownDocs){
        qDebug() << std::get<0>(t);
        QTextDocument td;
        td.setHtml(compileMarkdown(std::get<1>(t)));
        painter.translate(std::get<0>(t).x(), std::get<0>(t).y());
        td.setPageSize(QSizeF(std::get<0>(t).width(), std::get<0>(t).height()));
        td.drawContents(&painter);
        painter.translate(-std::get<0>(t).x(), -std::get<0>(t).y());
    }
}

void Page::setBackgroundColor(QColor backgroundColor)
{
  m_backgroundColor = backgroundColor;
}

QColor Page::backgroundColor() const
{
  return m_backgroundColor;
}

void Page::setBackgroundType(backgroundType type){
    m_backgroundType = type;
}

MrDoc::Page::backgroundType Page::getBackgroundType() const {
    return m_backgroundType;
}

const QRectF &Page::dirtyRect() const
{
  return m_dirtyRect;
}

void Page::clearDirtyRect()
{
  m_dirtyRect = QRectF(0.0, 0.0, 0.0, 0.0);
}

bool Page::changePenWidth(int strokeNum, qreal penWidth)
{
  if (strokeNum < 0 || strokeNum >= m_strokes.size() || m_strokes.isEmpty())
  {
    return false;
  }
  else
  {
    m_strokes[strokeNum].penWidth = penWidth;
    m_dirtyRect = m_dirtyRect.united(m_strokes[strokeNum].boundingRect());
    return true;
  }
}

bool Page::changeStrokeColor(int strokeNum, QColor color)
{
  if (strokeNum < 0 || strokeNum >= m_strokes.size() || m_strokes.isEmpty())
  {
    return false;
  }
  else
  {
    m_strokes[strokeNum].color = color;
    m_dirtyRect = m_dirtyRect.united(m_strokes[strokeNum].boundingRect());
    return true;
  }
}

bool Page::changeStrokePattern(int strokeNum, QVector<qreal> pattern)
{
  if (strokeNum < 0 || strokeNum >= m_strokes.size() || m_strokes.isEmpty())
  {
    return false;
  }
  else
  {
    m_strokes[strokeNum].pattern = pattern;
    m_dirtyRect = m_dirtyRect.united(m_strokes[strokeNum].boundingRect());
    return true;
  }
}

int Page::textIndexFromMouseClick(int x, int y){
    for(int i = 0; i < m_texts.length(); ++i){
        if(std::get<0>(m_texts[i]).contains(x, y)){
            return i;
        }
    }
    return -1;
}

int Page::appendText(const QRectF &rect, const QFont &font, const QColor &color, const QString &text){
    m_texts.append(std::make_tuple(rect, font, color, text));
    rectIsPoint = true;
    return m_texts.size()-1;
}

const QString& Page::textByIndex(int i){
    return std::get<3>(m_texts[i]);
}

void Page::setText(int index, const QFont& font, const QColor& color, const QString& text){
    if(text.isEmpty()){
        m_texts.remove(index);
    }
    else{
        //QRectF rect = std::get<0>(m_texts[index]);
        auto t = std::make_tuple(std::get<0>(m_texts[index]), font, color, text);
        m_texts[index] = t;
        rectIsPoint = true;
    }
}

const QRectF& Page::textRectByIndex(int i){
    return std::get<0>(m_texts[i]);
}

const QColor& Page::textColorByIndex(int i){
    return std::get<2>(m_texts[i]);
}

const QFont& Page::textFontByIndex(int i){
    return std::get<1>(m_texts[i]);
}

int Page::markdownIndexFromMouseClick(int x, int y){
    for(int i = 0; i < m_markdownDocs.size(); ++i){
        if(std::get<0>(m_markdownDocs[i]).contains(x, y)){
            return i;
        }
    }
    return -1;
}

int Page::appendMarkdown(const QRectF &rect, const QString &text){
    QRectF boundingRect;

    QTextDocument td;

    td.setHtml(compileMarkdown(text));

    td.setPageSize(adjustMarkdownSize(rect.x(), rect.y(), td.size()));

    if(rect.width() == 0 && rect.height() == 0){
        boundingRect = QRectF(rect.x(), rect.y(), td.size().width(), td.size().height());
    }
    else{
        boundingRect = rect;
    }

    m_markdownDocs.append(std::make_tuple(boundingRect, text));
    return m_markdownDocs.size()-1;
}

void Page::setMarkdown(int index, const QString &text, const QRectF& rect){
    if(text.isEmpty()){
        m_markdownDocs.remove(index);
    }
    else{
        QTextDocument td;

        td.setHtml(compileMarkdown(text));

        td.setPageSize(QSizeF(rect.width(), rect.height()));
        //td.setPageSize(adjustMarkdownSize(std::get<0>(m_markdownDocs[index]).x(), std::get<0>(m_markdownDocs[index]).y(), td.size()));

        //auto t = std::make_tuple(QRectF(QPointF(std::get<0>(m_markdownDocs[index]).x(), std::get<0>(m_markdownDocs[index]).y()), td.size()), text);
        //m_markdownDocs[index] = t;
        m_markdownDocs.insert(index, std::make_tuple(rect, text));
    }
}

QString Page::markdownByIndex(int i){
    return std::get<1>(m_markdownDocs[i]);
}

QRectF Page::markdownRectByIndex(int i){
    return std::get<0>(m_markdownDocs[i]);
}

const QVector<Stroke> &Page::strokes()
{
  return m_strokes;
}

const QVector<std::tuple<QRectF, QFont, QColor, QString> > &Page::texts(){
    return m_texts;
}

const QVector<std::tuple<QRectF, QString>>& Page::markdowns(){
    return m_markdownDocs;
}

QVector<QPair<Stroke, int>> Page::getStrokes(QPolygonF selectionPolygon)
{
  QVector<QPair<Stroke, int>> strokesAndPositions;

  for (int i = m_strokes.size() - 1; i >= 0; --i)
  {
    const MrDoc::Stroke &stroke = m_strokes.at(i);
    bool containsStroke = true;
    for (int j = 0; j < stroke.points.size(); ++j)
    {
      if (!selectionPolygon.containsPoint(stroke.points.at(j), Qt::OddEvenFill))
      {
        containsStroke = false;
      }
    }
    if (containsStroke)
    {
      // add selected strokes and positions to return vector
      strokesAndPositions.append(QPair<Stroke, int>(stroke, i));
    }
  }

  return strokesAndPositions;
}

QVector<QPair<Stroke, int>> Page::removeStrokes(QPolygonF selectionPolygon)
{
  auto removedStrokesAndPositions = getStrokes(selectionPolygon);

  for (auto sAndP : removedStrokesAndPositions)
  {
    removeStrokeAt(sAndP.second);
  }

  return removedStrokesAndPositions;
}

void Page::removeStrokeAt(int i)
{
  m_dirtyRect = m_dirtyRect.united(m_strokes[i].boundingRect());
  m_strokes.removeAt(i);
}

void Page::removeLastStroke()
{
  removeStrokeAt(m_strokes.size() - 1);
}

void Page::insertStrokes(const QVector<QPair<Stroke, int>> &strokesAndPositions)
{
  for (int i = strokesAndPositions.size() - 1; i >= 0; --i)
  {
    insertStroke(strokesAndPositions[i].second, strokesAndPositions[i].first);
  }
}

void Page::insertStroke(int position, const Stroke &stroke)
{
  m_dirtyRect = m_dirtyRect.united(stroke.boundingRect());
  m_strokes.insert(position, stroke);
}

void Page::appendStroke(const Stroke &stroke)
{
  m_dirtyRect = m_dirtyRect.united(stroke.boundingRect());
  m_strokes.append(stroke);
}

void Page::prependStroke(const Stroke &stroke)
{
  m_dirtyRect = m_dirtyRect.united(stroke.boundingRect());
  m_strokes.prepend(stroke);
}

void Page::appendStrokes(const QVector<Stroke> &strokes)
{
  for (auto &stroke : strokes)
  {
    appendStroke(stroke);
  }
}

void Page::setPdf(Poppler::Page* page, int pageNum, bool adjustSize){
    /*Poppler::Document* doc = Poppler::Document::load(path);
    if (!doc || doc->isLocked()){
        qDebug() << "Couldn't load PDF";
    }
    else{
        Poppler::Page* page = doc->page(pageNum);
        if(page == 0){
            qDebug() << "Couldn't load PDF page";
        }
        else{
            //m_pdf = page->renderToImage(72.0*10, 72.0*10, 0,0,int(m_width*10), int(m_height*10));
            m_pdfPointer.reset(page); //= std::make_shared<Poppler::Page>(page);
            pageno = pageNum;
            //delete page;
        }
    }*/
    //delete doc;

    if(adjustSize){
        m_width = page->pageSizeF().width();
        m_height = page->pageSizeF().height();
    }
    m_pdfPointer.reset(page);
    pageno = pageNum;
}

bool Page::searchPdfNext(const QString &text){
    if(isPdf()){
        searchResultRects = m_pdfPointer->search(text, Poppler::Page::IgnoreCase);
        return !searchResultRects.isEmpty();
    }
    return false;
}

bool Page::searchPdfPrev(const QString &text){
    if(isPdf()){
        searchResultRects = m_pdfPointer->search(text, Poppler::Page::IgnoreCase);
        return !searchResultRects.isEmpty();
    }
    return false;
}

void Page::clearPdfSearch(){
    searchResultRects.clear();
}

Poppler::LinkGoto* Page::linkFromMouseClick(qreal x, qreal y){
    if(isPdf()){
        QList<Poppler::Link*> links = m_pdfPointer->links();
        for(auto link : links){
            if(link->linkArea().contains(x/m_width,y/m_height) && link->linkType() == Poppler::Link::LinkType::Goto){
                return static_cast<Poppler::LinkGoto*>(link);
            }
        }
    }
    return nullptr;
}

char* Page::compileMarkdown(const QString &text){
    MMIOT* doc;
    char* output;
    QByteArray data = text.toUtf8();
    doc = mkd_string(data, data.length(), 0);
    mkd_compile(doc, 0);
    mkd_document(doc, &output);
    return output;
}

QSizeF Page::adjustMarkdownSize(int x, int y, QSizeF oldSize){
    QSizeF returnSize = oldSize;
    bool sizeChanged = false;
    if((oldSize.width()+x) > m_width){
        QSizeF newSize(m_width-x, std::max(m_height-y, oldSize.width()/(m_width-x)*oldSize.height())); //same area
        returnSize = newSize;
        sizeChanged = true;
    }
    if(!sizeChanged){
        if((returnSize.height()+y) > m_height){
            QSizeF newSize(std::max(m_width-x, returnSize.height()/(m_height-y)*returnSize.width()), m_height-y);
            returnSize = newSize;
        }
    }
    return returnSize;
}

}
