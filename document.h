#ifndef DOCUMENT_H
#define DOCUMENT_H

#include "page.h"

#include <QVector>

namespace MrDoc {

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

        QVector<MrDoc::Page> pages;

        QString toRGBA(QString argb);
        QString toARGB(QString rgba);

        QColor stringToColor(QString colorString);

    private:
        bool documentChanged;

        QString docName;
        QString path;
    };


}

#endif // DOCUMENT_H
