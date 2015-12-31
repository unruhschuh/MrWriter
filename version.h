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

// idea from http://stackoverflow.com/a/13341571/1580081

#ifndef VERSION_H
#define VERSION_H

#define PRODUCT_NAME "MrWriter"
#define PRODUCT_URL "http://www.unruhschuh.com/mrwriter"

#define MAJOR_VERSION 0
#define MINOR_VERSION 1

#define DOC_VERSION 1

#define MY_DOC_VERSION 0

// idea from https://forum.qt.io/topic/41021/solved-how-to-generate-build-number/2
#define BUILD QString("%1%2").arg(QLocale(QLocale::C).toDate(QString(__DATE__).simplified(), QLatin1String("MMM d yyyy")).toString("yyyyMMdd")).arg(QString("%1%2%3%4%5%6").arg(__TIME__[0]).arg(__TIME__[1]).arg(__TIME__[3]).arg(__TIME__[4]).arg(__TIME__[6]).arg(__TIME__[7]))

#endif // VERSION_H

