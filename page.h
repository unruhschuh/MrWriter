#ifndef PAGE_H
#define PAGE_H

#include "stroke.h"
#include <poppler-qt5.h>
#include <QDebug>
#include <QImage>
#include <QTextEdit>
#include <memory>
#include <algorithm>
#include <math.h>

namespace MrDoc
{

class Page
{
public:
    Page();

  qreal width() const;
  qreal height() const;

  void setWidth(qreal width);
  void setHeight(qreal height);

  void setBackgroundColor(QColor backgroundColor);
  QColor backgroundColor(void) const;

  const QRectF &dirtyRect() const;
  void clearDirtyRect();

  bool changePenWidth(int strokeNum, qreal penWidth);
  bool changeStrokeColor(int strokeNum, QColor color);
  bool changeStrokePattern(int strokeNum, QVector<qreal> pattern);

  int textIndexFromMouseClick(int x, int y);
  const QString& textByIndex(int i);
  int appendText(const QRectF& rect, const QFont& font, const QColor& color, const QString& text);
  void setText(int index, const QFont &font, const QColor &color, const QString& text);

  const QRectF& textRectByIndex(int i);
  const QColor& textColorByIndex(int i);
  const QFont& textFontByIndex(int i);

  const QVector<Stroke> &strokes();
  const QVector<std::tuple<QRectF, QFont, QColor, QString> > &texts();

  QVector<QPair<Stroke, int>> getStrokes(QPolygonF selectionPolygon);
  QVector<QPair<Stroke, int>> removeStrokes(QPolygonF selectionPolygon);
  void removeStrokeAt(int i);
  void removeLastStroke();

  void insertStrokes(const QVector<QPair<Stroke, int>> &strokesAndPositions);
  void insertStroke(int position, const Stroke &stroke);

  void appendStroke(const Stroke &stroke);
  void appendStrokes(const QVector<Stroke> &strokes);
  void prependStroke(const Stroke &stroke);

  void setPdf(Poppler::Page *page, int pageNum);
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

  void clearPdfSearch();


  //    virtual void paint(QPainter &painter, qreal zoom);
  /**
   * @brief paint
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

protected:
  QVector<Stroke> m_strokes;
  QImage m_pdf;
  std::shared_ptr<Poppler::Page> m_pdfPointer = std::shared_ptr<Poppler::Page>(nullptr);
  int pageno; //pageNumber
  QList<QRectF> searchResultRects;

  QVector<std::tuple<QRectF, QFont, QColor, QString>> m_texts;

private:
  QColor m_backgroundColor;

  qreal m_width;  // post script units
  qreal m_height; // post script units

  QRectF m_dirtyRect;

  bool rectIsPoint = true; //in m_texts
};
}

#endif // PAGE_H
