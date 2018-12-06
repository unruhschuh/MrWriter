#include "pagesettingsdialog.h"

#include <QFormLayout>
#include <QPageSize>
#include <QColorDialog>
#include <QDebug>
#include <QVector>

PageSettingsDialog::PageSettingsDialog(const QSizeF &newPageSize, const QColor &newBackgroundColor, const MrDoc::Page::backgroundType newBackgroundType, QWidget *parent) : QDialog(parent)
{
  backgroundColor = newBackgroundColor;
  m_backgroundType = newBackgroundType;

  standardPaperSizesComboBox = new QComboBox(this);

  myStandardPageSizes << QPageSize(QPageSize::A0).sizePoints() << QPageSize(QPageSize::A1).sizePoints() << QPageSize(QPageSize::A2).sizePoints()
                      << QPageSize(QPageSize::A3).sizePoints() << QPageSize(QPageSize::A4).sizePoints() << QPageSize(QPageSize::A5).sizePoints()
                      << QPageSize(QPageSize::A6).sizePoints() << QPageSize(QPageSize::Letter).sizePoints();
  myStandardPageSizeNames << QPageSize(QPageSize::A0).name() << QPageSize(QPageSize::A1).name() << QPageSize(QPageSize::A2).name()
                          << QPageSize(QPageSize::A3).name() << QPageSize(QPageSize::A4).name() << QPageSize(QPageSize::A5).name()
                          << QPageSize(QPageSize::A6).name() << QPageSize(QPageSize::Letter).name();
  int I = myStandardPageSizes.size();
  for (int i = 0; i < I; ++i)
  {
    QSizeF tmpSize = myStandardPageSizes[i];
    tmpSize = QSizeF(tmpSize.height(), tmpSize.width());
    QString tmpName = myStandardPageSizeNames[i];
    tmpName.prepend("landscape ");
    myStandardPageSizes.append(tmpSize);
    myStandardPageSizeNames.append(tmpName);
  }

  myStandardPageSizes.append(QSize(1, 1));
  myStandardPageSizeNames.append(tr("Custom"));

  int index;
  for (index = 0; index < myStandardPageSizes.size(); ++index)
  {
    standardPaperSizesComboBox->addItem(myStandardPageSizeNames[index], index);
  }
  connect(standardPaperSizesComboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(standardPaperSizesComboChanged()));

  QDoubleValidator *validator = new QDoubleValidator;
  validator->setBottom(1.0);

  widthLineEdit = new QLineEdit(this);
  widthLineEdit->setValidator(validator);
  connect(widthLineEdit, SIGNAL(textChanged(QString)), this, SLOT(textChanged()));

  heightLineEdit = new QLineEdit(this);
  widthLineEdit->setValidator(validator);
  connect(heightLineEdit, SIGNAL(textChanged(QString)), this, SLOT(textChanged()));

  swapWidthHeightButton = new QPushButton(tr("Swap width and height"), this);
  connect(swapWidthHeightButton, SIGNAL(clicked()), this, SLOT(swapWidthHeight()));

  colorButton = new ColorButton();
  colorButton->setColor(backgroundColor);
  connect(colorButton, SIGNAL(clicked()), this, SLOT(chooseBackgroundColor()));

  backgroundTypesComboBox = new QComboBox(this);

  backgroundTypes << MrDoc::Page::backgroundType::PLAIN << MrDoc::Page::backgroundType::SQUARED << MrDoc::Page::backgroundType::RULED;
  backgroundTypeNames << tr("Plain paper") << tr("Squared paper") << tr("Ruled paper");

  //backgroundTypes.insert(std::pair<MrDoc::Page::backgroundType, QString>(MrDoc::Page::backgroundType::PLAIN, tr("Plain paper")));
  //backgroundTypes.insert(std::pair<MrDoc::Page::backgroundType, QString>(MrDoc::Page::backgroundType::SQUARED, tr("Squared paper")));
  //backgroundTypes.insert(std::pair<MrDoc::Page::backgroundType, QString>(MrDoc::Page::backgroundType::RULED, tr("Ruled paper")));

  for(int i = 0; i < backgroundTypes.size(); ++i){
      backgroundTypesComboBox->addItem(backgroundTypeNames[i], i);
  }

  backgroundTypesComboBox->setCurrentIndex(0);
  for(int i = 0; i < backgroundTypes.size(); ++i){
      if(backgroundTypes.at(i) == m_backgroundType)
          backgroundTypesComboBox->setCurrentIndex(i);
  }

  connect(backgroundTypesComboBox, qOverload<int>(&QComboBox::currentIndexChanged), this, [this](int index){ m_backgroundType = backgroundTypes.at(index); } );

  okButton = new QPushButton(tr("OK"), this);
  okButton->setDefault(true);
  cancelButton = new QPushButton(tr("Cancel"), this);

  scaleContentCheckBox = new QCheckBox(tr("Scale Content"), this);
  scaleContentCheckBox->setChecked(false);

  applyToAllCheckBox = new QCheckBox(tr("Apply to all pages"), this);
  applyToAllCheckBox->setChecked(false);

  QFormLayout *formLayout = new QFormLayout;
  formLayout->addRow(tr("Standard paper sizes:"), standardPaperSizesComboBox);
  formLayout->addRow(tr("&Width (pt):"), widthLineEdit);
  formLayout->addRow(tr("&Height (pt):"), heightLineEdit);
  formLayout->addRow(tr("Orientation:"), swapWidthHeightButton);
  formLayout->addRow(tr("Background Color:"), colorButton);
  formLayout->addRow(tr("Background Type:"), backgroundTypesComboBox);
  formLayout->addWidget(scaleContentCheckBox);
  formLayout->addWidget(applyToAllCheckBox);
  formLayout->addRow(okButton, cancelButton);

  setLayout(formLayout);

  setWindowTitle(tr("Page Settings"));

  widthLineEdit->setText(QString::number(newPageSize.width()));
  heightLineEdit->setText(QString::number(newPageSize.height()));

  connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
  connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));
}

void PageSettingsDialog::textChanged()
{
  qreal width = widthLineEdit->text().toDouble();
  qreal height = heightLineEdit->text().toDouble();
  currentPageSize = QSizeF(width, height);

  bool foundPageSize = false;
  int index = 0;
  for (index = 0; index < myStandardPageSizes.size(); ++index)
  {
    if (width == myStandardPageSizes[index].width() && height == myStandardPageSizes[index].height())
    {
      foundPageSize = true;
      break;
    }
  }
  standardPaperSizesComboBox->blockSignals(true);
  if (foundPageSize)
  {
    standardPaperSizesComboBox->setCurrentIndex(index);
  }
  else
  {
    standardPaperSizesComboBox->setCurrentIndex(standardPaperSizesComboBox->count() - 1);
  }
  standardPaperSizesComboBox->blockSignals(false);
}

void PageSettingsDialog::standardPaperSizesComboChanged()
{
  if (standardPaperSizesComboBox->currentIndex() != myStandardPageSizes.size() - 1)
  {
    widthLineEdit->blockSignals(true);
    heightLineEdit->blockSignals(true);
    QSizeF tmpPageSize = myStandardPageSizes[standardPaperSizesComboBox->currentIndex()];
    widthLineEdit->setText(QString::number(tmpPageSize.width()));
    heightLineEdit->setText(QString::number(tmpPageSize.height()));
    widthLineEdit->blockSignals(false);
    heightLineEdit->blockSignals(false);

    currentPageSize = tmpPageSize;
  }
}

void PageSettingsDialog::chooseBackgroundColor()
{
  QColorDialog colorDialog;
  QColor newBackgroundColor = colorDialog.getColor(backgroundColor);
  if (newBackgroundColor.isValid())
  {
    backgroundColor = newBackgroundColor;
    colorButton->setColor(backgroundColor);
  }
}

void PageSettingsDialog::swapWidthHeight()
{
  QString width = widthLineEdit->text();
  QString height = heightLineEdit->text();

  widthLineEdit->blockSignals(true);
  //    heightLineEdit->blockSignals(true);

  widthLineEdit->setText(height);
  heightLineEdit->setText(width);

  widthLineEdit->blockSignals(false);
  //    heightLineEdit->blockSignals(false);
}
