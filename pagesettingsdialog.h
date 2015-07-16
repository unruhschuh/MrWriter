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
    explicit PageSettingsDialog(const QSizeF& newPageSize, const QColor& newBackgroundColor, QWidget *parent = 0);
    QSizeF currentPageSize;
    QColor backgroundColor;

signals:

public slots:
    void textChanged();
    void standardPaperSizesComboChanged();
    void chooseBackgroundColor();
    void swapWidthHeight();

private:
    QVector<QSizeF> myStandardPageSizes;
    QVector<QString> myStandardPageSizeNames;

    QComboBox* standardPaperSizesComboBox;
    QLineEdit* widthLineEdit;
    QLineEdit* heightLineEdit;
    QPushButton* swapWidthHeightButton;
    ColorButton* colorButton;
    QCheckBox* scaleContentCheckBox;
    QCheckBox* applyToAllCheckBox;

    QPushButton* okButton;
    QPushButton* cancelButton;
};

#endif // PAGESETTINGSDIALOG_H
