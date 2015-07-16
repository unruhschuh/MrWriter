#ifndef PAGESETTINGSDIALOG_H
#define PAGESETTINGSDIALOG_H

#include <QDialog>
#include <QComboBox>
#include <QLineEdit>
#include <QPushButton>

#include <QString>
#include <QPageSize>

class PageSettingsDialog : public QDialog
{
    Q_OBJECT
public:
    explicit PageSettingsDialog(const QPageSize& newPageSize, QWidget *parent = 0);
    QPageSize currentPageSize;

signals:

public slots:
    void textChanged();
    void standardPaperSizesComboChanged();

private:
    QComboBox* standardPaperSizesComboBox;
    QLineEdit* widthLineEdit;
    QLineEdit* heightLineEdit;
    QPushButton* okButton;
    QPushButton* cancelButton;
};

#endif // PAGESETTINGSDIALOG_H
