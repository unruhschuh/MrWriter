#ifndef SEARCHBAR_H
#define SEARCHBAR_H

#include <QWidget>

namespace Ui {
class SearchBar;
}

class SearchBar : public QWidget
{
    Q_OBJECT

public:
    explicit SearchBar(QWidget *parent = 0);
    ~SearchBar();

public slots:
    void onReturnPressed();
    void onTextChanged(const QString& text);
    void onNextPressed();
    void onPrevPressed();

private:
    Ui::SearchBar *ui;

signals:
    void searchNext(const QString& text);
    void searchPrev(const QString& text);
    void clearSearch();
};

#endif // SEARCHBAR_H
