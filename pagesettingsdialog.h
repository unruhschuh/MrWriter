#ifndef PAGESETTINGSDIALOG_H
#define PAGESETTINGSDIALOG_H

#include <QDialog>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>

#include <QString>
#include <QPageSize>
#include <QColor>

#include <map>

#include "colorbutton.h"
#include "page.h"

class PageSettingsDialog : public QDialog
{
  Q_OBJECT
public:
  explicit PageSettingsDialog(const QSizeF &newPageSize, const QColor &newBackgroundColor, const MrDoc::Page::backgroundType newBackgroundType, QWidget *parent = 0);
  QSizeF currentPageSize;
  QColor backgroundColor;
  MrDoc::Page::backgroundType m_backgroundType;

signals:

public slots:
  void textChanged();
  void standardPaperSizesComboChanged();
  void chooseBackgroundColor();
  void swapWidthHeight();

private:
  QVector<QSizeF> myStandardPageSizes;
  QVector<QString> myStandardPageSizeNames;

  QVector<MrDoc::Page::backgroundType> backgroundTypes;
  QVector<QString> backgroundTypeNames;

  QComboBox *standardPaperSizesComboBox;
  QLineEdit *widthLineEdit;
  QLineEdit *heightLineEdit;
  QPushButton *swapWidthHeightButton;
  ColorButton *colorButton;
  QComboBox *backgroundTypesComboBox;
  QCheckBox *scaleContentCheckBox;
  QCheckBox *applyToAllCheckBox;

  QPushButton *okButton;
  QPushButton *cancelButton;
};

#endif // PAGESETTINGSDIALOG_H
