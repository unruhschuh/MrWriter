#include "keypresseater.h"
#include <QDebug>

KeyPressEater::KeyPressEater(QObject *parent) : QObject(parent)
{

}

bool KeyPressEater::eventFilter(QObject *obj, QEvent *event)
{
  qDebug() << event->type();
  if (event->type() == QEvent::ShortcutOverride) {
      QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
      qDebug("Ate key press %d", keyEvent->key());
      return true;
  } else {
      // standard event processing
      return QObject::eventFilter(obj, event);
  }

}

