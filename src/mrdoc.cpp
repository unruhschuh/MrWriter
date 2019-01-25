#include "mrdoc.h"

namespace MrDoc
{

QString toARGB(QString rgba)
{
  // #RRGGBBAA
  // 012345678
  QString argb;
  if (rgba.length() == 9)
  {
    argb.append('#');
    argb.append(rgba.mid(7, 2));
    argb.append(rgba.mid(1, 6));
  }
  else
  {
    argb = QString("");
  }

  return argb;
}

QString toRGBA(QString argb)
{
  // #AARRGGBB
  // 012345678
  QString rgba;
  if (argb.length() == 9)
  {
    rgba.append('#');
    rgba.append(argb.mid(3, 6));
    rgba.append(argb.mid(1, 2));
  }
  else
  {
    rgba = QString("");
  }

  return rgba;
}

QColor stringToColor(QString colorString)
{
  QColor color;
  if (colorString.left(1).compare("#") == 0)
  {
    color = QColor(MrDoc::toARGB(colorString));
  }
  else
  {
    for (int i = 0; i < standardColors.size(); ++i)
    {
      if (standardColorNames[i].compare(colorString) == 0)
      {
        color = standardColors.at(i);
      }
    }
  }
  return color;
}

}
