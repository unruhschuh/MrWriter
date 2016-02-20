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

#include "colorbutton.h"

class PageSettingsDialog : public QDialog
{
  Q_OBJECT
public:
  explicit PageSettingsDialog(const QSizeF &pageSize, const QColor &backgroundColor, QWidget *parent = 0);
  QSizeF m_currentPageSize;
  QColor m_backgroundColor;

signals:

public slots:
  void textChanged();
  void standardPaperSizesComboChanged();
  void chooseBackgroundColor();
  void swapWidthHeight();

private:
  QVector<QSizeF> m_myStandardPageSizes;
  QVector<QString> m_myStandardPageSizeNames;

  QComboBox *m_standardPaperSizesComboBox;
  QLineEdit *m_widthLineEdit;
  QLineEdit *m_heightLineEdit;
  QPushButton *m_swapWidthHeightButton;
  ColorButton *m_colorButton;
  QCheckBox *m_scaleContentCheckBox;
  QCheckBox *m_applyToAllCheckBox;

  QPushButton *m_okButton;
  QPushButton *m_cancelButton;
};

#endif // PAGESETTINGSDIALOG_H
