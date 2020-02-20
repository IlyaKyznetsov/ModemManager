#include "ModemManager.h"
#include "Global.h"

#include "Automator.h"
#include "adapters/ConnectionContext.h"
#include "adapters/ConnectionManager.h"
#include "adapters/Manager.h"
#include "adapters/Modem.h"
#include "adapters/NetworkRegistration.h"
#include "adapters/SimManager.h"
#include "adapters/dbustypes.h"
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusServiceWatcher>

ModemManager::ModemManager(const ModemManagerData::Settings &settings, QObject *parent)
    : QObject(parent),
      _settings(settings),
      _manager(new Manager(_settings.dBusTimeouts.manager, this)),
      _modem(new Modem(_settings.dBusTimeouts.modem, this)),
      _simManager(new SimManager(_settings.dBusTimeouts.simManager, this)),
      _networkRegistration(new NetworkRegistration(_settings.dBusTimeouts.networkRegistration, this)),
      _connectionManager(new ConnectionManager(_settings.dBusTimeouts.connectionManager, this)),
      _connectionContext(new ConnectionContext(_settings.dBusTimeouts.connectionContext, this)),
      _automator(new Automator(_settings, this))
{
  DF();
  registerOfonoObjectPathProperties();
  QDBusConnection bus(QDBusConnection::systemBus());
  _watcher = new QDBusServiceWatcher(
      Ofono::SERVICE, bus, QDBusServiceWatcher::WatchForRegistration | QDBusServiceWatcher::WatchForUnregistration,
      this);
  connect(_watcher, &QDBusServiceWatcher::serviceRegistered, [this]() {
    _ofonoState.isOfonoConnected = true;
    _automator->reset();
    _manager->reset(Ofono::SERVICE);
  });
  connect(_watcher, &QDBusServiceWatcher::serviceUnregistered, [this]() {
    _automator->reset();
    _connectionContext->reset();
    _connectionManager->reset();
    _networkRegistration->reset();
    _simManager->reset();
    _modem->reset();
    _manager->reset();
    _ofonoState.isOfonoConnected = false;
    _ofonoState.connectionContext.reset();
    _ofonoState.connectionManager.reset();
    _ofonoState.networkRegistration.reset();
    _ofonoState.simManager.reset();
    _ofonoState.modem.reset();
  });

  _settings.debug();
  connect(_automator, &Automator::Call, this, &ModemManager::call);
  connect(_manager, &Manager::StateChanged, this, &ModemManager::onStateChanged);
  connect(_modem, &Modem::StateChanged, this, &ModemManager::onStateChanged);
  connect(_simManager, &SimManager::StateChanged, this, &ModemManager::onStateChanged);
  connect(_networkRegistration, &NetworkRegistration::StateChanged, this, &ModemManager::onStateChanged);
  connect(_connectionManager, &ConnectionManager::StateChanged, this, &ModemManager::onStateChanged);
  connect(_connectionContext, &ConnectionContext::StateChanged, this, &ModemManager::onStateChanged);

  if (bus.interface()->isServiceRegistered(Ofono::SERVICE))
  {
    _automator->reset();
    _manager->reset(Ofono::SERVICE);
  }
}

void ModemManager::onStateChanged(const State &state)
{
  static ulong n = 0;
  D("--- STATE --- " << ++n << state);

  QObject *sender_ptr = sender();

  Q_EMIT StateChanged(state);

  if (State::Signal == state.status())
  {
    if (_manager == sender_ptr)
      _signalManager(state);
    else if (_modem == sender_ptr)
      _signalModem(state);
    else if (_simManager == sender_ptr)
      _signalSimManager(state);
    else if (_networkRegistration == sender_ptr)
      _signalNetworkRegistration(state);
    else if (_connectionManager == sender_ptr)
      _signalConnectionManager(state);
    else if (_connectionContext == sender_ptr)
      _signalConnectionContext(state);
  }
  _automator->processing(state);

  //_automationHandler(sender_ptr, state);

  if (State::CallError == state.status())
  {
    C(state.error().message());
    //    throw 1;
  }
}

void ModemManager::call(const State::Type callType, const QVariant &value)
{
  //  DF() << "--- CALL ---" << State(callType, State::_EMPTYSTATUS_, value);
  switch (callType)
  {
    case State::OfonoModemLockdown:
    {
      if (_modem->isValid() && !_ofonoState.modem.isNull() && value.toBool() != _ofonoState.modem->lockdown)
        _modem->call(callType, value);
    }
    break;
    case State::OfonoModemPowered:
    {
      if (_modem->isValid() && !_ofonoState.modem.isNull() && value.toBool() != _ofonoState.modem->powered)
        _modem->call(callType, value);
    }
    break;
    case State::OfonoModemOnline:
    {
      if (_modem->isValid() && !_ofonoState.modem.isNull() && value.toBool() != _ofonoState.modem->online)
        _modem->call(callType, value);
    }
    break;
    case State::OfonoNetworkRegistrationRegister:
    {
      if (_networkRegistration->isValid() && !_ofonoState.networkRegistration.isNull() &&
          "registered" != _ofonoState.networkRegistration->status)
        _networkRegistration->call(callType);
    }
    break;
    case State::OfonoNetworkRegistrationScan:
    {
      if (_networkRegistration->isValid() && !_ofonoState.networkRegistration.isNull() &&
          "searching" != _ofonoState.networkRegistration->status)
        _networkRegistration->call(callType);
    }
    break;
#warning Дописать
    case State::OfonoConnectionManagerRemoveContext:
    case State::OfonoConnectionManagerAddContext:
    {
      if (!_connectionManager->isValid())
        return;
      _connectionManager->call(callType, value);
    }
    break;
    case State::OfonoConnectionContextAccessPointName:
    case State::OfonoConnectionContextUsername:
    case State::OfonoConnectionContextPassword:
    case State::OfonoConnectionContextActive:
    {
      if (!_connectionContext->isValid())
        return;
      _connectionContext->call(callType, value);
    }
    break;
    default:
    {
      C("Неподдерживаемый call:" << callType << value);
      throw astr_global::Exception("Неподдерживаемый call");
    }
    break;
  }
}

void ModemManager::_signalManager(const State &state)
{
  // DF() << state;
  switch (state.type())
  {
    case State::OfonoManagerModemAdded:
    {
      _automator->reset();
      _ofonoState.modem.reset(new ModemManagerData::OfonoState::Modem());
      _modem->reset(state.value().toString());
    }
    break;
    case State::OfonoManagerModemRemoved:
    {
      if (_modem->path() != state.value().toString())
        return;

      _automator->reset();
      _connectionContext->reset();
      _connectionManager->reset();
      _networkRegistration->reset();
      _simManager->reset();
      _modem->reset();

      _ofonoState.connectionContext.reset();
      _ofonoState.connectionManager.reset();
      _ofonoState.networkRegistration.reset();
      _ofonoState.simManager.reset();
      _ofonoState.modem.reset();
    }
    break;
    default: return;
  }
  Q_EMIT OfonoStateChanged(_ofonoState);
}

void ModemManager::_signalModem(const State &state)
{
  // DF() << state;
  if (_ofonoState.modem.isNull())
    throw astr_global::Exception("Нарушен порядок приема сигналов от Ofono");

  switch (state.type())
  {
    case State::OfonoModemPowered:
    {
      _ofonoState.modem->powered = state.value().toBool();
    }
    break;
    case State::OfonoModemOnline:
    {
      _ofonoState.modem->online = state.value().toBool();
    }
    break;
    case State::OfonoModemLockdown:
    {
      _ofonoState.modem->lockdown = state.value().toBool();
    }
    break;
    case State::OfonoModemManufacturer:
    {
      _ofonoState.modem->manufacturer = state.value().toString();
    }
    break;
    case State::OfonoModemModel:
    {
      _ofonoState.modem->model = state.value().toString();
    }
    break;
    case State::OfonoModemSerial:
    {
      _ofonoState.modem->serial = state.value().toString();
    }
    break;
    case State::OfonoModemInterfaceSimManagerAdded:
    {
      _ofonoState.simManager.reset(new ModemManagerData::OfonoState::SimManager());
      _simManager->reset(_modem->path());
    }
    break;
    case State::OfonoModemInterfaceSimManagerRemoved:
    {
      _simManager->reset();
      _ofonoState.simManager.reset();
    }
    break;
    case State::OfonoModemInterfaceNetworkRegistrationAdded:
    {
      _ofonoState.networkRegistration.reset(new ModemManagerData::OfonoState::NetworkRegistration());
      _networkRegistration->reset(_modem->path());
    }
    break;
    case State::OfonoModemInterfaceNetworkRegistrationRemoved:
    {
      _networkRegistration->reset();
      _ofonoState.networkRegistration.reset();
    }
    break;
    case State::OfonoModemInterfaceConnectionManagerAdded:
    {
      _ofonoState.connectionManager.reset(new ModemManagerData::OfonoState::ConnectionManager());
      _connectionManager->reset(_modem->path());
    }
    break;
    case State::OfonoModemInterfaceConnectionManagerRemoved:
    {
      _connectionContext->reset();
      _connectionManager->reset();
      _ofonoState.connectionContext.reset();
      _ofonoState.connectionManager.reset();
    }
    break;
    default: return;
  }
  Q_EMIT OfonoStateChanged(_ofonoState);
}

void ModemManager::_signalSimManager(const State &state)
{
  if (_ofonoState.simManager.isNull())
    throw astr_global::Exception("Нарушен порядок приема сигналов от Ofono");

  switch (state.type())
  {
    case State::OfonoSimManagerCardIdentifier:
    {
      _ofonoState.simManager->cardIdentifier = state.value().toString();
    }
    break;
    case State::OfonoSimManagerServiceProviderName:
    {
      _ofonoState.simManager->serviceProviderName = state.value().toString();
    }
    break;
    default: return;
  }
  Q_EMIT OfonoStateChanged(_ofonoState);
}

void ModemManager::_signalNetworkRegistration(const State &state)
{
  // DF() << state;
  if (_ofonoState.networkRegistration.isNull())
    throw astr_global::Exception("Нарушен порядок приема сигналов от Ofono");

  switch (state.type())
  {
    case State::OfonoNetworkRegistrationStatus:
    {
      _ofonoState.networkRegistration->status = state.value().toString();
    }
    break;
    case State::OfonoNetworkRegistrationName:
    {
      _ofonoState.networkRegistration->name = state.value().toString();
    }
    break;
    case State::OfonoNetworkRegistrationStrength:
    {
      _ofonoState.networkRegistration->strength = state.value().toString();
    }
    break;
    default: return;
  }
  Q_EMIT OfonoStateChanged(_ofonoState);
}

void ModemManager::_signalConnectionManager(const State &state)
{
  // DF() << state;
  if (_ofonoState.connectionManager.isNull())
    throw astr_global::Exception("Нарушен порядок приема сигналов от Ofono");

  switch (state.type())
  {
    case State::OfonoConnectionManagerPowered:
    {
      _ofonoState.connectionManager->powered = state.value().toBool();
    }
    break;
    case State::OfonoConnectionManagerAttached:
    {
      _ofonoState.connectionManager->attached = state.value().toBool();
    }
    break;
    case State::OfonoConnectionManagerRoamingAllowed:
    {
      _ofonoState.connectionManager->roamingAllowed = state.value().toBool();
    }
    break;
    case State::OfonoConnectionManagerContextAdded:
    {
      _ofonoState.connectionContext.reset(new ModemManagerData::OfonoState::ConnectionContext());
      _connectionContext->reset(_connectionManager->contextPath());
    }
    break;
    case State::OfonoConnectionManagerContextRemoved:
    {
      _connectionContext->reset();
      _ofonoState.connectionContext.reset();
    }
    break;
    default: return;
  }
  Q_EMIT OfonoStateChanged(_ofonoState);
}

void ModemManager::_signalConnectionContext(const State &state)
{
  switch (state.type())
  {
    case State::OfonoConnectionContextActive:
    {
      _ofonoState.connectionContext->active = state.value().toBool();
      if (!_ofonoState.connectionContext->active)
      {
        _ofonoState.connectionContext->interface.clear();
        _ofonoState.connectionContext->method.clear();
        _ofonoState.connectionContext->address.clear();
        _ofonoState.connectionContext->netmask.clear();
      }
    }
    break;
    case State::OfonoConnectionContextAccessPointName:
    {
      _ofonoState.connectionContext->accessPointName = state.value().toString();
    }
    break;
    case State::OfonoConnectionContextUsername:
    {
      _ofonoState.connectionContext->username = state.value().toString();
    }
    break;
    case State::OfonoConnectionContextPassword:
    {
      _ofonoState.connectionContext->password = state.value().toString();
    }
    break;
    case State::OfonoConnectionContextType:
    {
      _ofonoState.connectionContext->type = state.value().toString();
    }
    break;
    case State::OfonoConnectionContextAuthenticationMethod:
    {
      _ofonoState.connectionContext->authenticationMethod = state.value().toString();
    }
    break;
    case State::OfonoConnectionContextProtocol:
    {
      _ofonoState.connectionContext->protocol = state.value().toString();
    }
    break;
    case State::OfonoConnectionContextName:
    {
      _ofonoState.connectionContext->name = state.value().toString();
    }
    break;
    case State::OfonoConnectionContextInterface:
    {
      _ofonoState.connectionContext->interface = state.value().toString();
    }
    break;
    case State::OfonoConnectionContextMethod:
    {
      _ofonoState.connectionContext->method = state.value().toString();
    }
    break;
    case State::OfonoConnectionContextAddress:
    {
      _ofonoState.connectionContext->address = state.value().toString();
    }
    break;
    case State::OfonoConnectionContextNetmask:
    {
      _ofonoState.connectionContext->netmask = state.value().toString();
    }
    break;
    default: return;
  }
  Q_EMIT OfonoStateChanged(_ofonoState);
}
/*
void ModemManager::_automationHandler(QObject *sender_ptr, const State &state)
{
  if (_automatorInitializationIterator == _automatorScriptInitialization.cend() ||
      _automatorConnectionContextIterator == _automatorScriptInitializationConnectionContext.cend())
  {
    throw astr_global::Exception("Scripts Overflow");
  }

  static int n = 0;
  D(n++ << " ---STATE---:" << state);

  if (State::Signal == state.status())
  {
    switch (state.type())
    {
      case State::OfonoManagerModemRemoved:
      {
        _automatorInitializationIterator = _automatorScriptInitialization.begin();
        _automatorConnectionContextIterator = _automatorScriptInitializationConnectionContext.begin();
        _automatorData.modemLockdown.clear();
        _automatorData.modemPowered.clear();
        _automatorData.modemOnline.clear();
        _automatorData.simManagerCardIdentifier.clear();
        _automatorData.simManagerServiceProviderName.clear();
        _automatorData.networkRegistrationStatus.clear();
        _automatorData.connectionContextAccessPointName.clear();
        _automatorData.connectionContextUsername.clear();
        _automatorData.connectionContextPassword.clear();
      }
      break;
      case State::OfonoModemInterfaceSimManagerRemoved:
      {
        _automatorData.simManagerCardIdentifier.clear();
        _automatorData.simManagerServiceProviderName.clear();
      }
      break;
      case State::OfonoModemInterfaceNetworkRegistrationRemoved:
      {
        _automatorData.networkRegistrationStatus.clear();
      }
      break;
      case State::OfonoModemInterfaceConnectionManagerRemoved:
      {
        _automatorConnectionContextIterator = _automatorScriptInitializationConnectionContext.begin();
        _automatorData.connectionContextAccessPointName.clear();
        _automatorData.connectionContextUsername.clear();
        _automatorData.connectionContextPassword.clear();
        _automatorData.connectionManagerAttached.clear();
        _automatorData.connectionManagerPowered.clear();
        _automatorData.connectionContextActive.clear();
      }
      break;
      case State::OfonoModemPowered:
      {
        _automatorData.modemPowered = state.value();
      }
      break;
      case State::OfonoModemOnline:
      {
        _automatorData.modemOnline = state.value();
      }
      break;
      case State::OfonoModemLockdown:
      {
        _automatorData.modemLockdown = state.value();
      }
      break;
      case State::OfonoSimManagerCardIdentifier:
      {
        _automatorData.simManagerCardIdentifier = state.value();
      }
      break;
      case State::OfonoSimManagerServiceProviderName:
      {
        _automatorData.simManagerServiceProviderName = state.value();
        _automatorData.provider = _settings.providerSettings(_automatorData.simManagerServiceProviderName.toString());
        //        D("---PROVIDER---:" << _automatorData.provider.accessPointName << _automatorData.provider.username
        //                            << _automatorData.provider.password);
      }
      break;
      case State::OfonoNetworkRegistrationStatus:
      {
        _automatorData.networkRegistrationStatus = state.value();
      }
      break;

      case State::OfonoConnectionContextAccessPointName:
      {
        _automatorData.connectionContextAccessPointName = state.value();
      }
      break;
      case State::OfonoConnectionContextUsername:
      {
        _automatorData.connectionContextUsername = state.value();
      }
      break;
      case State::OfonoConnectionContextPassword:
      {
        _automatorData.connectionContextPassword = state.value();
      }
      break;

      case State::OfonoConnectionManagerAttached:
      {
        _automatorData.connectionManagerAttached = state.value();
      }
      break;
      case State::OfonoConnectionManagerPowered:
      {
        _automatorData.connectionManagerPowered = state.value();
      }
      break;
      case State::OfonoConnectionContextActive:
      {
        _automatorData.connectionContextActive = state.value();
      }
      break;

      default: break;
    }
  }

  AutomatorItem::Iterator iterator = _automatorInitializationIterator + 1;
  if (_automatorScriptInitialization.cend() != iterator && (*iterator) == state)
  {
    _automatorInitializationIterator = iterator;
    if (_automatorInitializationIterator->command)
      _automatorInitializationIterator->command(_automatorInitializationIterator, sender_ptr, _automatorData);
  }

  iterator = _automatorConnectionContextIterator + 1;
  if (_automatorScriptInitializationConnectionContext.cend() != iterator && (*iterator) == state)
  {
    _automatorConnectionContextIterator = iterator;
    if (_automatorConnectionContextIterator->command)
      _automatorConnectionContextIterator->command(_automatorConnectionContextIterator, sender_ptr, _automatorData);
  }

  if (!(_automatorInitializationIterator + 1 == _automatorScriptInitialization.cend() &&
        _automatorConnectionContextIterator + 1 == _automatorScriptInitializationConnectionContext.cend()))
    return;

  const QVector<State::Type> types{State::OfonoConnectionContextActive,
                                   State::OfonoConnectionManagerAttached,
                                   State::OfonoConnectionManagerPowered,
                                   State::OfonoNetworkRegistrationStatus,
                                   State::OfonoModemOnline,
                                   State::OfonoModemPowered};
  if (!types.contains(state.type()))
    return;

  if (!_automatorData.modemPowered.toBool())
  {
    _modem->call(State::OfonoModemPowered, true);
    return;
  }

  if (!_automatorData.modemOnline.toBool())
  {
    _modem->call(State::OfonoModemOnline, true);
    return;
  }

  //  State::OfonoNetworkRegistrationStatus
  //[readonly: "unregistered" "registered" "searching" "denied" "unknown" "roaming"];
  const QString status = _automatorData.networkRegistrationStatus.toString();
  if ("unregistered" == status)
  {
    _networkRegistration->call(State::OfonoNetworkRegistrationRegister);
    return;
  }
  else if ("registered" != status)
  {
    return;
  }

  if (!_automatorData.connectionManagerPowered.toBool())
  {
    _connectionManager->call(State::OfonoConnectionManagerPowered, true);
    return;
  }

  if (_automatorData.connectionManagerAttached.toBool() && !_automatorData.connectionContextActive.toBool())
  {
    _connectionContext->call(State::OfonoConnectionContextActive, true);
    return;
  }

  I("+++ STATUS +++:" << _automatorData.connectionContextActive << _automatorData.connectionManagerAttached);
}
*/
void ModemManager::debugOfonoState(const ModemManagerData::OfonoState &state)
{
  D("------------ OfonoState ------------");
  D("Ofono connected:" << state.isOfonoConnected);
  if (!state.isOfonoConnected)
    return;

  if (state.modem.isNull())
    return;

  D("----Modem----");
  D("Powered     :" << state.modem->powered);
  D("Online      :" << state.modem->online);
  D("Lockdown    :" << state.modem->lockdown);
  D("Manufacturer:" << state.modem->manufacturer);
  D("Model       :" << state.modem->model);
  D("Serial      :" << state.modem->serial);

  if (!state.simManager.isNull())
  {
    D("----SimManager----");
    D("CardIdentifier     :" << state.simManager->cardIdentifier);
    D("ServiceProviderName:" << state.simManager->serviceProviderName);
  }

  if (!state.networkRegistration.isNull())
  {
    D("----NetworkRegistration----");
    D("Status  :" << state.networkRegistration->status);
    D("Name    :" << state.networkRegistration->name);
    D("Strength:" << state.networkRegistration->strength);
  }

  if (!state.connectionManager.isNull())
  {
    D("----ConnectionManager----");
    D("Attached      :" << state.connectionManager->attached);
    D("RoamingAllowed:" << state.connectionManager->roamingAllowed);
    D("Powered       :" << state.connectionManager->powered);
  }

  if (!state.connectionContext.isNull())
  {
    D("--ConnectionContext--");
    D("Active              :" << state.connectionContext->active);
    D("AccessPointName     :" << state.connectionContext->accessPointName);
    D("Username            :" << state.connectionContext->username);
    D("Password            :" << state.connectionContext->password);
    D("Type                :" << state.connectionContext->type);
    D("AuthenticationMethod:" << state.connectionContext->authenticationMethod);
    D("Protocol            :" << state.connectionContext->protocol);
    D("Name                :" << state.connectionContext->name);
    D("Interface           :" << state.connectionContext->interface);
    D("Method              :" << state.connectionContext->method);
    D("Address             :" << state.connectionContext->address);
    D("Netmask             :" << state.connectionContext->netmask);
  }
}

void ModemManager::t_modemPowered(bool value)
{
  DF() << value;
  call(State::OfonoModemPowered, value);
}

void ModemManager::t_modemOnline(bool value)
{
  DF() << value;
  call(State::OfonoModemOnline, value);
}

void ModemManager::t_modemLockdown(bool value)
{
  DF() << value;
  call(State::OfonoModemLockdown, value);
}

void ModemManager::t_networkRegistered()
{
  call(State::OfonoNetworkRegistrationRegister, QVariant());
}

void ModemManager::t_networkUnregistered()
{
  call(State::OfonoNetworkRegistrationDeregister, QVariant());
}

void ModemManager::t_networkScan()
{
  call(State::OfonoNetworkRegistrationScan, QVariant());
}

void ModemManager::t_contextSetAPN(QString value)
{
  call(State::OfonoConnectionContextAccessPointName, value);
}

void ModemManager::t_contextSetUsername(QString value)
{
  call(State::OfonoConnectionContextUsername, value);
}

void ModemManager::t_contextSetPassword(QString value)
{
  call(State::OfonoConnectionContextPassword, value);
}

void ModemManager::t_contextSetActive(bool value)
{
  call(State::OfonoConnectionContextActive, true);
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
/*
ModemManager::AutomatorItem::AutomatorItem(const State &_state,
ModemManager::AutomatorItem::StateItemCommand _command) : state(_state), command(_command)
{
}

bool ModemManager::AutomatorItem::operator==(const State &state) const
{
  return state == this->state;
}

bool ModemManager::AutomatorItem::operator!=(const State &state) const
{
  return state != this->state;
}
*/
