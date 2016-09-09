#include "pensettingsdialog.h"

#include <QFormLayout>
#include <QPageSize>
#include <QColorDialog>
#include <QDebug>
#include <QVector>

PenSettingsDialog::PenSettingsDialog(QWidget *parent) : QDialog(parent)
{
    okButton = new QPushButton(tr("OK"), this);
    okButton->setDefault(true);
    cancelButton = new QPushButton(tr("Cancel"), this);

    QFormLayout *formLayout = new QFormLayout;
    formLayout->addRow(okButton, cancelButton);

    setLayout(formLayout);

    setWindowTitle(tr("Pen Settings"));

    connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
    connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));
}


