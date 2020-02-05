#include "ModemManager.h"
#include "Global.h"

#include "adapters/ConnectionContext.h"
#include "adapters/ConnectionManager.h"
#include "adapters/Manager.h"
#include "adapters/Modem.h"
#include "adapters/NetworkRegistration.h"
#include "adapters/OfonoManager.h"
#include "adapters/SimManager.h"

ModemManager::ModemManager(const ModemManagerData::Settings &settings, QObject *parent)
    : QObject(parent),
      _settings(settings),
      _ofonoManager(new OfonoManager(this)),
      _manager(new Manager(_settings.dBusTimeouts.manager, this)),
      _modem(new Modem(_settings.dBusTimeouts.modem, this)),
      _simManager(new SimManager(_settings.dBusTimeouts.simManager, this)),
      _networkRegistration(new NetworkRegistration(_settings.dBusTimeouts.networkRegistration, this)),
      _connectionManager(new ConnectionManager(_settings.dBusTimeouts.connectionManager, this)),
      _connectionContext(new ConnectionContext(_settings.dBusTimeouts.connectionContext, this)) /*,
       _deferredCall(new DeferredCall(_settings.modemManagerimeouts.deferredCall, this))*/
{
  DF();
  _settings.debug();

  //  connect(this, &ModemManager::OfonoStateChanged, this, &ModemManager::debugOfonoState);

  //  connect(_deferredCall, &DeferredCall::SendCallData, this, &ModemManager::onDeferredCall);

  connect(_ofonoManager, &OfonoManager::StateChanged, this, &ModemManager::onStateChanged);
  connect(_manager, &Manager::StateChanged, this, &ModemManager::onStateChanged);
  connect(_modem, &Modem::StateChanged, this, &ModemManager::onStateChanged);
  connect(_simManager, &SimManager::StateChanged, this, &ModemManager::onStateChanged);
  connect(_networkRegistration, &NetworkRegistration::StateChanged, this, &ModemManager::onStateChanged);
  connect(_connectionManager, &ConnectionManager::StateChanged, this, &ModemManager::onStateChanged);
  connect(_connectionContext, &ConnectionContext::StateChanged, this, &ModemManager::onStateChanged);

  _ofonoManager->reset(Ofono::SERVICE);
}

// bool ModemManager::deferredCall(const State::Type type, const QVariant &value)
//{
//  return _deferredCall->deferredCall(type, value);
//}

void ModemManager::onStateChanged(const State &state)
{
  DF() << state;

  QObject *sender_ptr = sender();

  Q_EMIT StateChanged(state);

  if (State::Signal == state.status())
  {
    if (_ofonoManager == sender_ptr)
      _signalOfonoManager(state);
    else if (_manager == sender_ptr)
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
}

void ModemManager::onDeferredCall(State::Type type, const QVariant &value)
{
  DF() << type;
  switch (type)
  {
    case State::OfonoModemLockdown:
    case State::OfonoModemPowered:
    case State::OfonoModemOnline:
    {
      if (_modem->isValid())
        _modem->call(type, value);
    }
    break;
    case State::State::OfonoNetworkRegistrationScan:
    case State::State::OfonoNetworkRegistrationRegister:
    case State::State::OfonoNetworkRegistrationDeregister:
    case State::State::OfonoNetworkRegistrationGetOperators:
    {
      if (_networkRegistration->isValid())
        _networkRegistration->call(type);
    }
    break;
    case State::OfonoConnectionContextActive:
    {
      if (_connectionContext->isValid())
        _connectionContext->call(type, value);
    }
    break;
    default: break;
  }
}

void ModemManager::_signalOfonoManager(const State &state)
{
  // DF() << state;
  switch (state.type())
  {
    case State::OfonoServiceRegistered:
    {
      _ofonoState.isOfonoConnected = true;
      _manager->reset(Ofono::SERVICE);
    }
    break;
    case State::OfonoServiceUnregistered:
    {
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
    }
    break;
    default: return;
  }
  Q_EMIT OfonoStateChanged(_ofonoState);
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
  DF() << state;
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
  _modem->call(State::OfonoModemPowered, value);
}

void ModemManager::t_modemOnline(bool value)
{
  DF() << value;
  _modem->call(State::OfonoModemOnline, value);
}

void ModemManager::t_modemLockdown(bool value)
{
  DF() << value;
  _modem->call(State::OfonoModemLockdown, value);
}

void ModemManager::t_contextSetAPN(QString value)
{
  _connectionContext->call(State::OfonoConnectionContextAccessPointName, value);
}

void ModemManager::t_contextSetUsername(QString value)
{
  _connectionContext->call(State::OfonoConnectionContextUsername, value);
}

void ModemManager::t_contextSetPassword(QString value)
{
  _connectionContext->call(State::OfonoConnectionContextPassword, value);
}

void ModemManager::t_contextSetActive(bool value)
{
  _connectionContext->call(State::OfonoConnectionContextActive, value);
}
