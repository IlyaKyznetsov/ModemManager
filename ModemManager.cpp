#include "ModemManager.h"
#include "Global.h"

#include "adapters/ConnectionContext.h"
#include "adapters/ConnectionManager.h"
#include "adapters/Manager.h"
#include "adapters/Modem.h"
#include "adapters/NetworkRegistration.h"
#include "adapters/OfonoManager.h"
#include "adapters/SimManager.h"

ModemManager::ModemManager(QObject *parent)
    : QObject(parent),
      _ofonoManager(new OfonoManager(this)),
      _manager(new Manager(this)),
      _modem(new Modem(this)),
      _simManager(new SimManager(this)),
      _networkRegistration(new NetworkRegistration(this)),
      _connectionManager(new ConnectionManager(this)),
      _connectionContext(new ConnectionContext(this))
{
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
  //  DF() << sender_str << state;
  if (_ofonoManager == sender_ptr)
    _ofonoManagerChanged(state);
  else if (_manager == sender_ptr)
    _managerChanged(state);
  else if (_modem == sender_ptr)
    _modemChanged(state);
  else if (_simManager == sender_ptr)
    _simManagerChanged(state);
  else if (_networkRegistration == sender_ptr)
    _networkRegistrationChanged(state);
  else if (_connectionManager == sender_ptr)
    _connectionManagerChanged(state);
  else if (_connectionContext == sender_ptr)
    _connectionContextChanged(state);
}

void ModemManager::_ofonoManagerChanged(State state)
{
  DF() << state;
  if (State::Signal == state._status)
  {
    if (State::OfonoServiceUnregistered == state._type)
    {
      _modem->reset();
      _manager->reset();
    }
    else if (State::OfonoServiceRegistered == state._type)
      _manager->reset(Ofono::SERVICE);
  }
}

void ModemManager::_managerChanged(State state)
{
  DF() << state;
  if (State::Signal == state._status)
  {
    if (State::OfonoManagerModemRemoved == state._type && _modem->path() == state._value.toString())
      _modem->reset();
    else if (State::OfonoManagerModemAdded == state._type)
      _modem->reset(state._value.toString());
  }
}

void ModemManager::_modemChanged(State state)
{
  DF() << state;
  // Call ...

  if (State::Signal == state._status)
  {
    switch (state._type)
    {
      case State::OfonoModemLockdown:
      {
        if (!state._value.toBool())
          _modem->call(State::OfonoModemLockdown, false);
      }
      break;
      case State::OfonoModemPowered:
      {
        if (!state._value.toBool())
          _modem->call(State::OfonoModemPowered, true);
      }
      break;
      case State::OfonoModemOnline:
      {
        if (!state._value.toBool())
          _modem->call(State::OfonoModemOnline, true);
      }
      break;
      case State::OfonoModemInterfaceSimManagerAdded:
      {
        //        D("XXX: State::OfonoModemInterfaceSimManagerAdded" << _modem->isValid() << !_simManager->isValid()
        //                                                           << _modem->path());
        if (_modem->isValid())
          _simManager->reset(_modem->path());
      }
      break;
      case State::OfonoModemInterfaceSimManagerRemoved:
      {
        _simManager->reset();
      }
      break;
      case State::OfonoModemInterfaceNetworkRegistrationAdded:
      {
      }
      break;
      case State::OfonoModemInterfaceNetworkRegistrationRemoved:
      {
      }
      break;
      case State::OfonoModemInterfaceConnectionManagerAdded:
      {
      }
      break;
      case State::OfonoModemInterfaceConnectionManagerRemoved:
      {
      }
      break;
      default: break;
    }
  }
}

void ModemManager::_simManagerChanged(State state)
{
  DF() << state;
}

void ModemManager::_networkRegistrationChanged(State state)
{
  DF() << state;
}

void ModemManager::_connectionManagerChanged(State state)
{
  DF() << state;
}

void ModemManager::_connectionContextChanged(State state)
{
  DF() << state;
}
