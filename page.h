#ifndef PAGE_H
#define PAGE_H

#include "stroke.h"
#include <poppler-qt5.h>
#include <poppler-link.h>
#include <QDebug>
#include <QImage>
#include <QTextEdit>
#include <QTextDocument>
#include <memory>
#include <algorithm>
#include <math.h>
extern "C" {
#include<mkdio.h>
}

namespace MrDoc
{
/**
 * @brief The Page class is the class containing all information about a page. A page can be blank or contain a pdf page to draw on.
 */
class Page
{
public:
    Page();

    enum class backgroundType {
        PLAIN,
        SQUARED,
        RULED
    };

  qreal width() const;
  qreal height() const;

  void setWidth(qreal width);
  void setHeight(qreal height);

  void setBackgroundColor(QColor backgroundColor);
  QColor backgroundColor(void) const;

  void setBackgroundType(backgroundType type);
  backgroundType getBackgroundType() const;

  const QRectF &dirtyRect() const;
  void clearDirtyRect();

  bool changePenWidth(int strokeNum, qreal penWidth);
  bool changeStrokeColor(int strokeNum, QColor color);
  bool changeStrokePattern(int strokeNum, QVector<qreal> pattern);

  /**
   * @brief textIndexFromMouseClick tests if a (mouse click) coordinate is on text
   * @param x x-coordinate of mouse click
   * @param y y-coordinate of mouse click
   * @return index of text in @ref m_texts if found, otherwise -1
   */
  int textIndexFromMouseClick(int x, int y);
  /**
   * @brief textByIndex
   * @param i index in @ref m_texts
   * @return text string
   */
  const QString& textByIndex(int i);
  /**
   * @brief appendText appends a tuple to @ref m_texts
   * @param rect is the rect bounding the text
   * @param font
   * @param color font color
   * @param text
   * @return index of the appended tuple
   */
  int appendText(const QRectF& rect, const QFont& font, const QColor& color, const QString& text);
  /**
   * @brief setText changes the text of tuple provided by @param index or removes the tuple if text is empty.
   * @param index
   * @param font
   * @param color
   * @param text new text. If text is empty, the tuple is removed
   */
  void setText(int index, const QFont &font, const QColor &color, const QString& text);

  /**
   * @brief textRectByIndex
   * @param i
   * @return bounding rect of the text at index @param i
   */
  const QRectF& textRectByIndex(int i);
  /**
   * @brief textColorByIndex
   * @param i
   * @return  text color of the text at index @param i
   */
  const QColor& textColorByIndex(int i);
  /**
   * @brief textFontByIndex
   * @param i
   * @return text font of the text at index @param i
   */
  const QFont& textFontByIndex(int i);


  /**
   * @brief markdownIndexFromMouseClick tests if a (mouse click) coordinate is on a inserted markdown document element
   * @param x x-coordinate of mouse click
   * @param y y-coordinate of mouse click
   * @return index of document in @ref m_markdownDocs if found, otherwise -1
   */
  int markdownIndexFromMouseClick(int x, int y);

  /**
   * @brief appendMarkdown appends a tuple to @ref m_texts
   * @param rect is the bounding rect. If width and height are 0, then width and height are calculated by appendMarkdown.
   * @param text
   * @return index of the new appended tuple
   */
  int appendMarkdown(const QRectF &rect, const QString& text);

  /**
   * @brief setMarkdown changes the text of a "markdown" tuple indexed at @param index or removes the tuple if text is empty.
   * @param index
   * @param text new text. If text is empty, the tuple is removed.
   */
  void setMarkdown(int index, const QString& text, const QRectF &rect);

  QString markdownByIndex(int i);
  QRectF markdownRectByIndex(int i);

  const QVector<Stroke> &strokes();
  const QVector<std::tuple<QRectF, QFont, QColor, QString> > &texts();
  const QVector<std::tuple<QRectF, QString>>& markdowns();

  QVector<QPair<Stroke, int>> getStrokes(QPolygonF selectionPolygon);
  QVector<QPair<Stroke, int>> removeStrokes(QPolygonF selectionPolygon);
  void removeStrokeAt(int i);
  void removeLastStroke();

  void insertStrokes(const QVector<QPair<Stroke, int>> &strokesAndPositions);
  void insertStroke(int position, const Stroke &stroke);

  void appendStroke(const Stroke &stroke);
  void appendStrokes(const QVector<Stroke> &strokes);
  void prependStroke(const Stroke &stroke);

  /**
   * @brief setPdf sets a pdf page as "background" for the page
   * @param page is the pointer to pdf page (provided by poppler)
   * @param pageNum is the page number (first page is 0)
   * @param adjustSize if true, the size will adjusted to pdf's size
   */
  void setPdf(Poppler::Page *page, int pageNum, bool adjustSize);
  //void setPdfPath(const QString path);

  /**
   * @brief searchPdfNext searches for all occurences of @param text in the page
   * @return true if some text was found, else false
   */
  bool searchPdfNext(const QString& text);
  /**
   * @brief searchPdfPrev searches for all occurences of @param text in the page
   * @return true if some text was found, else false
   * @see searchPdfNext
   */
  bool searchPdfPrev(const QString& text);
  /**
   * @brief clearPdfSearch clears the text search
   */
  void clearPdfSearch();

  /**
   * @brief linkFromMouseClick tests if a (mouse click) coordinate is on a pdf goto-link
   * @param x x-coordinate of mouse click
   * @param y y-coordinate of mouse click
   * @return pointer to the link in if clicked on a link, otherwise nullptr
   */
  Poppler::LinkGoto* linkFromMouseClick(qreal x, qreal y);


  //    virtual void paint(QPainter &painter, qreal zoom);
  /**
   * @brief paint paints the page with @param painter
   * @param painter
   * @param zoom
   * @param region
   */
  virtual void paint(QPainter &painter, qreal zoom, QRectF region = QRect(0, 0, 0, 0));
  void paintForPdfExport(QPainter &painter, qreal zoom);

  //    QVector<Stroke> strokes;

  bool isPdf() const{
      return m_pdfPointer != nullptr;
  }

  int pageNum(){
      return pageno;
  }

  static char* compileMarkdown(const QString& text);

protected:
  QVector<Stroke> m_strokes;
  std::shared_ptr<Poppler::Page> m_pdfPointer = std::shared_ptr<Poppler::Page>(nullptr); /**< pointer to the pdf page to draw on (nullptr, if blank page) */
  int pageno; //pageNumber in the document
  QList<QRectF> searchResultRects; /**< list of the (yellow) rectangles around search results */

  QVector<std::tuple<QRectF, QFont, QColor, QString>> m_texts; /**< contains the texts */
  QVector<std::tuple<QRectF, QString>> m_markdownDocs; /**< contains the inserted markdown documents, QRectF is the bounding rect (zoom factor 1)*/

private:
  QSizeF adjustMarkdownSize(int x, int y, QSizeF oldSize);
  QColor m_backgroundColor;
  backgroundType m_backgroundType;

  qreal m_width;  // post script units
  qreal m_height; // post script units

  QRectF m_dirtyRect;

  bool rectIsPoint = true; //in m_texts
};
}

#endif // PAGE_H
