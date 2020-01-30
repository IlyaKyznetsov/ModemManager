#ifndef CONNECTIONMANAGER_H
#define CONNECTIONMANAGER_H

#include "types.h"
#include <QObject>

class OfonoConnectionManagerInterface;

class ConnectionManager : public QObject
{
  Q_OBJECT
public:
  explicit ConnectionManager(QObject *parent = nullptr);
  bool isValid() const;
  void reset(const QString &path = QString());

Q_SIGNALS:
  void StateChanged(const State &state);

private:
  OfonoConnectionManagerInterface *_interface;
};

#endif // CONNECTIONMANAGER_H
