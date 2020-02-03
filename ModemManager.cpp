#include "ModemManager.h"
#include "Global.h"

#include "adapters/ConnectionContext.h"
#include "adapters/ConnectionManager.h"
#include "adapters/Manager.h"
#include "adapters/Modem.h"
#include "adapters/NetworkRegistration.h"
#include "adapters/OfonoManager.h"
#include "adapters/SimManager.h"

ModemManager::ModemManager(const int &msCallTimeout, QObject *parent)
    : QObject(parent),
      _ofonoManager(new OfonoManager(this)),
      _manager(new Manager(this)),
      _modem(new Modem(this)),
      _simManager(new SimManager(this)),
      _networkRegistration(new NetworkRegistration(this)),
      _connectionManager(new ConnectionManager(this)),
      _connectionContext(new ConnectionContext(this)),
      _deferredCall(new DeferredCall(msCallTimeout, this))
{
  connect(this, &ModemManager::OfonoStateChanged, this, &ModemManager::debugOfonoState);

  connect(_deferredCall, &DeferredCall::SendCallData, this, &ModemManager::onDeferredCall);

  connect(_ofonoManager, &OfonoManager::StateChanged, this, &ModemManager::onStateChanged);
  connect(_manager, &Manager::StateChanged, this, &ModemManager::onStateChanged);
  connect(_modem, &Modem::StateChanged, this, &ModemManager::onStateChanged);
  connect(_simManager, &SimManager::StateChanged, this, &ModemManager::onStateChanged);
  connect(_networkRegistration, &NetworkRegistration::StateChanged, this, &ModemManager::onStateChanged);
  connect(_connectionManager, &ConnectionManager::StateChanged, this, &ModemManager::onStateChanged);
  connect(_connectionContext, &ConnectionContext::StateChanged, this, &ModemManager::onStateChanged);

  _ofonoManager->reset(Ofono::SERVICE);
}

void ModemManager::onStateChanged(const State &state)
{
  QObject *sender_ptr = sender();
  //  QString sender_str;
  DF() << state;
  if (State::Signal == state._status)
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
  autoConnection(state);
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
  switch (state._type)
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
  switch (state._type)
  {
    case State::OfonoManagerModemAdded:
    {
      _ofonoState.modem.reset(new OfonoState::Modem());
      _modem->reset(state._value.toString());
    }
    break;
    case State::OfonoManagerModemRemoved:
    {
      if (_modem->path() != state._value.toString())
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

  switch (state._type)
  {
    case State::OfonoModemPowered:
    {
      _ofonoState.modem->powered = state._value.toBool();
    }
    break;
    case State::OfonoModemOnline:
    {
      _ofonoState.modem->online = state._value.toBool();
    }
    break;
    case State::OfonoModemLockdown:
    {
      _ofonoState.modem->lockdown = state._value.toBool();
    }
    break;
    case State::OfonoModemManufacturer:
    {
      _ofonoState.modem->manufacturer = state._value.toString();
    }
    break;
    case State::OfonoModemModel:
    {
      _ofonoState.modem->model = state._value.toString();
    }
    break;
    case State::OfonoModemSerial:
    {
      _ofonoState.modem->serial = state._value.toString();
    }
    break;
    case State::OfonoModemInterfaceSimManagerAdded:
    {
      DF() << "OfonoModemInterfaceSimManagerAdded";
      _ofonoState.simManager.reset(new OfonoState::SimManager());
      _simManager->reset(_modem->path());
    }
    break;
    case State::OfonoModemInterfaceSimManagerRemoved:
    {
      DF() << "OfonoModemInterfaceSimManagerRemoved";
      _simManager->reset();
      _ofonoState.simManager.reset();
    }
    break;
    case State::OfonoModemInterfaceNetworkRegistrationAdded:
    {
      _ofonoState.networkRegistration.reset(new OfonoState::NetworkRegistration());
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
      _ofonoState.connectionManager.reset(new OfonoState::ConnectionManager());
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

  switch (state._type)
  {
    case State::OfonoSimManagerCardIdentifier:
    {
      _ofonoState.simManager->cardIdentifier = state._value.toString();
    }
    break;
    case State::OfonoSimManagerServiceProviderName:
    {
      _ofonoState.simManager->serviceProviderName = state._value.toString();
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

  switch (state._type)
  {
    case State::OfonoNetworkRegistrationStatus:
    {
      _ofonoState.networkRegistration->status = state._value.toString();
    }
    break;
    case State::OfonoNetworkRegistrationName:
    {
      _ofonoState.networkRegistration->name = state._value.toString();
    }
    break;
    case State::OfonoNetworkRegistrationStrength:
    {
      _ofonoState.networkRegistration->strength = state._value.toString();
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

  switch (state._type)
  {
    case State::OfonoConnectionManagerPowered:
    {
      _ofonoState.connectionManager->powered = state._value.toBool();
    }
    break;
    case State::OfonoConnectionManagerAttached:
    {
      _ofonoState.connectionManager->attached = state._value.toBool();
    }
    break;
    case State::OfonoConnectionManagerRoamingAllowed:
    {
      _ofonoState.connectionManager->roamingAllowed = state._value.toBool();
    }
    break;
    case State::OfonoConnectionManagerContextAdded:
    {
      _ofonoState.connectionContext.reset(new OfonoState::ConnectionContext());
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
  switch (state._type)
  {
    case State::OfonoConnectionContextActive:
    {
      _ofonoState.connectionContext->active = state._value.toBool();
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
      _ofonoState.connectionContext->accessPointName = state._value.toString();
    }
    break;
    case State::OfonoConnectionContextUsername:
    {
      _ofonoState.connectionContext->username = state._value.toString();
    }
    break;
    case State::OfonoConnectionContextPassword:
    {
      _ofonoState.connectionContext->password = state._value.toString();
    }
    break;
    case State::OfonoConnectionContextType:
    {
      _ofonoState.connectionContext->type = state._value.toString();
    }
    break;
    case State::OfonoConnectionContextAuthenticationMethod:
    {
      _ofonoState.connectionContext->authenticationMethod = state._value.toString();
    }
    break;
    case State::OfonoConnectionContextProtocol:
    {
      _ofonoState.connectionContext->protocol = state._value.toString();
    }
    break;
    case State::OfonoConnectionContextName:
    {
      _ofonoState.connectionContext->name = state._value.toString();
    }
    break;
    case State::OfonoConnectionContextInterface:
    {
      _ofonoState.connectionContext->interface = state._value.toString();
    }
    break;
    case State::OfonoConnectionContextMethod:
    {
      _ofonoState.connectionContext->method = state._value.toString();
    }
    break;
    case State::OfonoConnectionContextAddress:
    {
      _ofonoState.connectionContext->address = state._value.toString();
    }
    break;
    case State::OfonoConnectionContextNetmask:
    {
      _ofonoState.connectionContext->netmask = state._value.toString();
    }
    break;
    default: return;
  }
  Q_EMIT OfonoStateChanged(_ofonoState);
}

void ModemManager::autoConnection(const State &state)
{
  if (!_ofonoState.isOfonoConnected)
    return;

  switch (state.status())
  {
    case State::CallFinished:
    case State::CallError: _deferredCall->clear(state.type()); break;
    default: break;
  }

  if (_ofonoState.modem.isNull() || !_modem->isValid())
    return;

  if (State::OfonoManagerModemAdded == state.type())
  {
    _deferredCall->deferredCall(State::OfonoModemLockdown, true);
    return;
  }

  if (_ofonoState.modem->lockdown)
  {
    _deferredCall->deferredCall(State::OfonoModemLockdown, false);
    return;
  }

  if (!_ofonoState.modem->powered)
  {
    _deferredCall->deferredCall(State::OfonoModemPowered, true);
    return;
  }

  if (!_ofonoState.modem->online)
  {
    _deferredCall->deferredCall(State::OfonoModemOnline, true);
    return;
  }

  if (_ofonoState.networkRegistration.isNull() || !_networkRegistration->isValid())
    return;

  if (_ofonoState.connectionManager.isNull() || !_connectionManager->isValid())
    return;

  if (!_ofonoState.connectionManager->powered)
  {
    _deferredCall->deferredCall(State::OfonoConnectionManagerPowered, true);
    return;
  }

  if (_ofonoState.connectionContext.isNull() || !_connectionContext->isValid())
    return;

  if (!_ofonoState.connectionManager->attached)
  {
    _deferredCall->deferredCall(State::OfonoModemOnline, false);
    return;
  }

  if (!_ofonoState.connectionContext->active)
  {
    _deferredCall->deferredCall(State::OfonoConnectionContextActive, true);
    return;
  }
}

void ModemManager::debugOfonoState(const ModemManager::OfonoState &state)
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
