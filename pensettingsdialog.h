#ifndef PENSETTINGSDIALOG_H
#define PENSETTINGSDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QPixmap>
#include <QBitmap>
#include <QFormLayout>
#include <QPointer>

#include <QString>
#include <QColor>


#include "colorbutton.h"

class PenSettingsDialog : public QDialog
{
  Q_OBJECT
public:
  explicit PenSettingsDialog(QWidget *parent = 0);

private:
  QPointer<QPushButton> okButton;
  QPointer<QPushButton> cancelButton;

  QPointer<QFormLayout> formLayout;

  QPointer<QRadioButton> pencilStyleButton;
  QPointer<QRadioButton> dotStyleButton;

  QPointer<QLabel> cursorStyleLabel;
};

#endif // PENSETTINGSDIALOG_H
