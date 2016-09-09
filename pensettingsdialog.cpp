#include "pensettingsdialog.h"


#include <QDebug>
#include <QVector>

PenSettingsDialog::PenSettingsDialog(QWidget *parent) : QDialog(parent)
{
    okButton = new QPushButton(tr("OK"), this);
    okButton->setDefault(true);
    cancelButton = new QPushButton(tr("Cancel"), this);

    pencilStyleButton = new QRadioButton(tr("Pencil"), this);
    QPixmap penPixmap(":/images/penCursor3.png");
    QPixmap penPixmapMask(":/images/penCursor3Mask.png");
    penPixmap.setMask(QBitmap(penPixmapMask));
    pencilStyleButton->setIcon(QIcon(penPixmap));

    dotStyleButton = new QRadioButton(tr("Dot"), this);
    dotStyleButton->setIcon(QIcon(":/images/veryFinePenWidthIcon.png"));

    cursorStyleLabel = new QLabel(tr("Pick Cursor Style:"));

    formLayout = new QFormLayout;
    formLayout->addRow(cursorStyleLabel);
    formLayout->addRow(pencilStyleButton, dotStyleButton);
    formLayout->addRow(okButton, cancelButton);

    setLayout(formLayout);

    setWindowTitle(tr("Pen Settings"));

    connect(cancelButton, SIGNAL(clicked()), this, SLOT(reject()));
    connect(okButton, SIGNAL(clicked()), this, SLOT(accept()));
}


