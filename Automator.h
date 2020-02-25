#ifndef AUTOMATOR_H
#define AUTOMATOR_H

#include "ModemManagerData.h"
#include "types.h"
#include <QtCore/QObject>

class Automator : public QObject
{
  Q_OBJECT
  Q_DISABLE_COPY_MOVE(Automator)
public:
  Automator(QObject *parent);
  void reset();
  void processing(const ModemManagerData::Settings &settings, const State &state,
                  const ModemManagerData::OfonoState &ofonoState);
  void errorHandler(const State &state, const ModemManagerData::OfonoState &ofonoState);
Q_SIGNALS:
  void call(const State::Type callType, const QVariant &value);

private:
  void debug(const State &state, const ModemManagerData::OfonoState &ofonoState) const;
  bool isTimeoutError(const QDBusError &error);
  bool modemAdded;
  bool connectionContextConfigured;
  int connectionContextCount;
  State::Type requestCall;
  QVariant requestValue;
  State::Status modemLockdownStatus;
  State::Status modemPoweredStatus;
  State::Status modemOnlineStatus;
  State::Status connectionManagerPoweredStatus;
  State::Status connectionContextAccessPointNameStatus;
  State::Status connectionContextUsernameStatus;
  State::Status connectionContextPasswordStatus;
  State::Status connectionContextActiveStatus;
};

#endif // AUTOMATOR_H
