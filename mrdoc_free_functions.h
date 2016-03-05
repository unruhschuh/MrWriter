#ifndef MRDOC_FREE_FUNCTIONS_H
#define MRDOC_FREE_FUNCTIONS_H

#include <stroke.h>

namespace MrDoc
{
QVector<MrDoc::Stroke> splitStroke(MrDoc::Stroke const &origStroke, QLineF line, QLineF::IntersectType intersectType)
{
  QVector<MrDoc::Stroke> strokes;

  QPointF intersectionPoint;

  QPolygonF origPoints         = origStroke.m_points;
  QVector<qreal> origPressures = origStroke.m_pressures;

  for (int i = 0; i < origPoints.size() - 1; ++i)
  {
    QLineF pointLine = QLineF(origPoints.at(i), origPoints.at(i + 1));
    if (line.intersect(pointLine, &intersectionPoint) == intersectType //
        && intersectionPoint != origPoints.first()                     //
        && intersectionPoint != origPoints.last()                      //
        )
    {
      QPolygonF points;
      points = origPoints.mid(0, i + 1);
      points.append(intersectionPoint);

      QVector<qreal> pressures;
      pressures = origPressures.mid(0, i + 1);
      pressures.append(origPressures.at(i));

      MrDoc::Stroke stroke(origStroke.m_pattern, origStroke.m_penWidth, origStroke.m_color, points, pressures);
      strokes.append(stroke);

      std::remove(origPoints.begin(), origPoints.begin() + i + 1, origPoints);
      std::remove(origPressures.begin(), origPressures.begin() + i + 1, origPressures);

      i = 0;
    }
  }
  if (origPoints.size() > 0)
  {
    MrDoc::Stroke stroke(origStroke.m_pattern, origStroke.m_penWidth, origStroke.m_color, origPoints, origPressures);
    strokes.append(stroke);
  }

  return strokes;
}
}

#endif // MRDOC_FREE_FUNCTIONS_H
