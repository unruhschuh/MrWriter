#include "document.h"

#include "qcompressor.h"
#include "version.h"

#include <QPdfWriter>
#include <QPrinter>
#include <QPageSize>
#include <iostream>
#include <QXmlStreamReader>
#include <QFile>
#include <QFileInfo>
#include <QErrorMessage>
//#include <QSvgGenerator>
#include <QDebug>

#include <zlib.h>

// static members

namespace MrDoc
{

Document::Document()
{
  for (int i = 0; i < 1; ++i)
  {
    Page nextPage;
    pages.append(nextPage);
  }
  setDocumentChanged(false);
}

/*Document::Document(const Document &doc)
{
  for (int i = 0; i < doc.pages.size(); ++i)
  {
    pages.append(doc.pages.at(i));
  }
  m_documentChanged = doc.m_documentChanged;
  m_docName = doc.m_docName;
  m_pdfPath = doc.m_pdfPath;
  m_pdfDoc.reset(Poppler::Document::load(m_pdfPath));
}

Document& Document::operator=(const Document& doc){
    pages.clear();
    for(int i = 0; i < doc.pages.size(); ++i){
        pages.append(doc.pages.at(i));
    }
    m_documentChanged = doc.m_documentChanged;
    m_docName = doc.m_docName;
    m_pdfPath = doc.m_pdfPath;
    m_pdfDoc.reset(Poppler::Document::load(m_pdfPath));
    return *this;
}*/

void Document::paintPage(int pageNum, QPainter &painter, qreal zoom)
{
  pages[pageNum].paint(painter, zoom);
}

/* will become exportSVG
void Document::exportPDF(QString fileName)
{
    QPainter painter;
    QSvgGenerator svgGenerator;
    for (int pageNum = 0; pageNum < pages.size(); ++pageNum)
    {
        svgGenerator.setFileName(fileName);
//.prepend(QString::number(pageNum)));
        svgGenerator.setSize(QSize(pages[pageNum].width(),
pages[pageNum].height()));
//        svgGenerator.setViewBox(QRect(0, 0, pages[pageNum+1].width(),
pages[pageNum+1].height()));
        svgGenerator.setResolution(72);
        if (!painter.begin(&svgGenerator))
        {
            qDebug() << "error";
        }

        pages[pageNum].paint(painter, 1.0);
        painter.end();
    }
}
*/

void Document::exportPDF(QString fileName)
{
    if(m_pdfPath.isEmpty()){
        exportPDFAsImage(fileName);
        return;
    }

    QString overlay = "overlay.pdf";
    QString overlayFileName = QUrl(fileName).adjusted(QUrl::RemoveFilename).toString();
    overlayFileName += overlay;

    QPrinter pdfWriter(QPrinter::HighResolution);
    pdfWriter.setOutputFormat(QPrinter::PdfFormat);
    pdfWriter.setOutputFileName(overlayFileName);

    pdfWriter.setPageSize(QPageSize(QSizeF(pages[0].width(), pages[0].height()), QPageSize::Point));
    pdfWriter.setPageMargins(QMarginsF(0, 0, 0, 0));

//    qreal zoomW = ((qreal)pdfWriter.pageRect().width()) / ((qreal)pdfWriter.paperRect().width());
//    qreal zoomH = ((qreal)pdfWriter.pageRect().height()) / ((qreal)pdfWriter.paperRect().height());
//    qreal zoom = zoomW;
//    if (zoomH < zoomW)
//        zoom = zoomH;

    pdfWriter.setResolution(72);
    pdfWriter.pageLayout().setUnits(QPageLayout::Point);
    QPainter painter;
    painter.begin(&pdfWriter);
    painter.setRenderHint(QPainter::Antialiasing, true);

    struct pdfOrNotInterval{
        pageType type;
        int begin;
        int beginInPdf;
        int endInPdf;
    };

    QVector<pdfOrNotInterval> pdfIntervals;
    int interval = 0;
    for (int pageNum = 0; pageNum < pages.size(); ++pageNum)
    {
        if(!pages[pageNum].isPdf()){
            if(pdfIntervals.isEmpty() || pdfIntervals.last().type == pageType::PDF){
                pdfOrNotInterval newInterval;
                newInterval.type = pageType::NOPDF;
                newInterval.begin = pageNum;
                if(!pdfIntervals.isEmpty()){
                    pdfIntervals.last().endInPdf = interval;
                }
                pdfIntervals.append(newInterval);
            }
        }
        else{
            if(pdfIntervals.isEmpty() || pdfIntervals.last().type == pageType::NOPDF){
                pdfOrNotInterval newInterval;
                newInterval.type = pageType::PDF;
                newInterval.begin = pageNum;
                newInterval.beginInPdf = interval+1;
                pdfIntervals.append(newInterval);
            }
            ++interval;
        }

        if(pages[pageNum].isPdf()){
            pages[pageNum].paintForPdfExport(painter, 1);
        }
        else{
            if (pages[pageNum].backgroundColor() != QColor("white"))
            {
                QRectF pageRect = pdfWriter.pageRect(QPrinter::Point);
                pageRect.translate(-pageRect.topLeft());
                painter.fillRect(pageRect, pages[pageNum].backgroundColor());
            }
            pages[pageNum].paint(painter, 1);
        }

        if (pageNum + 1 < pages.size())
        {
            pdfWriter.setPageSize(QPageSize(QSize(pages[pageNum + 1].width(), pages[pageNum + 1].height())));
            pdfWriter.newPage();
        }
    }
    painter.end();

    for(int i = pdfIntervals.size()-1; i >= 0; --i){
        if(pdfIntervals[i].type == pageType::PDF){
            pdfIntervals[i].endInPdf = interval;
            break;
        }
    }

    QStringList pdfExtendIntervals;
    for(int i = 0; i < pdfIntervals.size(); ++i){
        if(pdfIntervals.at(i).type == pageType::PDF){
            pdfExtendIntervals << "A"+QString::number(pdfIntervals.at(i).beginInPdf)+"-"+QString::number(pdfIntervals.at(i).endInPdf);
        }
        else{
            if(i == pdfIntervals.size()-1){
                pdfExtendIntervals << "B"+QString::number(pdfIntervals.at(i).begin+1)+"-"+QString::number(pages.size());
            }
            else{
                pdfExtendIntervals << "B"+QString::number(pdfIntervals.at(i).begin+1)+"-"+QString::number(pdfIntervals.at(i+1).begin);
            }
        }
    }
//    for(int i = 0; i < pdfExtendIntervals.size(); ++i){
//        qDebug() << pdfExtendIntervals[i];
//    }

    QString extended = "extended.pdf";
    QString extendedPdfFileName = QUrl(fileName).adjusted(QUrl::RemoveFilename).toString();
    extendedPdfFileName += extended;

    QProcess extendProcess;
    extendProcess.start("pdftk", QStringList() << "A="+m_pdfPath << "B="+overlayFileName <<
                        "cat" << pdfExtendIntervals << "output" << extendedPdfFileName);

    if(extendProcess.error() == QProcess::FailedToStart){
        QMessageBox::StandardButton answer = QMessageBox::warning(nullptr, QObject::tr("Failed to start pdftk"), QObject::tr("Failed to start pdftk. \nCheck if you have "
                                                                   "pdftk installed and the permission to start it.\n"
                                                                   "Do you want to resume (PDF will be exported as image)?"),
                             QMessageBox::Ignore | QMessageBox::Abort);
        if(answer == QMessageBox::Ignore){
            exportPDFAsImage(fileName);
            return;
        }
        else{
            return;
        }
        QDir dir(overlayFileName);
        dir.remove(overlay);
    }
    extendProcess.waitForFinished();

    QProcess process;
    process.start("pdftk", QStringList() << extendedPdfFileName << "multistamp" <<
                  overlayFileName << "output" << fileName);
    process.waitForFinished();
    QDir dir(QUrl(fileName).adjusted(QUrl::RemoveFilename).toString());
    //dir.remove(overlay);
    //dir.remove(extended);
}

void Document::exportPDFAsImage(QString fileName){
    QPrinter pdfWriter(QPrinter::HighResolution);
    pdfWriter.setOutputFormat(QPrinter::PdfFormat);
    pdfWriter.setOutputFileName(fileName);
    pdfWriter.setPageSize(QPageSize(QSizeF(pages[0].width(), pages[0].height()), QPageSize::Point));
    pdfWriter.setPageMargins(QMarginsF(0, 0, 0, 0));
    //*2 is a little bit arbitrarily
    pdfWriter.setResolution(72*2);
    pdfWriter.pageLayout().setUnits(QPageLayout::Point);
    QPainter painter;
    painter.begin(&pdfWriter);
    painter.setRenderHint(QPainter::Antialiasing, true);
    for (int pageNum = 0; pageNum < pages.size(); ++pageNum)
    {
        if (pages[pageNum].backgroundColor() != QColor("white"))
        {
            QRectF pageRect = pdfWriter.pageRect(QPrinter::Point);
            pageRect.translate(-pageRect.topLeft());
            painter.fillRect(pageRect, pages[pageNum].backgroundColor());
        }
        //2 (as zoom) is a little bit arbitrarily (has to be the same number as above)
        pages[pageNum].paint(painter, 2, QRect(0, 0, 0, 0));

        if (pageNum + 1 < pages.size())
        {
            pdfWriter.setPageSize(QPageSize(QSize(pages[pageNum + 1].width(), pages[pageNum + 1].height())));
            pdfWriter.newPage();
        }
    }
    painter.end();
}

bool Document::loadXOJ(QString fileName)
{
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly))
    {
        return false;
    }

    QXmlStreamReader reader;

    // check if it is a gzipped xoj
    QByteArray s = file.read(2);
    if (s.size() == 2)
    {
        if (s.at(0) == static_cast<char>(0x1f) && s.at(1) == static_cast<char>(0x8b))
        {
            // this is a gzipped file
            file.reset();
            QByteArray compressedData = file.readAll();
            QByteArray uncompressedData;
            if (!QCompressor::gzipDecompress(compressedData, uncompressedData))
            {
                return false;
            }
            reader.addData(uncompressedData);
        }
        else
        {
            file.reset();
            reader.setDevice(&file);
        }
    }
    else
    {
        return false;
    }

    pages.clear();

    int strokeCount = 0;

    while (!reader.atEnd())
    {
        reader.readNext();
        if (reader.name() == "page" && reader.tokenType() == QXmlStreamReader::StartElement)
        {
            QXmlStreamAttributes attributes = reader.attributes();
            QStringRef width = attributes.value("", "width");
            QStringRef height = attributes.value("", "height");
            Page newPage;
            newPage.setWidth(width.toDouble());
            newPage.setHeight(height.toDouble());

            pages.append(newPage);
        }
        if (reader.name() == "background" && reader.tokenType() == QXmlStreamReader::StartElement)
        {
            QXmlStreamAttributes attributes = reader.attributes();
            QStringRef type = attributes.value("", "type");
            if(type == "pdf"){
                QStringRef pdfPath = attributes.value("", "filename");
                if(!pdfPath.isEmpty()){
                    m_pdfPath = pdfPath.toString();
                    m_pdfDoc.reset(Poppler::Document::load(m_pdfPath));
                    m_pdfDoc->setRenderHint(Poppler::Document::Antialiasing);
                    m_pdfDoc->setRenderHint(Poppler::Document::TextAntialiasing);
                }
                QStringRef pdfPageNumStr = attributes.value("", "pageno");
                int pdfPageNum = pdfPageNumStr.toInt();
                Poppler::Page* page = m_pdfDoc->page(pdfPageNum-1);
                pages.last().setPdf(page, pdfPageNum-1, false);

                continue;
            }
            QStringRef color = attributes.value("", "color");
            QColor newColor = stringToColor(color.toString());
            pages.last().setBackgroundColor(newColor);
        }
        if (reader.name() == "text" && reader.tokenType() == QXmlStreamReader::StartElement){
            QXmlStreamAttributes attributes = reader.attributes();
            QStringRef fontString = attributes.value("", "font");
            QFont font = QFont(fontString.toString());
            QStringRef sizeString = attributes.value("", "size");
            font.setPointSize(sizeString.toFloat());
            QStringRef xString = attributes.value("", "x");
            QStringRef yString = attributes.value("", "y");
            QStringRef colorString = attributes.value("", "color");
            QColor color = stringToColor(colorString.toString());
            QString text = reader.readElementText();
            //QPointF point(xString.toFloat(), yString.toFloat());
            QRectF rect(xString.toDouble(), yString.toDouble(), 0, 0);
            pages.last().appendText(rect, font, color, text);
        }
        if (reader.name() == "stroke" && reader.tokenType() == QXmlStreamReader::StartElement)
        {
            QXmlStreamAttributes attributes = reader.attributes();
            QStringRef tool = attributes.value("", "tool");
            if (tool == "pen" || tool == "highlighter")
            {
                Stroke newStroke;
                newStroke.pattern = MrDoc::solidLinePattern;
                QStringRef color = attributes.value("", "color");
                if(tool == "highlighter"){
                    newStroke.isHighlighter = true;
                    QColor highlighterColor = stringToColor(color.toString());
                    highlighterColor.setAlpha(127);
                    newStroke.color = highlighterColor;
                }
                else{
                    newStroke.color = stringToColor(color.toString());
                }
                QStringRef strokeWidth = attributes.value("", "width");
                QStringList strokeWidthList = strokeWidth.toString().split(" ");
                newStroke.penWidth = strokeWidthList.at(0).toDouble();
                newStroke.pressures.append(newStroke.penWidth / strokeWidthList.at(0).toDouble());
                for (int i = 1; i < strokeWidthList.size(); ++i)
                {
                    newStroke.pressures.append(2 * strokeWidthList.at(i).toDouble() / newStroke.penWidth - newStroke.pressures.at(i - 1));
                }
                QString elementText = reader.readElementText();
                QStringList elementTextList = elementText.split(" ");
                for (int i = 0; i + 1 < elementTextList.size(); i = i + 2)
                {
                    newStroke.points.append(QPointF(elementTextList.at(i).toDouble(), elementTextList.at(i + 1).toDouble()));
                }
                while (newStroke.points.size() > newStroke.pressures.size())
                {
                    newStroke.pressures.append(1.0);
                }
                pages.last().appendStroke(newStroke);
                strokeCount++;
                qDebug() << strokeCount;
            }
        }
    }

    //    QFileInfo fileInfo(file);
    file.close();

    for (auto &page : pages)
    {
        page.clearDirtyRect();
    }

    if (reader.hasError())
    {
        return false;
    }
    else
    {
        setDocumentChanged(true);
        return true;
    }
}

bool Document::saveXOJ(QString fileName)
{
  QFile file(fileName);
  if (!file.open(QIODevice::WriteOnly))
  {
    return false;
  }

  QByteArray uncompressedData;

  QXmlStreamWriter writer(&uncompressedData);
  //  QXmlStreamWriter writer;

  writer.setAutoFormatting(true);

  //  writer.setDevice(&file);

  writer.writeStartDocument("1.0", false);
  writer.writeStartElement("xournal");
  writer.writeAttribute(QXmlStreamAttribute("version", "0.4.8"));

  writer.writeStartElement("title");
  writer.writeCharacters("Xournal document - see http://math.mit.edu/~auroux/software/xournal/");
  writer.writeEndElement();

  bool pdfFileNameSet = false;

  for (int i = 0; i < pages.size(); ++i)
  {
    writer.writeStartElement("page");
    writer.writeAttribute(QXmlStreamAttribute("width", QString::number(pages[i].width())));
    writer.writeAttribute(QXmlStreamAttribute("height", QString::number(pages[i].height())));
    writer.writeEmptyElement("background");

    if(pages[i].isPdf()){
        writer.writeAttribute(QXmlStreamAttribute("type", "pdf"));
        if(!pdfFileNameSet){
            writer.writeAttribute(QXmlStreamAttribute("domain", "absolute"));
            writer.writeAttribute(QXmlStreamAttribute("filename", m_pdfPath));
            pdfFileNameSet = true;
        }
        writer.writeAttribute(QXmlStreamAttribute("pageno", QString::number(pages[i].pageNum()+1)));
    }
    else{
        writer.writeAttribute(QXmlStreamAttribute("type", "solid"));
        writer.writeAttribute(QXmlStreamAttribute("color", toRGBA(pages[i].backgroundColor().name(QColor::HexArgb))));
        writer.writeAttribute(QXmlStreamAttribute("style", "plain"));
    }

    writer.writeStartElement("layer");

    for (auto t : pages[i].texts()){
        writer.writeStartElement("text");
        writer.writeAttribute(QXmlStreamAttribute("font", std::get<1>(t).key()));
        writer.writeAttribute(QXmlStreamAttribute("size", QString::number(std::get<1>(t).pointSize())));
        writer.writeAttribute(QXmlStreamAttribute("x", QString::number(std::get<0>(t).x())));
        writer.writeAttribute(QXmlStreamAttribute("y", QString::number(std::get<0>(t).y())));
        writer.writeAttribute(QXmlStreamAttribute("color", toRGBA(std::get<2>(t).name(QColor::HexArgb))));
        writer.writeCharacters(std::get<3>(t));
        writer.writeEndElement();
    }
    //    for (int j = 0; j < pages[i].m_strokes.size(); ++j)
    for (auto strokes : pages[i].strokes())
    {
      writer.writeStartElement("stroke");
      if(strokes.isHighlighter){
          writer.writeAttribute(QXmlStreamAttribute("tool", "highlighter"));
          QColor highlighterColor = strokes.color;
          highlighterColor.setAlpha(127);
          writer.writeAttribute(QXmlStreamAttribute("color", toRGBA(highlighterColor.name(QColor::HexArgb))));
      }
      else{
          writer.writeAttribute(QXmlStreamAttribute("tool", "pen"));
          writer.writeAttribute(QXmlStreamAttribute("color", toRGBA(strokes.color.name(QColor::HexArgb))));
      }
      qreal width = strokes.penWidth;
      QString widthString;
      widthString.append(QString::number(width));
      for (int k = 0; k < strokes.pressures.size() - 1; ++k)
      {
        qreal p0 = strokes.pressures[k];
        qreal p1 = strokes.pressures[k + 1];
        widthString.append(' ');
        widthString.append(QString::number(0.5 * (p0 + p1) * width));
      }
      writer.writeAttribute(QXmlStreamAttribute("width", widthString));
      for (int k = 0; k < strokes.points.size(); ++k)
      {
        writer.writeCharacters(QString::number(strokes.points[k].x()));
        writer.writeCharacters(" ");
        writer.writeCharacters(QString::number(strokes.points[k].y()));
        writer.writeCharacters(" ");
      }
      writer.writeEndElement(); // closing "stroke"
    }

    writer.writeEndElement(); // closing "layer"
    writer.writeEndElement(); // closing "page"
  }

  writer.writeEndDocument();

  QByteArray compressedData;

  QCompressor::gzipCompress(uncompressedData, compressedData);

  file.write(compressedData);

  //  QFileInfo fileInfo(file);

  file.close();

  if (writer.hasError())
  {
    return false;
  }
  else
  {
    return true;
  }
}

bool Document::loadMOJ(QString fileName)
{
  QFile file(fileName);
  if (!file.open(QIODevice::ReadOnly))
  {
    return false;
  }

  QXmlStreamReader reader;

  // check if it is a gzipped moj
  QByteArray s = file.read(2);
  if (s.size() == 2)
  {
    if (s.at(0) == static_cast<char>(0x1f) && s.at(1) == static_cast<char>(0x8b))
    {
      // this is a gzipped file
      file.reset();
      QByteArray compressedData = file.readAll();
      QByteArray uncompressedData;
      if (!QCompressor::gzipDecompress(compressedData, uncompressedData))
      {
        return false;
      }
      reader.addData(uncompressedData);
    }
    else
    {
      file.reset();
      reader.setDevice(&file);
    }
  }
  else
  {
    return false;
  }

  pages.clear();

  while (!reader.atEnd())
  {
    reader.readNext();
    if (reader.name() == "MrWriter" && reader.tokenType() == QXmlStreamReader::StartElement)
    {
      QXmlStreamAttributes attributes = reader.attributes();
      QStringRef docversion = attributes.value("document-version");
      if (docversion.toInt() > DOC_VERSION)
      {
        // TODO warn about newer document version
      }
    }
    if (reader.name() == "page" && reader.tokenType() == QXmlStreamReader::StartElement)
    {
      QXmlStreamAttributes attributes = reader.attributes();
      QStringRef width = attributes.value("", "width");
      QStringRef height = attributes.value("", "height");
      Page newPage;
      newPage.setWidth(width.toDouble());
      newPage.setHeight(height.toDouble());

      pages.append(newPage);
    }
    if (reader.name() == "background" && reader.tokenType() == QXmlStreamReader::StartElement)
    {
      QXmlStreamAttributes attributes = reader.attributes();
      QStringRef type = attributes.value("", "type");
      if(type == "pdf"){
          QStringRef pdfPath = attributes.value("", "filename");
          if(!pdfPath.isEmpty()){
              m_pdfPath = pdfPath.toString();
              m_pdfDoc.reset(Poppler::Document::load(m_pdfPath));
              m_pdfDoc->setRenderHint(Poppler::Document::Antialiasing);
              m_pdfDoc->setRenderHint(Poppler::Document::TextAntialiasing);
          }
          QStringRef pdfPageNumStr = attributes.value("", "pageno");
          int pdfPageNum = pdfPageNumStr.toInt();
          Poppler::Page* page = m_pdfDoc->page(pdfPageNum-1);
          pages.last().setPdf(page, pdfPageNum-1, false);

          continue;
      }
      QStringRef color = attributes.value("", "color");
      QColor newColor = stringToColor(color.toString());
      pages.last().setBackgroundColor(newColor);

      QStringRef backgroundTypeString = attributes.value("", "backgroundType");
      MrDoc::Page::backgroundType backgroundType = MrDoc::Page::backgroundType::PLAIN;
      if(backgroundTypeString.toString() == QString("plain")){
          backgroundType = MrDoc::Page::backgroundType::PLAIN;
      }
      else if(backgroundTypeString.toString() == QString("squared")){
          backgroundType = MrDoc::Page::backgroundType::SQUARED;
      }
      else if(backgroundTypeString.toString() == QString("ruled")){
          backgroundType = MrDoc::Page::backgroundType::RULED;
      }
      pages.last().setBackgroundType(backgroundType);
    }
    if (reader.name() == "text" && reader.tokenType() == QXmlStreamReader::StartElement){
        QXmlStreamAttributes attributes = reader.attributes();
        QStringRef fontString = attributes.value("", "font");
        QFont font = QFont(fontString.toString());
        QStringRef sizeString = attributes.value("", "size");
        font.setPointSize(sizeString.toFloat());
        QStringRef xString = attributes.value("", "x");
        QStringRef yString = attributes.value("", "y");
        QStringRef colorString = attributes.value("", "color");
        QColor color = stringToColor(colorString.toString());
        QString text = reader.readElementText();
        //QPointF point(xString.toFloat(), yString.toFloat());
        QRectF rect(xString.toDouble(), yString.toDouble(), 0, 0);
        pages.last().appendText(rect, font, color, text);
    }
    if (reader.name() == "markdown" && reader.tokenType() == QXmlStreamReader::StartElement){
        QXmlStreamAttributes attributes = reader.attributes();
        QStringRef xString = attributes.value("", "x");
        QStringRef yString = attributes.value("", "y");
        QStringRef widthString = attributes.value("", "width");
        QStringRef heightString = attributes.value("", "height");
        int x = xString.toInt();
        int y = yString.toInt();
        qreal width = static_cast<qreal>(widthString.toDouble());
        qreal height = static_cast<qreal>(heightString.toDouble());
        QString text = reader.readElementText();
        pages.last().appendMarkdown(QRectF(x, y, width, height), text);
    }
    if (reader.name() == "stroke" && reader.tokenType() == QXmlStreamReader::StartElement)
    {
      QXmlStreamAttributes attributes = reader.attributes();
      QStringRef tool = attributes.value("", "tool");
      if (tool == "pen" || tool == "highlighter")
      {
        Stroke newStroke;
        if(tool == "highlighter"){
            newStroke.isHighlighter = true;
        }
        newStroke.pattern = MrDoc::solidLinePattern;
        QStringRef color = attributes.value("", "color");
        newStroke.color = stringToColor(color.toString());
        QStringRef style = attributes.value("", "style");
        if (style.toString().compare("solid") == 0)
        {
          newStroke.pattern = MrDoc::solidLinePattern;
        }
        else if (style.toString().compare("dash") == 0)
        {
          newStroke.pattern = MrDoc::dashLinePattern;
        }
        else if (style.toString().compare("dashdot") == 0)
        {
          newStroke.pattern = MrDoc::dashDotLinePattern;
        }
        else if (style.toString().compare("dot") == 0)
        {
          newStroke.pattern = MrDoc::dotLinePattern;
        }
        else
        {
          newStroke.pattern = MrDoc::solidLinePattern;
        }
        QStringRef strokeWidth = attributes.value("", "width");
        newStroke.penWidth = strokeWidth.toDouble();
        QString elementText = reader.readElementText();
        QStringList elementTextList = elementText.trimmed().split(" ");
        for (int i = 0; i + 1 < elementTextList.size(); i = i + 2)
        {
          newStroke.points.append(QPointF(elementTextList.at(i).toDouble(), elementTextList.at(i + 1).toDouble()));
        }
        QStringRef pressures = attributes.value("pressures");
        QStringList pressuresList = pressures.toString().trimmed().split(" ");
        for (int i = 0; i < pressuresList.length(); ++i)
        {
          if (pressuresList.length() == 0)
          {
            newStroke.pressures.append(1.0);
          }
          else
          {
            newStroke.pressures.append(pressuresList.at(i).toDouble());
          }
        }
        if (newStroke.pressures.size() != newStroke.points.size())
        {
          return false;
        }
        pages.last().appendStroke(newStroke);
      }
    }
  }

  QFileInfo fileInfo(file);
  file.close();

  for (auto &page : pages)
  {
    page.clearDirtyRect();
  }

  if (reader.hasError())
  {
    return false;
  }
  else
  {
    m_path = fileInfo.absolutePath();
    m_docName = fileInfo.completeBaseName();
    return true;
  }
}

bool Document::saveMOJ(QString fileName)
{
  QFile file(fileName);
  if (!file.open(QIODevice::WriteOnly))
  {
    return false;
  }

  QByteArray uncompressedData;

  QXmlStreamWriter writer(&uncompressedData);

  writer.setAutoFormatting(true);

  //  writer.setDevice(&file);

  writer.writeStartDocument("1.0", false);
  writer.writeStartElement("MrWriter");
  QString version;
  version.append(QString::number(MAJOR_VERSION)).append(".").append(QString::number(MINOR_VERSION)).append(QString::number(PATCH_VERSION));
  writer.writeAttribute(QXmlStreamAttribute("version", version));
  QString docVersion = QString::number(DOC_VERSION);
  writer.writeAttribute(QXmlStreamAttribute("docversion", docVersion));

  writer.writeStartElement("title");
  writer.writeCharacters("MrWriter document - see http://unruhschuh.com/mrwriter/");
  writer.writeEndElement();

  bool pdfFileNameSet = false;

  for (int i = 0; i < pages.size(); ++i)
  {
    writer.writeStartElement("page");
    writer.writeAttribute(QXmlStreamAttribute("width", QString::number(pages[i].width())));
    writer.writeAttribute(QXmlStreamAttribute("height", QString::number(pages[i].height())));
    writer.writeEmptyElement("background");

    if(pages[i].isPdf()){
        writer.writeAttribute(QXmlStreamAttribute("type", "pdf"));
        if(!pdfFileNameSet){
            writer.writeAttribute(QXmlStreamAttribute("domain", "absolute"));
            writer.writeAttribute(QXmlStreamAttribute("filename", m_pdfPath));
            pdfFileNameSet = true;
        }
        writer.writeAttribute(QXmlStreamAttribute("pageno", QString::number(pages[i].pageNum()+1)));
    }
    else{
        writer.writeAttribute(QXmlStreamAttribute("type", "solid"));
        writer.writeAttribute(QXmlStreamAttribute("color", toRGBA(pages[i].backgroundColor().name(QColor::HexArgb))));
        writer.writeAttribute(QXmlStreamAttribute("style", "plain"));
    }

    QString backgroundTypeString;
    switch(pages[i].getBackgroundType()){
        case MrDoc::Page::backgroundType::PLAIN : backgroundTypeString = "plain"; break;
        case MrDoc::Page::backgroundType::SQUARED : backgroundTypeString = "squared"; break;
        case MrDoc::Page::backgroundType::RULED : backgroundTypeString = "ruled"; break;
    }
    writer.writeAttribute(QXmlStreamAttribute("backgroundType", backgroundTypeString));

//    writer.writeAttribute(QXmlStreamAttribute("type", "solid"));
//    writer.writeAttribute(QXmlStreamAttribute("color", toRGBA(pages[i].backgroundColor().name(QColor::HexArgb))));
//    writer.writeAttribute(QXmlStreamAttribute("style", "plain"));

    writer.writeStartElement("layer");

    for (auto t : pages[i].texts()){
        writer.writeStartElement("text");
        writer.writeAttribute(QXmlStreamAttribute("font", std::get<1>(t).key()));
        writer.writeAttribute(QXmlStreamAttribute("size", QString::number(std::get<1>(t).pointSize())));
        writer.writeAttribute(QXmlStreamAttribute("x", QString::number(std::get<0>(t).x())));
        writer.writeAttribute(QXmlStreamAttribute("y", QString::number(std::get<0>(t).y())));
        writer.writeAttribute(QXmlStreamAttribute("color", toRGBA(std::get<2>(t).name(QColor::HexArgb))));
        writer.writeCharacters(std::get<3>(t));
        writer.writeEndElement();
    }
    for (auto t : pages[i].markdowns()){
        writer.writeStartElement("markdown");
        writer.writeAttribute(QXmlStreamAttribute("x", QString::number(std::get<0>(t).x())));
        writer.writeAttribute(QXmlStreamAttribute("y", QString::number(std::get<0>(t).y())));
        writer.writeAttribute(QXmlStreamAttribute("width", QString::number(std::get<0>(t).width())));
        writer.writeAttribute(QXmlStreamAttribute("height", QString::number(std::get<0>(t).height())));
        writer.writeCharacters(std::get<1>(t));
        writer.writeEndElement();
    }

    //    for (int j = 0; j < pages[i].m_strokes.size(); ++j)
    for (auto strokes : pages[i].strokes())
    {
      writer.writeStartElement("stroke");
      if(strokes.isHighlighter)
          writer.writeAttribute(QXmlStreamAttribute("tool", "highlighter"));
      else
        writer.writeAttribute(QXmlStreamAttribute("tool", "pen"));
      writer.writeAttribute(QXmlStreamAttribute("color", toRGBA(strokes.color.name(QColor::HexArgb))));
      QString patternString;
      if (strokes.pattern == MrDoc::solidLinePattern)
      {
        patternString = "solid";
      }
      else if (strokes.pattern == MrDoc::dashLinePattern)
      {
        patternString = "dash";
      }
      else if (strokes.pattern == MrDoc::dashDotLinePattern)
      {
        patternString = "dashdot";
      }
      else if (strokes.pattern == MrDoc::dotLinePattern)
      {
        patternString = "dot";
      }
      else
      {
        patternString = "solid";
      }
      writer.writeAttribute(QXmlStreamAttribute("style", patternString));
      qreal width = strokes.penWidth;
      writer.writeAttribute(QXmlStreamAttribute("width", QString::number(width)));
      QString pressures;
      for (int k = 0; k < strokes.pressures.length(); ++k)
      {
        pressures.append(QString::number(strokes.pressures[k])).append(" ");
      }
      writer.writeAttribute((QXmlStreamAttribute("pressures", pressures.trimmed())));
      QString points;
      for (int k = 0; k < strokes.points.size(); ++k)
      {
        points.append(QString::number(strokes.points[k].x()));
        points.append(" ");
        points.append(QString::number(strokes.points[k].y()));
        points.append(" ");
      }
      writer.writeCharacters(points.trimmed());
      writer.writeEndElement(); // closing "stroke"
    }

    writer.writeEndElement(); // closing "layer"
    writer.writeEndElement(); // closing "page"
  }

  writer.writeEndDocument();

  QByteArray compressedData;
  QCompressor::gzipCompress(uncompressedData, compressedData);
  file.write(compressedData);

  QFileInfo fileInfo(file);

  file.close();

  if (writer.hasError())
  {
    return false;
  }
  else
  {
    setDocumentChanged(false);

    m_path = fileInfo.absolutePath();
    m_docName = fileInfo.completeBaseName();
    return true;
  }
}

bool Document::loadPDF(QString fileName){
    pages.clear();
    if(!fileName.isEmpty()){
        m_pdfPath = fileName;
        m_pdfDoc.reset(Poppler::Document::load(m_pdfPath));
        if(m_pdfDoc->isLocked()){
            return false;
        }
        m_pdfDoc->setRenderHint(Poppler::Document::Antialiasing);
        m_pdfDoc->setRenderHint(Poppler::Document::TextAntialiasing);
        int numPages = m_pdfDoc->numPages();
        for(int i = 0; i < numPages; ++i){
            Poppler::Page* page = m_pdfDoc->page(i);
            pages.append(Page());
            pages.last().setPdf(page, i, true);
        }
        return true;
    }
    return false;
}

bool Document::setDocName(QString docName)
{
  // check for special characters not to be used in filenames ... (probably
  // not)
  m_docName = docName;
  return true;
}

QString Document::docName()
{
  return m_docName;
}

bool Document::setPath(QString path)
{
  m_path = path;
  return true;
}

QString Document::path()
{
  return m_path;
}

bool Document::documentChanged()
{
  return m_documentChanged;
}

void Document::setDocumentChanged(bool changed)
{
  m_documentChanged = changed;
}

QString Document::toARGB(QString rgba)
{
  // #RRGGBBAA
  // 012345678
  QString argb;
  if (rgba.length() == 9)
  {
    argb.append('#');
    argb.append(rgba.mid(7, 2));
    argb.append(rgba.mid(1, 6));
  }
  else
  {
    argb = QString("");
  }

  return argb;
}

QString Document::toRGBA(QString argb)
{
  // #AARRGGBB
  // 012345678
  QString rgba;
  if (argb.length() == 9)
  {
    rgba.append('#');
    rgba.append(argb.mid(3, 6));
    rgba.append(argb.mid(1, 2));
  }
  else
  {
    rgba = QString("");
  }

  return rgba;
}

QColor Document::stringToColor(QString colorString)
{
  QColor color;
  if (colorString.left(1).compare("#") == 0)
  {
    color = QColor(toARGB(colorString));
  }
  else
  {
    for (int i = 0; i < standardColors.size(); ++i)
    {
      if (standardColorNames[i].compare(colorString) == 0)
      {
        color = standardColors.at(i);
      }
    }
  }
  return color;
}
}
