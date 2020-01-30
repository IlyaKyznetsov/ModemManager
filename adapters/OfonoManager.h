#ifndef OFONOMANAGER_H
#define OFONOMANAGER_H

#include "types.h"
#include <QObject>

class QDBusServiceWatcher;

class OfonoManager : public QObject
{
  Q_OBJECT
public:
  explicit OfonoManager(QObject *parent = nullptr);
  void reset(const QString &service = QString());

Q_SIGNALS:
  void StateChanged(State state);

private:
  QDBusServiceWatcher *_watcher;
};

#endif // OFONOMANAGER_H
