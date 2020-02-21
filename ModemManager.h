#ifndef MODEMMANAGER_H
#define MODEMMANAGER_H

#include "ModemManagerData.h"
#include "types.h"
#include <QObject>

class QDBusServiceWatcher;
class Manager;
class Modem;
class SimManager;
class NetworkRegistration;
class ConnectionManager;
class ConnectionContext;

class ModemManager : public QObject
{
  Q_OBJECT
public:
  explicit ModemManager(const ModemManagerData::Settings &settings, QObject *parent = nullptr);

Q_SIGNALS:
  void OfonoStateChanged(const ModemManagerData::OfonoState &state);
  void StateChanged(const State &state);

private Q_SLOTS:
  void onStateChanged(const State &state);
  void call(const State::Type callType, const QVariant &value);

private:
  const ModemManagerData::Settings _settings;
  QDBusServiceWatcher *_watcher;
  Manager *_manager;
  Modem *_modem;
  SimManager *_simManager;
  NetworkRegistration *_networkRegistration;
  ConnectionManager *_connectionManager;
  ConnectionContext *_connectionContext;
  ModemManagerData::OfonoState _ofonoState;
  struct AutomatorStates
  {
    bool modemInitialized;
    bool needPowerOff;
    bool networkRegistrationRegistered;
    bool connectionManagerAtached;
    bool connectionContextActive;

    State::Status modemLockdownStatus;
    State::Status modemPoweredStatus;
    State::Status modemOnlineStatus;
    State::Status connectionContextAccessPointNameStatus;
    State::Status connectionContextUsernameStatus;
    State::Status connectionContextPasswordStatus;
    State::Status connectionContextActiveStatus;
    AutomatorStates();
    void reset();
  } _automator;

  inline void _signalManager(const State &state);
  inline void _signalModem(const State &state);
  inline void _signalSimManager(const State &state);
  inline void _signalNetworkRegistration(const State &state);
  inline void _signalConnectionManager(const State &state);
  inline void _signalConnectionContext(const State &state);
  inline void _automatorProcessing(const State &state);
  bool isTimeoutError(const QDBusError &error);

private Q_SLOTS:
  void debugOfonoState(const ModemManagerData::OfonoState &state);

public Q_SLOTS:
  void t_modemPowered(bool value);
  void t_modemOnline(bool value);
  void t_modemLockdown(bool value);
  void t_networkRegistered();
  void t_networkUnregistered();
  void t_networkScan();
  void t_contextSetAPN(QString value);
  void t_contextSetUsername(QString value);
  void t_contextSetPassword(QString value);
  void t_contextSetActive(bool value);
};

#endif // MODEMMANAGER_H
