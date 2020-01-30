#ifndef SIMMANAGER_H
#define SIMMANAGER_H

#include "types.h"
#include <QObject>

class OfonoSimManagerInterface;

class SimManager : public QObject
{
  Q_OBJECT
public:
  explicit SimManager(QObject *parent = nullptr);
  bool isValid() const;
  void reset(const QString &path = QString());

Q_SIGNALS:
  void StateChanged(const State &state);

private:
  OfonoSimManagerInterface *_interface;
};

#endif // SIMMANAGER_H
