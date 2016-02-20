#include "pagesettingsdialog.h"

#include <QFormLayout>
#include <QPageSize>
#include <QColorDialog>
#include <QDebug>
#include <QVector>

PageSettingsDialog::PageSettingsDialog(const QSizeF &pageSize, const QColor &backgroundColor, QWidget *parent) : QDialog(parent)
{
  m_backgroundColor = backgroundColor;

  m_standardPaperSizesComboBox = new QComboBox(this);

  m_myStandardPageSizes << QPageSize(QPageSize::A0).sizePoints() << QPageSize(QPageSize::A1).sizePoints() << QPageSize(QPageSize::A2).sizePoints()
                      << QPageSize(QPageSize::A3).sizePoints() << QPageSize(QPageSize::A4).sizePoints() << QPageSize(QPageSize::A5).sizePoints()
                      << QPageSize(QPageSize::A6).sizePoints() << QPageSize(QPageSize::Letter).sizePoints();
  m_myStandardPageSizeNames << QPageSize(QPageSize::A0).name() << QPageSize(QPageSize::A1).name() << QPageSize(QPageSize::A2).name()
                          << QPageSize(QPageSize::A3).name() << QPageSize(QPageSize::A4).name() << QPageSize(QPageSize::A5).name()
                          << QPageSize(QPageSize::A6).name() << QPageSize(QPageSize::Letter).name();
  int I = m_myStandardPageSizes.size();
  for (int i = 0; i < I; ++i)
  {
    QSizeF tmpSize = m_myStandardPageSizes[i];
    tmpSize = QSizeF(tmpSize.height(), tmpSize.width());
    QString tmpName = m_myStandardPageSizeNames[i];
    tmpName.prepend("landscape ");
    m_myStandardPageSizes.append(tmpSize);
    m_myStandardPageSizeNames.append(tmpName);
  }

  m_myStandardPageSizes.append(QSize(1, 1));
  m_myStandardPageSizeNames.append(tr("Custom"));

  int index;
  for (index = 0; index < m_myStandardPageSizes.size(); ++index)
  {
    m_standardPaperSizesComboBox->addItem(m_myStandardPageSizeNames[index], index);
  }
  connect(m_standardPaperSizesComboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(standardPaperSizesComboChanged()));

  QDoubleValidator *validator = new QDoubleValidator;
  validator->setBottom(1.0);

  m_widthLineEdit = new QLineEdit(this);
  m_widthLineEdit->setValidator(validator);
  connect(m_widthLineEdit, SIGNAL(textChanged(QString)), this, SLOT(textChanged()));

  m_heightLineEdit = new QLineEdit(this);
  m_widthLineEdit->setValidator(validator);
  connect(m_heightLineEdit, SIGNAL(textChanged(QString)), this, SLOT(textChanged()));

  m_swapWidthHeightButton = new QPushButton(tr("Swap width and height"), this);
  connect(m_swapWidthHeightButton, SIGNAL(clicked()), this, SLOT(swapWidthHeight()));

  m_colorButton = new ColorButton();
  m_colorButton->setColor(m_backgroundColor);
  connect(m_colorButton, SIGNAL(clicked()), this, SLOT(chooseBackgroundColor()));

  m_okButton = new QPushButton(tr("OK"), this);
  m_okButton->setDefault(true);
  m_cancelButton = new QPushButton(tr("Cancel"), this);

  m_scaleContentCheckBox = new QCheckBox(tr("Scale Content"), this);
  m_scaleContentCheckBox->setChecked(false);

  m_applyToAllCheckBox = new QCheckBox(tr("Apply to all pages"), this);
  m_applyToAllCheckBox->setChecked(false);

  QFormLayout *formLayout = new QFormLayout;
  formLayout->addRow(tr("Standard paper sizes:"), m_standardPaperSizesComboBox);
  formLayout->addRow(tr("&Width (pt):"), m_widthLineEdit);
  formLayout->addRow(tr("&Height (pt):"), m_heightLineEdit);
  formLayout->addRow(tr("Orientation:"), m_swapWidthHeightButton);
  formLayout->addRow(tr("Background Color:"), m_colorButton);
  formLayout->addWidget(m_scaleContentCheckBox);
  formLayout->addWidget(m_applyToAllCheckBox);
  formLayout->addRow(m_okButton, m_cancelButton);

  setLayout(formLayout);

  setWindowTitle(tr("Page Settings"));

  m_widthLineEdit->setText(QString::number(pageSize.width()));
  m_heightLineEdit->setText(QString::number(pageSize.height()));

  connect(m_cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
  connect(m_okButton, SIGNAL(clicked()), this, SLOT(accept()));
}

void PageSettingsDialog::textChanged()
{
  qreal width = m_widthLineEdit->text().toDouble();
  qreal height = m_heightLineEdit->text().toDouble();
  m_currentPageSize = QSizeF(width, height);

  bool foundPageSize = false;
  int index = 0;
  for (index = 0; index < m_myStandardPageSizes.size(); ++index)
  {
    if (width == m_myStandardPageSizes[index].width() && height == m_myStandardPageSizes[index].height())
    {
      foundPageSize = true;
      break;
    }
  }
  m_standardPaperSizesComboBox->blockSignals(true);
  if (foundPageSize)
  {
    m_standardPaperSizesComboBox->setCurrentIndex(index);
  }
  else
  {
    m_standardPaperSizesComboBox->setCurrentIndex(m_standardPaperSizesComboBox->count() - 1);
  }
  m_standardPaperSizesComboBox->blockSignals(false);
}

void PageSettingsDialog::standardPaperSizesComboChanged()
{
  if (m_standardPaperSizesComboBox->currentIndex() != m_myStandardPageSizes.size() - 1)
  {
    m_widthLineEdit->blockSignals(true);
    m_heightLineEdit->blockSignals(true);
    QSizeF tmpPageSize = m_myStandardPageSizes[m_standardPaperSizesComboBox->currentIndex()];
    m_widthLineEdit->setText(QString::number(tmpPageSize.width()));
    m_heightLineEdit->setText(QString::number(tmpPageSize.height()));
    m_widthLineEdit->blockSignals(false);
    m_heightLineEdit->blockSignals(false);

    m_currentPageSize = tmpPageSize;
  }
}

void PageSettingsDialog::chooseBackgroundColor()
{
  QColorDialog colorDialog;
  QColor backgroundColor = colorDialog.getColor(m_backgroundColor);
  if (backgroundColor.isValid())
  {
    m_backgroundColor = backgroundColor;
    m_colorButton->setColor(m_backgroundColor);
  }
}

void PageSettingsDialog::swapWidthHeight()
{
  QString width = m_widthLineEdit->text();
  QString height = m_heightLineEdit->text();

  m_widthLineEdit->blockSignals(true);
  //    heightLineEdit->blockSignals(true);

  m_widthLineEdit->setText(height);
  m_heightLineEdit->setText(width);

  m_widthLineEdit->blockSignals(false);
  //    heightLineEdit->blockSignals(false);
}
