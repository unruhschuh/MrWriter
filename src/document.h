#ifndef DOCUMENT_H
#define DOCUMENT_H

#include "page.h"

#include <QVector>

namespace MrDoc
{

class Document
{
public:
  Document();
  Document(const Document &doc);

  void exportPDF(QString fileName);

  bool loadXOJ(QString fileName);
  bool saveXOJ(QString fileName);

  bool loadMOJ(QString fileName);
  bool saveMOJ(QString fileName);

  void paintPage(size_t pageNum, QPainter &painter, qreal zoom);

  bool setDocName(QString docName);
  QString docName();

  bool setPath(QString path);
  QString path();

  bool documentChanged();
  void setDocumentChanged(bool changed);

  std::vector<MrDoc::Page> pages;
  //std::vector<std::unique_ptr<MrDoc::Page>> pages;

private:
  bool m_documentChanged;

  QString m_docName;
  QString m_path;
};
}

#endif // DOCUMENT_H
