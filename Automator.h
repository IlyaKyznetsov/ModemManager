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
  void automatorScriptStatusChanged(const State::Status status, const State &state);

private:
  const ModemManagerData::Settings &_settings;
  QScopedPointer<QTimer> _networkRegistrationTimer;
  void debugScriptsRunning();
  AutomatorScript _managerModemAdded;
  //
  AutomatorScript _modemLockdownDisable;
  AutomatorScript _modemLockdownEnable;
  //
  AutomatorScript _modemPoweredDisable;
  AutomatorScript _modemPoweredEnable;
  //
  AutomatorScript _modemOnlineDisable;
  AutomatorScript _modemOnlineEnable;
  //
  AutomatorScript _simManagerAdded;
  //
  AutomatorScript _networkRegistrationAdded;
  //
  AutomatorScript _connectionManagerAdded;
  //
  AutomatorScript _connectionContextAdded;
  AutomatorScript _connectionContextAccessPointName;
  AutomatorScript _connectionContextUsername;
  AutomatorScript _connectionContextPassword;
  AutomatorScript _connectionContextActiveDisable;
  AutomatorScript _connectionContextActiveEnable;

  AutomatorScript::Data _data;
};

#endif // AUTOMATOR_H
