#ifndef PENSETTINGSDIALOG_H
#define PENSETTINGSDIALOG_H

#include <QDialog>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>
#include <QCheckBox>

#include <QString>
#include <QPageSize>
#include <QColor>

#include "colorbutton.h"

class PenSettingsDialog : public QDialog
{
  Q_OBJECT
public:
  explicit PenSettingsDialog(QWidget *parent = 0);

private:
  QPushButton *okButton;
  QPushButton *cancelButton;
};

#endif // PENSETTINGSDIALOG_H
