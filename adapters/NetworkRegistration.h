#ifndef NETWORKREGISTRATION_H
#define NETWORKREGISTRATION_H

#include "types.h"
#include <QObject>

class OfonoNetworkRegistrationInterface;

class NetworkRegistration : public QObject
{
  Q_OBJECT
public:
  explicit NetworkRegistration(QObject *parent = nullptr);
  bool isValid() const;
  void reset(const QString &path = QString());
  void call(const State::Type type);

Q_SIGNALS:
  void StateChanged(const State &state);

private:
  OfonoNetworkRegistrationInterface *_interface;
  State::Type _currentCallType;
};

#endif // NETWORKREGISTRATION_H
