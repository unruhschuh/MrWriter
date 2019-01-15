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
    pages.push_back(nextPage);
  }
  setDocumentChanged(false);
}

Document::Document(const Document &doc)
{
  for (size_t i = 0; i < doc.pages.size(); ++i)
  {
    pages.push_back(doc.pages.at(i));
  }
}

void Document::paintPage(size_t pageNum, QPainter &painter, qreal zoom)
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
  //    QPdfWriter pdfWriter("/Users/tom/Desktop/qpdfwriter.pdf");
  //    QPdfWriter pdfWriter(fileName);

  QPrinter pdfWriter(QPrinter::HighResolution);
  pdfWriter.setOutputFormat(QPrinter::PdfFormat);
  pdfWriter.setOutputFileName(fileName);
  //    pdfWriter.setMargins();

  pdfWriter.setPageSize(QPageSize(QSizeF(pages[0].width(), pages[0].height()), QPageSize::Point));
  pdfWriter.setPageMargins(QMarginsF(0, 0, 0, 0));
  qreal zoomW = ((qreal)pdfWriter.pageRect().width()) / ((qreal)pdfWriter.paperRect().width());
  qreal zoomH = ((qreal)pdfWriter.pageRect().height()) / ((qreal)pdfWriter.paperRect().height());
  qreal zoom = zoomW;
  if (zoomH < zoomW)
    zoom = zoomH;
  pdfWriter.setResolution(72);
  pdfWriter.pageLayout().setUnits(QPageLayout::Point);
  QPainter painter;

  //    std::cout << "PDF " << pdfWriter.colorCount() << std::endl;

  painter.begin(&pdfWriter);
  painter.setRenderHint(QPainter::Antialiasing, true);
  for (size_t pageNum = 0; pageNum < pages.size(); ++pageNum)
  {
    if (pages[pageNum].backgroundColor() != QColor("white"))
    {
      QRectF pageRect = pdfWriter.pageRect(QPrinter::Point);
      pageRect.translate(-pageRect.topLeft());
      painter.fillRect(pageRect, pages[pageNum].backgroundColor());
    }
    pages[pageNum].paint(painter, zoom, QRect(0, 0, 0, 0));

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

      pages.push_back(std::move(newPage));
      // pages.push_back(std::make_unique<Page>(newPage));
    }
    if (reader.name() == "background" && reader.tokenType() == QXmlStreamReader::StartElement)
    {
      QXmlStreamAttributes attributes = reader.attributes();
      QStringRef color = attributes.value("", "color");
      QColor newColor = stringToColor(color.toString());
      pages.back().setBackgroundColor(newColor);
    }
    if (reader.name() == "stroke" && reader.tokenType() == QXmlStreamReader::StartElement)
    {
      QXmlStreamAttributes attributes = reader.attributes();
      QStringRef tool = attributes.value("", "tool");
      if (tool == "pen")
      {
        Stroke newStroke;
        newStroke.pattern = MrDoc::solidLinePattern;
        QStringRef color = attributes.value("", "color");
        newStroke.color = stringToColor(color.toString());
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
        //pages.back().appendElement(std::make_unique<Element>(newStroke));
        pages.back().appendElement(newStroke.clone());
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

  for (size_t i = 0; i < pages.size(); ++i)
  {
    writer.writeStartElement("page");
    writer.writeAttribute(QXmlStreamAttribute("width", QString::number(pages[i].width())));
    writer.writeAttribute(QXmlStreamAttribute("height", QString::number(pages[i].height())));
    writer.writeEmptyElement("background");
    writer.writeAttribute(QXmlStreamAttribute("type", "solid"));
    writer.writeAttribute(QXmlStreamAttribute("color", toRGBA(pages[i].backgroundColor().name(QColor::HexArgb))));
    writer.writeAttribute(QXmlStreamAttribute("style", "plain"));
    writer.writeStartElement("layer");

    //    for (int j = 0; j < pages[i].m_strokes.size(); ++j)
    for (auto & element : pages[i].elements())
    {
      auto stroke = dynamic_cast<Stroke*>(element.get());
      if (nullptr != stroke)
      {
        writer.writeStartElement("stroke");
        writer.writeAttribute(QXmlStreamAttribute("tool", "pen"));
        writer.writeAttribute(QXmlStreamAttribute("color", toRGBA(stroke->color.name(QColor::HexArgb))));
        qreal width = stroke->penWidth;
        QString widthString;
        widthString.append(QString::number(width));
        for (int k = 0; k < stroke->pressures.size() - 1; ++k)
        {
          qreal p0 = stroke->pressures[k];
          qreal p1 = stroke->pressures[k + 1];
          widthString.append(' ');
          widthString.append(QString::number(0.5 * (p0 + p1) * width));
        }
        writer.writeAttribute(QXmlStreamAttribute("width", widthString));
        for (int k = 0; k < stroke->points.size(); ++k)
        {
          writer.writeCharacters(QString::number(stroke->points[k].x()));
          writer.writeCharacters(" ");
          writer.writeCharacters(QString::number(stroke->points[k].y()));
          writer.writeCharacters(" ");
        }
        writer.writeEndElement(); // closing "stroke"
      }
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

  int strokeCount = 0;

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

      pages.push_back(std::move(newPage));
    }
    if (reader.name() == "background" && reader.tokenType() == QXmlStreamReader::StartElement)
    {
      QXmlStreamAttributes attributes = reader.attributes();
      QStringRef color = attributes.value("", "color");
      QColor newColor = stringToColor(color.toString());
      pages.back().setBackgroundColor(newColor);
    }
    if (reader.name() == "stroke" && reader.tokenType() == QXmlStreamReader::StartElement)
    {
      QXmlStreamAttributes attributes = reader.attributes();
      QStringRef tool = attributes.value("", "tool");
      if (tool == "pen")
      {
        Stroke newStroke;
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
        pages.back().appendElement(std::make_unique<Element>(newStroke));
        strokeCount++;
        qDebug() << strokeCount;
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

  for (size_t i = 0; i < pages.size(); ++i)
  {
    writer.writeStartElement("page");
    writer.writeAttribute(QXmlStreamAttribute("width", QString::number(pages[i].width())));
    writer.writeAttribute(QXmlStreamAttribute("height", QString::number(pages[i].height())));
    writer.writeEmptyElement("background");
    writer.writeAttribute(QXmlStreamAttribute("type", "solid"));
    writer.writeAttribute(QXmlStreamAttribute("color", toRGBA(pages[i].backgroundColor().name(QColor::HexArgb))));
    writer.writeAttribute(QXmlStreamAttribute("style", "plain"));
    writer.writeStartElement("layer");

    //    for (int j = 0; j < pages[i].m_strokes.size(); ++j)
    for (auto & element : pages[i].elements())
    {
      auto stroke = dynamic_cast<Stroke*>(element.get());
      if (nullptr != stroke)
      {
        writer.writeStartElement("stroke");
        writer.writeAttribute(QXmlStreamAttribute("tool", "pen"));
        writer.writeAttribute(QXmlStreamAttribute("color", toRGBA(stroke->color.name(QColor::HexArgb))));
        QString patternString;
        if (stroke->pattern == MrDoc::solidLinePattern)
        {
          patternString = "solid";
        }
        else if (stroke->pattern == MrDoc::dashLinePattern)
        {
          patternString = "dash";
        }
        else if (stroke->pattern == MrDoc::dashDotLinePattern)
        {
          patternString = "dashdot";
        }
        else if (stroke->pattern == MrDoc::dotLinePattern)
        {
          patternString = "dot";
        }
        else
        {
          patternString = "solid";
        }
        writer.writeAttribute(QXmlStreamAttribute("style", patternString));
        qreal width = stroke->penWidth;
        writer.writeAttribute(QXmlStreamAttribute("width", QString::number(width)));
        QString pressures;
        for (int k = 0; k < stroke->pressures.length(); ++k)
        {
          pressures.append(QString::number(stroke->pressures[k])).append(" ");
        }
        writer.writeAttribute((QXmlStreamAttribute("pressures", pressures.trimmed())));
        QString points;
        for (int k = 0; k < stroke->points.size(); ++k)
        {
          points.append(QString::number(stroke->points[k].x()));
          points.append(" ");
          points.append(QString::number(stroke->points[k].y()));
          points.append(" ");
        }
        writer.writeCharacters(points.trimmed());
        writer.writeEndElement(); // closing "stroke"
      }
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
