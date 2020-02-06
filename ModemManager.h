#ifndef MODEMMANAGER_H
#define MODEMMANAGER_H

#include "DeferredCall.h"
#include "ModemManagerData.h"
#include "types.h"
#include <QObject>

class OfonoManager;
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
  void onDeferredCall(State::Type type, const QVariant &value);

private:
  const QVector<State> _autoStates;
  QVector<State>::const_iterator _autoStateIterator;
  QVector<State>::const_iterator _modemStateIterator;
  QVector<State>::const_iterator _simManagerStateIterator;
  QVector<State>::const_iterator _networkRegistrationStateIterator;
  QVector<State>::const_iterator _connectionManagerStateIterator;
  QVector<State>::const_iterator _connectionContextStateIterator;

  const ModemManagerData::Settings _settings;
  OfonoManager *_ofonoManager;
  Manager *_manager;
  Modem *_modem;
  SimManager *_simManager;
  NetworkRegistration *_networkRegistration;
  ConnectionManager *_connectionManager;
  ConnectionContext *_connectionContext;
  //  DeferredCall *_deferredCall;
  ModemManagerData::OfonoState _ofonoState;

  void _signalOfonoManager(const State &state);
  void _signalManager(const State &state);
  void _signalModem(const State &state);
  void _signalSimManager(const State &state);
  void _signalNetworkRegistration(const State &state);
  void _signalConnectionManager(const State &state);
  void _signalConnectionContext(const State &state);

  void _autoStateChangedHandler(const State &state);

private Q_SLOTS:
  void debugOfonoState(const ModemManagerData::OfonoState &state);

public Q_SLOTS:
  void t_modemPowered(bool value);
  void t_modemOnline(bool value);
  void t_modemLockdown(bool value);
  void t_contextSetAPN(QString value);
  void t_contextSetUsername(QString value);
  void t_contextSetPassword(QString value);
  void t_contextSetActive(bool value);
};

#endif // MODEMMANAGER_H
