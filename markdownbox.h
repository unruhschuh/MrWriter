#ifndef MARKDOWNBOX_H
#define MARKDOWNBOX_H
#include <QTextEdit>
#include "page.h"
#include "abstracttextbox.h"

class MarkdownBox : public AbstractTextBox {
    Q_OBJECT
public:
    MarkdownBox(QWidget* parent = nullptr);

signals:
    void updatePageMarkdown(int pageNum);

public slots:
    virtual void applyText() override;
};

#endif // MARKDOWNBOX_H
