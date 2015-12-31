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
//        Document& operator=(const Document& doc) {};
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
