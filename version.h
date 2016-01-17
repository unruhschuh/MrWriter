// idea from http://stackoverflow.com/a/13341571/1580081

#ifndef VERSION_H
#define VERSION_H

#define PRODUCT_NAME "MrWriter"
#define PRODUCT_URL "http://github.com/unruhschuh/MrWriter"

#define MAJOR_VERSION 0
#define MINOR_VERSION 0
#define PATCH_VERSION 3

#define DOC_VERSION 0

// idea from https://forum.qt.io/topic/41021/solved-how-to-generate-build-number/2
#define BUILD                                                                                                                                                  \
  QString("%1%2")                                                                                                                                              \
      .arg(QLocale(QLocale::C).toDate(QString(__DATE__).simplified(), QLatin1String("MMM d yyyy")).toString("yyyyMMdd"))                                       \
      .arg(QString("%1%2%3%4%5%6").arg(__TIME__[0]).arg(__TIME__[1]).arg(__TIME__[3]).arg(__TIME__[4]).arg(__TIME__[6]).arg(__TIME__[7]))

#endif // VERSION_H
