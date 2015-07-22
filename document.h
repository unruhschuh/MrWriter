#ifndef DOCUMENT_H
#define DOCUMENT_H

#include "page.h"

#include <QVector>

class Document
{
public:
    Document();
    Document(const Document& doc);
    Document& operator=(const Document& doc) {};
    void exportPDF(QString fileName);

    bool loadXOJ(QString fileName);
    bool saveXOJ(QString fileName);

    bool loadMOJ(QString fileName);
    bool saveMOJ(QString fileName);

    void paintPage(int pageNum, QPainter &painter, qreal zoom);

    bool setDocName(QString newDocName);
    QString getDocName();

    bool setPath(QString newDocName);
    QString getPath();

    bool getDocumentChanged();
    void setDocumentChanged(bool changed);

    QVector<Page> pages;

    const static QColor black;
    const static QColor blue;
    const static QColor red;
    const static QColor green;
    const static QColor gray;
    const static QColor lightblue;
    const static QColor lightgreen;
    const static QColor magenta;
    const static QColor orange;
    const static QColor yellow;
    const static QColor white;

    const static QVector<QString> standardColorNames; // = QVector<QString>() << "black" << "blue" << "red" << "green" << "gray" << "lightblue" << "lightgreen" << "magenta" << "orange" << "yellow" << "white";
    const static QVector<QColor> standardColors; // = QVector<QColor>() << black << blue << red << green << gray << lightblue << lightgreen << magenta << orange << yellow << white;

    QString toRGBA(QString argb);
    QString toARGB(QString rgba);

    QColor stringToColor(QString colorString);

private:
    bool documentChanged;

    QString docName;
    QString path;
};

#endif // DOCUMENT_H
