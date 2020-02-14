#ifndef MODEM_H
#define MODEM_H

#include "types.h"
#include <QObject>

class OfonoModemInterface;

class Modem : public QObject
{
  Q_OBJECT
public:
  explicit Modem(const int &dbusTimeout, QObject *parent = nullptr);
  bool isValid() const;
  QString path() const;
  void reset(const QString &path = QString());
  void call(const State::Type type, const QVariant &value);

Q_SIGNALS:
  void StateChanged(const State &state);

private:
  const int _dbusTimeout;
  OfonoModemInterface *_interface;
  QStringList _modemInterfaces;
  QList<State::Type> _modemInterfacesChanged(const QStringList &interfaces);
  State::Type _currentCallType;
  QVariant _currentCallValue;
};

#endif // MODEM_H
