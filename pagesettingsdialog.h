/*
#####################################################################
Copyright (C) 2015 Thomas Leitz (thomas.leitz@web.de)
#####################################################################

LICENSE:

This file is part of MrWriter.

MrWriter is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License 2.0 as published
by the Free Software Foundation.

MrWriter is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with MrWriter.  If not, see <http://www.gnu.org/licenses/>.
#####################################################################
*/

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
