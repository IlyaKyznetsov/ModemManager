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
class Automator;

class ModemManager : public QObject
{
  Q_OBJECT
public:
  explicit ModemManager(const ModemManagerData::Settings &settings, QObject *parent = nullptr);

Q_SIGNALS:
  void OfonoStateChanged(const ModemManagerData::OfonoState &state);
  void StateChanged(const State &state);

public Q_SLOTS:
  void call(const State::Type callType, const QVariant &value);

private Q_SLOTS:
  void onStateChanged(const State &state);

private:
  const ModemManagerData::Settings _settings;
  QDBusServiceWatcher *_watcher;
  Automator *_automator;
  Manager *_manager;
  Modem *_modem;
  SimManager *_simManager;
  NetworkRegistration *_networkRegistration;
  ConnectionManager *_connectionManager;
  ConnectionContext *_connectionContext;
  ModemManagerData::OfonoState _ofonoState;

  inline void _signalManager(const State &state);
  inline void _signalModem(const State &state);
  inline void _signalSimManager(const State &state);
  inline void _signalNetworkRegistration(const State &state);
  inline void _signalConnectionManager(const State &state);
  inline void _signalConnectionContext(const State &state);

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
