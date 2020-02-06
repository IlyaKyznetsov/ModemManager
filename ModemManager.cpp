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
      _autoStates{State(State::_EMPTYTYPE_, State::_EMPTYSTATUS_),             // 0
                  State(State::OfonoModemGetProperties, State::CallStarted),   // 1
                  State(State::OfonoModemGetProperties, State::CallFinished),  // 2
                  State(State::OfonoModemLockdown, State::CallStarted, true),  // 3
                  State(State::OfonoModemLockdown, State::CallFinished),       // 4
                  State(State::OfonoModemLockdown, State::Signal, true),       // 5
                  State(State::OfonoModemLockdown, State::CallStarted, false), // 6
                  State(State::OfonoModemLockdown, State::CallFinished),       // 7
                  State(State::OfonoModemLockdown, State::Signal, false),      // 8
                  State(State::OfonoModemPowered, State::CallStarted, true),   // 9
                  State(State::OfonoModemPowered, State::CallFinished),        // 10
                  State(State::OfonoModemPowered, State::Signal, true),        // 11
                  //
                  State(State::OfonoConnectionManagerGetProperties, State::CallStarted),  // 12
                  State(State::OfonoConnectionManagerGetProperties, State::CallFinished), // 13
                  State(State::OfonoConnectionManagerGetContexts, State::CallStarted),    // 14
                  State(State::OfonoConnectionManagerGetContexts, State::CallFinished),   // 15
                  //
                  State(State::OfonoConnectionManagerRemoveContext, State::Status::CallStarted),  // 16
                  State(State::OfonoConnectionManagerRemoveContext, State::Status::CallFinished), // 17
                  State(State::OfonoConnectionManagerContextRemoved, State::Status::Signal),      // 18
                  //
                  State(State::OfonoConnectionManagerAddContext, State::Status::CallStarted),  // 19
                  State(State::OfonoConnectionManagerAddContext, State::Status::CallFinished), // 20

                  State(State::OfonoConnectionContextGetProperties, State::CallStarted),  // 21
                  State(State::OfonoConnectionContextGetProperties, State::CallFinished), // 22
                  //
                  State(State::OfonoModemOnline, State::CallStarted, true), // 23
                  State(State::OfonoModemOnline, State::CallFinished),      // 24
                  State(State::OfonoModemOnline, State::Signal, true)},     // 25
      _autoStateIterator(_autoStates.begin()),
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

  _autoStateChangedHandler(state);
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

void ModemManager::_autoStateChangedHandler(const State &state)
{
  if (state.type() == State::OfonoManagerModemRemoved)
  {
    _autoStateIterator = _autoStates.cend();
    return;
  }
  else if (state.type() == State::OfonoModemGetProperties && State::CallStarted == state.status())
  {
    _autoStateIterator = _autoStates.cbegin();
  }

  if (state.status() == State::CallError)
  {
    D("ERROR:" << state.error().message());
  }

  /*
QVector<State>::const_iterator *p_iterator = nullptr;

if (_modem == sender_ptr)
  p_iterator = &_modemStateIterator;
else if (_simManager == sender_ptr)
  p_iterator = &_simManagerStateIterator;
else if (_networkRegistration == sender_ptr)
  p_iterator = &_networkRegistrationStateIterator;
else if (_connectionManager == sender_ptr)
  p_iterator = &_connectionManagerStateIterator;
else if (_connectionContext == sender_ptr)
  p_iterator = &_connectionContextStateIterator;
else
  return;


  if (p_iterator && (*p_iterator) == _autoStates.cend())
    return;
    */
  if (_autoStateIterator == _autoStates.cend())
    return;

  D("---START---:" << state << *_autoStateIterator);

  QVector<State>::const_iterator nextIterator = _autoStateIterator + 1;
  if (nextIterator == _autoStates.cend() || nextIterator->operator!=(state))
    return;

  const int index = nextIterator - _autoStates.begin();

  switch (index)
  {
    case 1:
    {
      ++_autoStateIterator;
      D("XXX1:" << index << state << *_autoStateIterator);
    }
    break;
    case 2:
    {
      _autoStateIterator += (_ofonoState.modem->lockdown ? 4 : 1);
      D("XXX2:" << index << state << *_autoStateIterator);
      _modem->call(State::OfonoModemLockdown, !_ofonoState.modem->lockdown);
    }
    break;
    case 3:
    {
      ++_autoStateIterator;
      D("XXX3:" << index << state << *_autoStateIterator);
    }
    break;
    case 4:
    {
      ++_autoStateIterator;
      D("XXX4:" << index << state << *_autoStateIterator);
    }
    break;
    case 5:
    {
      ++_autoStateIterator;
      D("XXX5:" << index << state << *_autoStateIterator);
      _modem->call(State::OfonoModemLockdown, false);
    }
    break;
    case 6:
    {
      ++_autoStateIterator;
      D("XXX6:" << index << state << *_autoStateIterator);
    }
    break;
    case 7:
    {
      ++_autoStateIterator;
      D("XXX7:" << index << state << *_autoStateIterator);
    }
    break;
    case 8:
    {
      ++_autoStateIterator;
      D("XXX8:" << index << state << *_autoStateIterator);
      _modem->call(State::OfonoModemPowered, true);
    }
    break;
    case 9:
    {
      ++_autoStateIterator;
      D("XXX9:" << index << state << *_autoStateIterator);
    }
    break;
    case 10:
    {
      ++_autoStateIterator;
      D("XXX10:" << index << state << *_autoStateIterator);
    }
    break;
    case 11:
    {
      ++_autoStateIterator;
      D("XXX11:" << index << state << *_autoStateIterator);
    }
    break;
    case 12:
    {
      ++_autoStateIterator;
      D("XXX12:" << index << state << *_autoStateIterator);
    }
    break;
    case 13:
    {
      ++_autoStateIterator;
      D("XXX13:" << index << state << *_autoStateIterator);
    }
    break;
    case 14:
    {
      ++_autoStateIterator;
      D("XXX14:" << index << state << *_autoStateIterator);
    }
    break;
    case 15:
    {
      ++_autoStateIterator;
      D("XXX15:" << index << state << *_autoStateIterator);

      D("COUNT:" << _connectionManager->contextsCount());
      switch (_connectionManager->contextsCount())
      {
        case 0:
        {
          _autoStateIterator += 3;
          _connectionManager->call(State::OfonoConnectionManagerAddContext, "internet");
        }
        break;
        case 1:
        {
          _autoStateIterator += 5;
        }
        break;
        default:
        {
          _autoStateIterator += 1;
          _connectionManager->call(State::OfonoConnectionManagerRemoveContext, _connectionManager->contextPath());
        }
        break;
      }
      /// for Test:
      /// _connectionManager->call(State::OfonoConnectionManagerAddContext, "internet");
    }
    break;
    case 16:
    {
      ++_autoStateIterator;
      D("XXX16:" << index << state << *_autoStateIterator);
    }
    break;
    case 17:
    {
      ++_autoStateIterator;
      D("XXX17:" << index << state << *_autoStateIterator);
    }
    break;
    case 18:
    {
      if (_connectionManager->contextsCount() > 1)
      {
        _autoStateIterator -= 2;
        /// D("RRRRRRRRRR:" << _connectionManager->contextPath());
        _connectionManager->call(State::OfonoConnectionManagerRemoveContext, _connectionManager->contextPath());
      }
      else
      {
        _autoStateIterator += 2;
      }
      D("XXX18:" << index << state << *_autoStateIterator);
    }
    break;
    case 19:
    {
      ++_autoStateIterator;
      D("XXX19:" << index << state << *_autoStateIterator);
    }
    break;
    case 20:
    {
      ++_autoStateIterator;
      D("XXX20:" << index << state << *_autoStateIterator);
    }
    break;
    case 21:
    {
      ++_autoStateIterator;
      D("XXX21:" << index << state << *_autoStateIterator);
    }
    break;
    case 22:
    {
      ++_autoStateIterator;
      D("XXX22:" << index << state << *_autoStateIterator);
      _modem->call(State::OfonoModemOnline, true);
    }
    break;
    case 23:
    {
      ++_autoStateIterator;
      D("XXX23:" << index << state << *_autoStateIterator);
    }
    break;
    case 24:
    {
      ++_autoStateIterator;
      D("XXX23:" << index << state << *_autoStateIterator);
    }
    break;
    case 25:
    {
      ++_autoStateIterator;
      D("XXX23:" << index << state << *_autoStateIterator);
    }
    break;
    default: break;
  }
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
