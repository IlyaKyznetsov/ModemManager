#ifndef MANAGER_H
#define MANAGER_H

#include "types.h"
#include <QObject>

class OfonoManagerInterface;

class Manager : public QObject
{
  Q_OBJECT
public:
  explicit Manager(QObject *parent = nullptr);
  bool isValid() const;
  void reset(const QString &service = QString());

Q_SIGNALS:
  void StateChanged(State state);

private:
  OfonoManagerInterface *_interface;
  QStringList _modemsPath;
};

#endif // MANAGER_H
