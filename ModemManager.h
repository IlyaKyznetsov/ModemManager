#ifndef MODEMMANAGER_H
#define MODEMMANAGER_H

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
class Automator;

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
  OfonoManager *_ofonoManager;
  Manager *_manager;
  Modem *_modem;
  SimManager *_simManager;
  NetworkRegistration *_networkRegistration;
  ConnectionManager *_connectionManager;
  ConnectionContext *_connectionContext;
  //  DeferredCall *_deferredCall;
  ModemManagerData::OfonoState _ofonoState;

  Automator *_automator;
  void _signalOfonoManager(const State &state);
  void _signalManager(const State &state);
  void _signalModem(const State &state);
  void _signalSimManager(const State &state);
  void _signalNetworkRegistration(const State &state);
  void _signalConnectionManager(const State &state);
  void _signalConnectionContext(const State &state);

  //  void _automationHandler(QObject *sender_ptr, const State &state);
  /*
    struct AutomatorData
    {
      QVariant modemLockdown;
      QVariant modemPowered;
      QVariant modemOnline;
      QVariant simManagerCardIdentifier;
      QVariant simManagerServiceProviderName;
      QVariant networkRegistrationStatus;
      QVariant connectionContextAccessPointName;
      QVariant connectionContextUsername;
      QVariant connectionContextPassword;
      QVariant connectionManagerAttached;
      QVariant connectionManagerPowered;
      QVariant connectionContextActive;

      ModemManagerData::Settings::Provider provider;
      AutomatorData() = default;
    };

    struct AutomatorItem
    {
      typedef QVector<ModemManager::AutomatorItem>::const_iterator Iterator;
      typedef void (*StateItemCommand)(Iterator &iterator, QObject *sender, const ModemManager::AutomatorData &data);
      State state;
      StateItemCommand command;
      AutomatorItem(
          const State &_state,
  #ifdef QT_DEBUG
          StateItemCommand _command =
              [](ModemManager::AutomatorItem::Iterator &iterator, QObject *sender,
                 const ModemManager::AutomatorData &data) {
                Q_UNUSED(sender)
                Q_UNUSED(data)
                DF() << iterator->state;
              }
  #else
          StateItemCommand _command = nullptr
  #endif
      );
      bool operator==(const State &state) const;
      bool operator!=(const State &state) const;
    };

    const QVector<ModemManager::AutomatorItem> _automatorScriptInitialization;
    AutomatorItem::Iterator _automatorInitializationIterator;
    const QVector<ModemManager::AutomatorItem> _automatorScriptInitializationConnectionContext;
    AutomatorItem::Iterator _automatorConnectionContextIterator;
    const QVector<ModemManager::AutomatorItem> _automatorScriptAutoconnection;
    AutomatorItem::Iterator _automatorAutoconnectionIterator;
    AutomatorData _automatorData;
  */
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
