#ifndef PAGESETTINGSDIALOG_H
#define PAGESETTINGSDIALOG_H

#include <QDialog>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>

#include <QVector>
#include <QString>
#include <QSizeF>

class PageSettingsDialog : public QDialog
{
    Q_OBJECT
public:
    explicit PageSettingsDialog(QWidget *parent = 0);

signals:

public slots:

private:
    const static QVector<QString> standardPaperNames;
    const static QVector<QSizeF> standardPaperSizes;

    QComboBox* standardPaperSizesComboBox;
    QLineEdit* widthLineEdit;
    QLineEdit* heightLineEdit;
    QPushButton* okButton;
    QPushButton* cancelButton;
};

#endif // PAGESETTINGSDIALOG_H
