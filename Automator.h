#ifndef AUTOMATOR_H
#define AUTOMATOR_H

#include "AutomatorScript.h"
#include "ModemManagerData.h"
#include <QObject>
#include <QTimer>

class Automator : public QObject
{
  Q_OBJECT
public:
  explicit Automator(const ModemManagerData::Settings &settings, QObject *parent = nullptr);
  ~Automator();

public:
  void processing(QObject *sender, const State &state);

Q_SIGNALS:
  void Call(const State::Type callType, const QVariant &value);

private Q_SLOTS:
  void automatorScriptStatusChanged(const State::Status status, const QDBusError &error);

private:
  const ModemManagerData::Settings &_settings;
  QScopedPointer<QTimer> _networkRegistrationTimer;
  void debugScriptsRunning();
  AutomatorScript _managerModemRemoved;
  AutomatorScript _managerModemAdded;
  //
  AutomatorScript _modemLockdownSignal;
  AutomatorScript _modemLockdownDisable;
  AutomatorScript _modemLockdownEnable;
  //
  AutomatorScript _modemPoweredSignal;
  AutomatorScript _modemPoweredDisable;
  AutomatorScript _modemPoweredEnable;
  //
  AutomatorScript _modemOnlineSignal;
  AutomatorScript _modemOnlineDisable;
  AutomatorScript _modemOnlineEnable;
  //
  AutomatorScript _simManagerRemoved;
  AutomatorScript _simManagerAdded;
  //
  AutomatorScript _networkRegistrationRemoved;
  AutomatorScript _networkRegistrationAdded;
  //
  AutomatorScript _connectionManagerRemoved;
  AutomatorScript _connectionManagerAdded;
  //
  AutomatorScript _connectionContextRemoved;
  AutomatorScript _connectionContextAdded;
  //
  AutomatorScript _connectionContextAccessPointNameSignal;
  AutomatorScript _connectionContextAccessPointName;
  AutomatorScript _connectionContextUsernameSignal;
  AutomatorScript _connectionContextUsername;
  AutomatorScript _connectionContextPasswordSignal;
  AutomatorScript _connectionContextPassword;
  AutomatorScript _connectionContextActiveSignal;
  AutomatorScript _connectionContextActiveDisable;
  AutomatorScript _connectionContextActiveEnable;
  void resetConnectionContext();
  void resetConnectionManager();
  void resetNetworkRegistration();
  void resetSimManager();
  void resetModem();
  void resetManager();
  void errorHandler(const State &state);
  //  Добавить функции сброса _scriptsRunningCount и установку скриптов в начало

  AutomatorScript::Data _data;
};

#endif // AUTOMATOR_H
