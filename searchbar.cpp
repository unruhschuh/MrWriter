#include "searchbar.h"
#include "ui_searchbar.h"

SearchBar::SearchBar(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SearchBar)
{
    ui->setupUi(this);
    connect(ui->lineEdit, &QLineEdit::returnPressed, this, &SearchBar::onReturnPressed);
    connect(ui->lineEdit, &QLineEdit::textChanged, this, &SearchBar::onTextChanged);
    connect(ui->searchDownButton, &QPushButton::clicked, this, &SearchBar::onNextPressed);
    connect(ui->searchUpButton, &QPushButton::clicked, this, &SearchBar::onPrevPressed);
}

SearchBar::~SearchBar()
{
    delete ui;
}

void SearchBar::onTextChanged(const QString& text){
    if(text.isEmpty()){
        emit clearSearch();
    }
}

void SearchBar::onReturnPressed(){
    if(ui->lineEdit->text().isEmpty()){
        emit clearSearch();
    }
    else{
        emit searchNext(ui->lineEdit->text());
    }
}

void SearchBar::onNextPressed(){
    if(ui->lineEdit->text().isEmpty()){
        emit clearSearch();
    }
    else{
        emit searchNext(ui->lineEdit->text());
    }
}

void SearchBar::onPrevPressed(){
    if(ui->lineEdit->text().isEmpty()){
        emit clearSearch();
    }
    else{
        emit searchPrev(ui->lineEdit->text());
    }
}
