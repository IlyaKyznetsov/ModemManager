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
      _automator(new Automator(this)),
      _manager(new Manager(_settings.dBusTimeouts.manager, this)),
      _modem(new Modem(_settings.dBusTimeouts.modem, this)),
      _simManager(new SimManager(_settings.dBusTimeouts.simManager, this)),
      _networkRegistration(new NetworkRegistration(_settings.dBusTimeouts.networkRegistration, this)),
      _connectionManager(new ConnectionManager(_settings.dBusTimeouts.connectionManager, this)),
      _connectionContext(new ConnectionContext(_settings.dBusTimeouts.connectionContext, this))
{
  DF();
  registerOfonoObjectPathProperties();
  QDBusConnection bus(QDBusConnection::systemBus());
  _watcher = new QDBusServiceWatcher(
      Ofono::SERVICE, bus, QDBusServiceWatcher::WatchForRegistration | QDBusServiceWatcher::WatchForUnregistration,
      this);
  connect(_watcher, &QDBusServiceWatcher::serviceRegistered, [this]() {
    _ofonoState.isOfonoConnected = true;
    _manager->reset(Ofono::SERVICE);
  });
  connect(_watcher, &QDBusServiceWatcher::serviceUnregistered, [this]() {
    _ofonoState.isOfonoConnected = false;
    _connectionContext->reset();
    _connectionManager->reset();
    _networkRegistration->reset();
    _simManager->reset();
    _modem->reset();
    _manager->reset();
    _ofonoState.connectionContext.reset();
    _ofonoState.connectionManager.reset();
    _ofonoState.networkRegistration.reset();
    _ofonoState.simManager.reset();
    _ofonoState.modem.reset();
    _automator->reset();
  });

  _settings.debug();
  connect(_automator, &Automator::call, this, &ModemManager::call);
  connect(_manager, &Manager::StateChanged, this, &ModemManager::onStateChanged);
  connect(_modem, &Modem::StateChanged, this, &ModemManager::onStateChanged);
  connect(_simManager, &SimManager::StateChanged, this, &ModemManager::onStateChanged);
  connect(_networkRegistration, &NetworkRegistration::StateChanged, this, &ModemManager::onStateChanged);
  connect(_connectionManager, &ConnectionManager::StateChanged, this, &ModemManager::onStateChanged);
  connect(_connectionContext, &ConnectionContext::StateChanged, this, &ModemManager::onStateChanged);

  if (bus.interface()->isServiceRegistered(Ofono::SERVICE))
  {
    _ofonoState.isOfonoConnected = true;
    _manager->reset(Ofono::SERVICE);
  }
}

void ModemManager::onStateChanged(const State &state)
{
  //  static ulong n = 0;
  //  D("--- STATE --- " << ++n << state);

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

  //  if (State(State::Type::OfonoModemGetProperties, State::Status::CallFinished) == state)
  //    _automator.modemInitialized = true;

  //  if (_ofonoState.isOfonoConnected && _modem->isValid())
  //    _automatorProcessing(state);
  _automator->processing(_settings, state, _ofonoState);
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
    case State::OfonoConnectionManagerPowered:
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
  }
}

void ModemManager::_signalManager(const State &state)
{
  // DF() << state;
  switch (state.type())
  {
    case State::OfonoManagerModemAdded:
    {
      _ofonoState.modem.reset(new ModemManagerData::OfonoState::Modem());
      _modem->reset(state.value().toString());
    }
    break;
    case State::OfonoManagerModemRemoved:
    {
      if (_modem->path() != state.value().toString())
        return;

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
void ModemManager::_automatorProcessing(const State &state)
{
  // Autoconnection

  if (!(_modem->isValid() && _automator.modemInitialized && State::Signal == _automator.modemLockdownStatus &&
        State::Signal == _automator.modemPoweredStatus && State::Signal == _automator.modemOnlineStatus &&
        (State::Signal == _automator.connectionContextAccessPointNameStatus ||
         State::_EMPTYSTATUS_ == _automator.connectionContextAccessPointNameStatus) &&
        (State::Signal == _automator.connectionContextUsernameStatus ||
         State::_EMPTYSTATUS_ == _automator.connectionContextUsernameStatus) &&
        (State::Signal == _automator.connectionContextPasswordStatus ||
         State::_EMPTYSTATUS_ == _automator.connectionContextPasswordStatus) &&
        (State::Signal == _automator.connectionContextActiveStatus ||
         State::_EMPTYSTATUS_ == _automator.connectionContextActiveStatus)))
    return;

  D("1. Restart new modem");
  if (_automator.needPowerOff)
  {
    _automator.needPowerOff = false;
    _automator.modemPoweredStatus = State::CallStarted;
    Q_EMIT call(State::OfonoModemPowered, false);
    return;
  }

  D("2. Modem PowerOn");
  if (!_ofonoState.modem->powered)
  {
    _automator.modemPoweredStatus = State::CallStarted;
    Q_EMIT call(State::OfonoModemPowered, true);
    return;
  }

  D("3. Wait modem interfaces");
  if (!(_simManager->isValid() /*&& _connectionManager->isValid() && _connectionContext->isValid()))
    return;

  D("4. Online modem");
  if (!_ofonoState.modem->online)
  {
    _automator.modemOnlineStatus = State::CallStarted;
    Q_EMIT call(State::OfonoModemOnline, true);
    return;
  }

  D("5. Wait sim provider");
  if (!(_networkRegistration->isValid() && _connectionManager->isValid() && _connectionContext->isValid() &&
        State::Signal == _automator.connectionContextAccessPointNameStatus &&
        State::Signal == _automator.connectionContextUsernameStatus &&
        State::Signal == _automator.connectionContextPasswordStatus &&
        State::Signal == _automator.connectionContextActiveStatus &&
        !_ofonoState.simManager->serviceProviderName.isEmpty()))
    return;

  D("6. Configuration ConnectionContext");
  const ModemManagerData::Settings::Provider provider =
      _settings.providerSettings(_ofonoState.simManager->serviceProviderName);
  if (provider.accessPointName.isValid() &&
      _ofonoState.connectionContext->accessPointName != provider.accessPointName.toString())
  {
    _automator.connectionContextAccessPointNameStatus = State::CallStarted;
    Q_EMIT call(State::OfonoConnectionContextAccessPointName, provider.accessPointName.toString());
    return;
  }
  if (provider.username.isValid() && _ofonoState.connectionContext->username != provider.username.toString())
  {
    _automator.connectionContextUsernameStatus = State::CallStarted;
    Q_EMIT call(State::OfonoConnectionContextUsername, provider.username.toString());
    return;
  }
  if (provider.password.isValid() && _ofonoState.connectionContext->password != provider.password.toString())
  {
    _automator.connectionContextPasswordStatus = State::CallStarted;
    Q_EMIT call(State::OfonoConnectionContextPassword, provider.password.toString());
    return;
  }

  if (_automator.networkRegistrationRegistered && "unregistered" == _ofonoState.networkRegistration->status)
  {
    _automator.networkRegistrationRegistered = false;
    _automator.connectionManagerAtached = false;
    _automator.connectionContextActive = false;
    _automator.modemPoweredStatus = State::CallStarted;
    Q_EMIT call(State::OfonoModemPowered, false);
    return;
  }

  D("7. ConnectionManagerDetached");
  if (_automator.connectionManagerAtached && !_ofonoState.connectionManager->attached)
  {
    _automator.connectionManagerAtached = false;
    _automator.connectionContextActive = false;
    _automator.modemPoweredStatus = State::CallStarted;
    Q_EMIT call(State::OfonoModemPowered, false);
    return;
  }

  D("8. ConnectionContextActive");
  if ("registered" == _ofonoState.networkRegistration->status && _ofonoState.connectionManager->attached &&
      !_ofonoState.connectionContext->active)
  {
    if (_automator.connectionContextActive)
    {
      _automator.connectionContextActive = false;
      _automator.modemPoweredStatus = State::CallStarted;
      Q_EMIT call(State::OfonoModemPowered, false);
    }
    else
    {
      _automator.connectionContextActiveStatus = State::CallStarted;
      Q_EMIT call(State::OfonoConnectionContextActive, true);
    }
    return;
  }
}*/

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
