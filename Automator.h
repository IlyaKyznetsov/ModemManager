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

Q_SIGNALS:
  void Call(const State::Type callType, const QVariant &callValue);

public:
  void processing(const State &state);
  void reset();

private:
  enum ScriptType
  {
    _EMPTYTYPE_ = 0,
    ManagerModemAdded,
    ModemLockdownDisable,
    ModemLockdownEnable,
    ModemPoweredDisable,
    ModemPoweredEnable,
    ModemOnlineDisable,
    ModemOnlineEnable,
    SimManagerAdded,
    NetworkRegistrationAdded,
    ConnectionManagerAdded,
    ConnectionContextAdded,
    ConnectionContextAccessPointName,
    ConnectionContextUsername,
    ConnectionContextPassword,
    ConnectionContextActiveDisable,
    ConnectionContextActiveEnable
  };
  QString toString(ScriptType type)
  {
    QMap<ScriptType, QString> types{{_EMPTYTYPE_, "_EMPTYTYPE_"},
                                    {ManagerModemAdded, "ManagerModemAdded"},
                                    {ModemLockdownDisable, "ModemLockdownDisable"},
                                    {ModemLockdownEnable, "ModemLockdownEnable"},
                                    {ModemPoweredDisable, "ModemPoweredDisable"},
                                    {ModemPoweredEnable, "ModemPoweredEnable"},
                                    {ModemOnlineDisable, "ModemOnlineDisable"},
                                    {ModemOnlineEnable, "ModemOnlineEnable"},
                                    {SimManagerAdded, "SimManagerAdded"},
                                    {NetworkRegistrationAdded, "NetworkRegistrationAdded"},
                                    {ConnectionManagerAdded, "ConnectionManagerAdded"},
                                    {ConnectionContextAdded, "ConnectionContextAdded"},
                                    {ConnectionContextAccessPointName, "ConnectionContextAccessPointName"},
                                    {ConnectionContextUsername, "ConnectionContextUsername"},
                                    {ConnectionContextPassword, "ConnectionContextPassword"},
                                    {ConnectionContextActiveDisable, "ConnectionContextActiveDisable"},
                                    {ConnectionContextActiveEnable, "ConnectionContextActiveEnable"}};
    return types.value(type);
  }

  struct Data
  {
    bool restartModem = false;
    bool managerModemAdded = false;
    bool modemLockdown = false;
    bool modemPowered = false;
    bool modemOnline = false;
    bool simManagerAdded = false;
    bool networkRegistrationAdded = false;
    bool connectionManagerAdded = false;
    bool connectionManagerAttached = false;
    bool connectionManagerPowered = false;
    bool connectionContextAdded = false;
    bool connectionContextActive = false;
    QString simManagerCardIdentifier;
    QString simManagerServiceProviderName;
    QString networkRegistrationStatus;
    QString connectionContextAccessPointName;
    QString connectionContextUsername;
    QString connectionContextPassword;
    Data() = default;
    void debug();
  };

  const ModemManagerData::Settings &_settings;
  QScopedPointer<QTimer> _timer;
  const QMap<ScriptType, AutomatorScript *> _scripts;
  Data _data;
  void debugScriptsRunning();
};

#endif // AUTOMATOR_H
