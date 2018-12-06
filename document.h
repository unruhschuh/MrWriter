#ifndef DOCUMENT_H
#define DOCUMENT_H

#include <QMessageBox>
#include <QPushButton>
#include <QObject>
#include <QDir>
#include <QProcess>
#include "page.h"
#include <poppler-qt5.h>
#include <memory>

#include <QVector>

namespace MrDoc
{

class Document
{
public:
  Document();
  /*Document(const Document &doc);

  Document& operator= (const Document& doc);*/

  /**
   * @brief exportPDF exports the document as pdf using pdftk
   * @param fileName is the full path
   */
  void exportPDF(QString fileName);
  /**
   * @brief exportPDFAsImage exports the document as pdf but only as images
   * @details It is not possible to search in the document.
   * @param fileName is the full path
   */
  void exportPDFAsImage(QString fileName);

  /**
   * @brief loadXOJ loads a .xoj (xournal file)
   * @param fileName is the full path
   * @return true if successful opening, otherwise false
   */
  bool loadXOJ(QString fileName);
  /**
   * @brief saveXOJ exports the document as .xoj (xournal file)
   * @param fileName is the full path
   * @return  true if saving was successful, otherwise false
   */
  bool saveXOJ(QString fileName);

  /**
   * @brief loadMOJ loads a .moj (MrWriter file)
   * @param fileName is the full path
   * @return true if opening was successful, otherwise false
   */
  bool loadMOJ(QString fileName);
  /**
   * @brief saveMOJ saves the document (as .moj)
   * @param fileName is the full path
   * @return  true if saving was successful, otherwise false
   */
  bool saveMOJ(QString fileName);

  /**
   * @brief loadPDF loads a .pdf file to annotate it
   * @param fileName is the full path
   * @return true if opening was successful otherwise false
   */
  bool loadPDF(QString fileName);

  void paintPage(int pageNum, QPainter &painter, qreal zoom);

  bool setDocName(QString docName);
  QString docName();

  bool setPath(QString path);
  QString path();

  bool documentChanged();
  void setDocumentChanged(bool changed);

  QVector<MrDoc::Page> pages;

  QString toRGBA(QString argb);
  QString toARGB(QString rgba);

  QColor stringToColor(QString colorString);

private:
  /**
   * @brief The pageType enum is for distinguishing between plain pages and pdf pages
   */
  enum class pageType {
      PDF,
      NOPDF
  };
  bool m_documentChanged;

  QString m_docName;
  QString m_path;
  QString m_pdfPath; /**< path to the underlying pdf file */

  std::shared_ptr<Poppler::Document> m_pdfDoc; /**< pointer to the underlying pdf file (opened with poppler) */
};
}

#endif // DOCUMENT_H
