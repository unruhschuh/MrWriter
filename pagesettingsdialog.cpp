#include "pagesettingsdialog.h"

#include <QFormLayout>
#include <QPageSize>
#include <QDebug>

PageSettingsDialog::PageSettingsDialog(const QPageSize& newPageSize, QWidget *parent) : QDialog(parent)
{
    standardPaperSizesComboBox = new QComboBox(this);

    for (int i = 0; i < QPageSize::LastPageSize; ++i)
    {
        QPageSize::PageSizeId pageSizeId = static_cast<QPageSize::PageSizeId>(i);
        standardPaperSizesComboBox->addItem(QPageSize(pageSizeId).name(), pageSizeId);
    }
    connect(standardPaperSizesComboBox, SIGNAL(currentIndexChanged(QString)), this, SLOT(standardPaperSizesComboChanged()));

    QDoubleValidator* validator = new QDoubleValidator;
    validator->setBottom(1.0);

    widthLineEdit = new QLineEdit(this);
    widthLineEdit->setValidator(validator);
    connect(widthLineEdit, SIGNAL(textChanged(QString)), this, SLOT(textChanged()));

    heightLineEdit = new QLineEdit(this);
    widthLineEdit->setValidator(validator);
    connect(heightLineEdit, SIGNAL(textChanged(QString)), this, SLOT(textChanged()));

    okButton = new QPushButton(tr("OK"), this);
    okButton->setDefault(true);
    cancelButton = new QPushButton(tr("Cancel"), this);

    QFormLayout *formLayout = new QFormLayout;
    formLayout->addRow(tr("Standard paper sizes:"), standardPaperSizesComboBox);
    formLayout->addRow(tr("&Width:"), widthLineEdit);
    formLayout->addRow(tr("&Height:"), heightLineEdit);
    formLayout->addRow
    formLayout->addRow(okButton, cancelButton);

    setLayout(formLayout);

    setWindowTitle(tr("Page Settings"));

    qDebug() << currentPageSize;
    widthLineEdit->setText(QString::number(newPageSize.size(QPageSize::Millimeter).width()));
    heightLineEdit->setText(QString::number(newPageSize.size(QPageSize::Millimeter).height()));

    connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
    connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));
}

void PageSettingsDialog::textChanged()
{
    qreal width = widthLineEdit->text().toDouble();
    qreal height = heightLineEdit->text().toDouble();
    currentPageSize = QPageSize(QSizeF(width, height), QPageSize::Millimeter);
    qDebug() << currentPageSize;

    int index = standardPaperSizesComboBox->findData(currentPageSize.id());
    standardPaperSizesComboBox->blockSignals(true);
    standardPaperSizesComboBox->setCurrentIndex(index);
    standardPaperSizesComboBox->blockSignals(false);
}

void PageSettingsDialog::standardPaperSizesComboChanged()
{
    widthLineEdit->blockSignals(true);
    heightLineEdit->blockSignals(true);
    QPageSize tmpPageSize = QPageSize(static_cast<QPageSize::PageSizeId>(standardPaperSizesComboBox->currentData().toInt()));
    widthLineEdit->setText(QString::number(tmpPageSize.size(QPageSize::Millimeter).width()));
    heightLineEdit->setText(QString::number(tmpPageSize.size(QPageSize::Millimeter).height()));
    widthLineEdit->blockSignals(false);
    heightLineEdit->blockSignals(false);

    currentPageSize = tmpPageSize;
}
