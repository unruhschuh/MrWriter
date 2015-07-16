#include "pagesettingsdialog.h"

#include <QFormLayout>

const QVector<QString> PageSettingsDialog::standardPaperNames = { "A0", "A1", "A2", "A3", "A4", "A5"};
const QVector<QSizeF>  PageSettingsDialog::standardPaperSizes = { QSizeF(2384, 3371),
                                                                  QSizeF(1685, 2384),
                                                                  QSizeF(1190, 1684),
                                                                  QSizeF(842, 1190),
                                                                  QSizeF(595, 842),
                                                                  QSizeF(420, 595) };


PageSettingsDialog::PageSettingsDialog(QWidget *parent) : QDialog(parent)
{
    standardPaperSizesComboBox = new QComboBox(this);
    for (int i = 0; i < standardPaperNames.size(); ++i)
    {
        standardPaperSizesComboBox->addItem(standardPaperNames.at(i), standardPaperSizes.at(i));
    }

    widthLineEdit = new QLineEdit(this);
    heightLineEdit = new QLineEdit(this);

    okButton = new QPushButton(tr("OK"), this);
    okButton->setDefault(true);
    cancelButton = new QPushButton(tr("Cancel"), this);

    QFormLayout *formLayout = new QFormLayout;
    formLayout->addRow(tr("Standard paper sizes:"), standardPaperSizesComboBox);
    formLayout->addRow(tr("&Width:"), widthLineEdit);
    formLayout->addRow(tr("&Height:"), heightLineEdit);
    formLayout->addRow(okButton, cancelButton);

    setLayout(formLayout);

    setWindowTitle(tr("Page Settings"));
    setModal(true);

    connect(cancelButton, SIGNAL(clicked()), this, SLOT(close()));
}

