#ifndef AUTOMATOR_H
#define AUTOMATOR_H

#include "AutomatorScript.h"
#include "ModemManagerData.h"
#include <QObject>

class Automator : public QObject
{
  Q_OBJECT
public:
  explicit Automator(const ModemManagerData::Settings &settings, QObject *parent = nullptr);

public:
  void processing(QObject *sender, const State &state);
  void run(QObject *adapter, const State::Type type, const QVariant &value);

private Q_SLOTS:
  void onStatusChanged(const AutomatorScript::ScriptStatus status);

private:
  const ModemManagerData::Settings &_settings;
  AutomatorScript _managerModemRemoved;
  AutomatorScript _managerModemAdded;
  AutomatorScript _modemLockdownDisable;
  AutomatorScript _modemLockdownEnable;
  AutomatorScript _modemPoweredDisable;
  AutomatorScript _modemPoweredEnable;
  AutomatorScript _modemOnlineDisable;
  AutomatorScript _modemOnlineEnable;
  AutomatorScript _simManagerAdded;
  AutomatorScript _networkRegistrationAdded;
  AutomatorScript _connectionManagerAdded;
  AutomatorScript _connectionContextAdded;
  AutomatorScript::Data _data;
};

#endif // AUTOMATOR_H
