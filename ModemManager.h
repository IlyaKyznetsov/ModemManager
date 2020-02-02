#ifndef MODEMMANAGER_H
#define MODEMMANAGER_H

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
  struct OfonoState
  {
    struct Modem
    {
      bool powered = false;
      bool online = false;
      bool lockdown = false;
      QString manufacturer; // readonly, optional
      QString model;        // readonly, optional
      QString serial;       // readonly, optional
    };
    struct SimManager
    {
      QString cardIdentifier;      // readonly
      QString serviceProviderName; // readonly, optional
    };
    struct NetworkRegistration
    {
      QString status;
      QString name;     //[readonly];
      QString strength; //[readonly, optional between 0-100 percent]
    };
    struct ConnectionManager
    {
      bool attached = false;       // [readonly]
      bool roamingAllowed = false; // [readwrite]
      bool powered = false;        // [readwrite]
      // QString connectionContextPath;
    };
    struct ConnectionContext
    {
      bool active = false; // [readwrite]
      QString accessPointName;
      QString username;
      QString password;
      QString type;
      QString authenticationMethod;
      QString protocol;
      QString name;
      QString interface; // [readonly, optional]
      QString method;    // [readonly, optional]
      QString address;   // [readonly, optional]
      QString netmask;   // [readonly, optional]
    };
    bool isOfonoConnected = false;
    QSharedPointer<Modem> modem;
    QSharedPointer<SimManager> simManager;
    QSharedPointer<NetworkRegistration> networkRegistration;
    QSharedPointer<ConnectionManager> connectionManager;
    QSharedPointer<ConnectionContext> connectionContext;
  };
  explicit ModemManager(QObject *parent = nullptr);

Q_SIGNALS:
  void OfonoStateChanged(const OfonoState &state);

private Q_SLOTS:
  void onStateChanged(const State &state);

private:
  OfonoManager *_ofonoManager;
  Manager *_manager;
  Modem *_modem;
  SimManager *_simManager;
  NetworkRegistration *_networkRegistration;
  ConnectionManager *_connectionManager;
  ConnectionContext *_connectionContext;

  void _signalOfonoManager(const State &state);
  void _signalManager(const State &state);
  void _signalModem(const State &state);
  void _signalSimManager(const State &state);
  void _signalNetworkRegistration(const State &state);
  void _signalConnectionManager(const State &state);
  void _signalConnectionContext(const State &state);

  State::Type _currentStateType;
  OfonoState _ofonoState;

private Q_SLOTS:
  void debugOfonoState(const OfonoState &state);

public Q_SLOTS:
  void t_modemPowered(bool value);
  void t_modemOnline(bool value);
  void t_modemLockdown(bool value);
  void t_contextSetAPN(QString value);
  void t_contextSetUsername(QString& value);
  void t_contextSetPassword(QString& value);
};

#include <qmetatype.h>
Q_DECLARE_METATYPE(ModemManager::OfonoState)

#endif // MODEMMANAGER_H
