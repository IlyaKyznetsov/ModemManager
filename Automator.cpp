#include "Automator.h"
#include "adapters/Modem.h"

Automator::Automator(const ModemManagerData::Settings &settings, QObject *parent)
    : QObject(parent),
      _settings(settings),
      _timer(new QTimer()),
      _scripts{
          {ManagerModemAdded,
           new AutomatorScript({AutomatorScript::emptyState,
                                AutomatorScript::Item(State(State::OfonoModemGetProperties, State::CallStarted)),
                                AutomatorScript::Item(State(State::OfonoModemGetProperties, State::CallFinished))})},
          {ModemLockdownDisable,
           new AutomatorScript({AutomatorScript::emptyState,
                                AutomatorScript::Item(State(State::OfonoModemLockdown, State::CallStarted, false)),
                                AutomatorScript::Item(State(State::OfonoModemLockdown, State::CallFinished)),
                                AutomatorScript::Item(State(State::OfonoModemLockdown, State::Signal, false))})},
          {ModemLockdownEnable,
           new AutomatorScript({AutomatorScript::emptyState,
                                AutomatorScript::Item(State(State::OfonoModemLockdown, State::CallStarted, true)),
                                AutomatorScript::Item(State(State::OfonoModemLockdown, State::CallFinished)),
                                AutomatorScript::Item(State(State::OfonoModemLockdown, State::Signal, true))})},
          {ModemPoweredDisable,
           new AutomatorScript({AutomatorScript::emptyState,
                                AutomatorScript::Item(State(State::OfonoModemPowered, State::CallStarted, false)),
                                AutomatorScript::Item(State(State::OfonoModemPowered, State::CallFinished)),
                                AutomatorScript::Item(State(State::OfonoModemPowered, State::Signal, false))})},
          {ModemPoweredEnable,
           new AutomatorScript({AutomatorScript::emptyState,
                                AutomatorScript::Item(State(State::OfonoModemPowered, State::CallStarted, true)),
                                AutomatorScript::Item(State(State::OfonoModemPowered, State::CallFinished)),
                                AutomatorScript::Item(State(State::OfonoModemPowered, State::Signal, true))})},
          {ModemOnlineDisable,
           new AutomatorScript({AutomatorScript::emptyState,
                                AutomatorScript::Item(State(State::OfonoModemOnline, State::CallStarted, false)),
                                AutomatorScript::Item(State(State::OfonoModemOnline, State::CallFinished)),
                                AutomatorScript::Item(State(State::OfonoModemOnline, State::Signal, false))})},
          {ModemOnlineEnable,
           new AutomatorScript({AutomatorScript::emptyState,
                                AutomatorScript::Item(State(State::OfonoModemOnline, State::CallStarted, true)),
                                AutomatorScript::Item(State(State::OfonoModemOnline, State::CallFinished)),
                                AutomatorScript::Item(State(State::OfonoModemOnline, State::Signal, true))})},
          {SimManagerAdded,
           new AutomatorScript(
               {AutomatorScript::emptyState,
                AutomatorScript::Item(State(State::OfonoSimManagerGetProperties, State::CallStarted)),
                AutomatorScript::Item(State(State::OfonoSimManagerGetProperties, State::CallFinished))})},
          {NetworkRegistrationAdded,
           new AutomatorScript(
               {AutomatorScript::emptyState,
                AutomatorScript::Item(State(State::OfonoNetworkRegistrationGetProperties, State::CallStarted)),
                AutomatorScript::Item(State(State::OfonoNetworkRegistrationGetProperties, State::CallFinished))})},
          {ConnectionManagerAdded,
           new AutomatorScript(
               {AutomatorScript::emptyState,
                AutomatorScript::Item(State(State::OfonoConnectionManagerGetProperties, State::CallStarted)),
                AutomatorScript::Item(State(State::OfonoConnectionManagerGetProperties, State::CallFinished)),
                AutomatorScript::Item(State(State::OfonoConnectionManagerGetContexts, State::CallStarted)),
                AutomatorScript::Item(State(State::OfonoConnectionManagerGetContexts, State::CallFinished))})},
          {ConnectionContextAdded,
           new AutomatorScript(
               {AutomatorScript::emptyState,
                AutomatorScript::Item(State(State::OfonoConnectionContextGetProperties, State::CallStarted)),
                //           AutomatorScript::Item(State(State::OfonoConnectionManagerContextAdded, State::Signal)),
                AutomatorScript::Item(State(State::OfonoConnectionContextGetProperties, State::CallFinished))})},
          {ConnectionContextAccessPointName,
           new AutomatorScript(
               {AutomatorScript::emptyState,
                AutomatorScript::Item(State(State::OfonoConnectionContextAccessPointName, State::CallStarted)),
                AutomatorScript::Item(State(State::OfonoConnectionContextAccessPointName, State::CallFinished)),
                AutomatorScript::Item(State(State::OfonoConnectionContextAccessPointName, State::Signal))})},
          {ConnectionContextUsername,
           new AutomatorScript(
               {AutomatorScript::emptyState,
                AutomatorScript::Item(State(State::OfonoConnectionContextUsername, State::CallStarted)),
                AutomatorScript::Item(State(State::OfonoConnectionContextUsername, State::CallFinished)),
                AutomatorScript::Item(State(State::OfonoConnectionContextUsername, State::Signal))})},
          {ConnectionContextPassword,
           new AutomatorScript(
               {AutomatorScript::emptyState,
                AutomatorScript::Item(State(State::OfonoConnectionContextPassword, State::CallStarted)),
                AutomatorScript::Item(State(State::OfonoConnectionContextPassword, State::CallFinished)),
                AutomatorScript::Item(State(State::OfonoConnectionContextPassword, State::Signal))})},
          {ConnectionContextActiveDisable,
           new AutomatorScript(
               {AutomatorScript::emptyState,
                AutomatorScript::Item(State(State::OfonoConnectionContextActive, State::CallStarted, false)),
                AutomatorScript::Item(State(State::OfonoConnectionContextActive, State::CallFinished)),
                AutomatorScript::Item(State(State::OfonoConnectionContextActive, State::Signal, false))})},
          {ConnectionContextActiveEnable,
           new AutomatorScript(
               {AutomatorScript::emptyState,
                AutomatorScript::Item(State(State::OfonoConnectionContextActive, State::CallStarted, true)),
                AutomatorScript::Item(State(State::OfonoConnectionContextActive, State::CallFinished)),
                AutomatorScript::Item(State(State::OfonoConnectionContextActive, State::Signal, true))})}}
{
  _timer->setSingleShot(true);
  _timer->setInterval(1000);
  connect(_timer.data(), &QTimer::timeout, [this]() { processing(State()); });
}

Automator::~Automator()
{
  if (!_timer.isNull())
  {
    _timer->stop();
    _timer.reset();
  }
}

void Automator::processing(const State &state)
{
  static long n = 0;
  D("|STATE|" << n++ << ":" << state);
  if (_timer->isActive())
    _timer->stop();

  bool connectionManagerDetached = false;
  bool networkRegistrationDeregistered = false;
  int scriptsSignalCount = 0;
  int scriptsRunningCount = 0;

  for (auto it = _scripts.keyValueBegin(); it != _scripts.keyValueEnd(); ++it)
  {
    switch ((*it).second->processing(state))
    {
      case State::Signal: ++scriptsSignalCount; break;
      case State::CallStarted: ++scriptsRunningCount; break;
      case State::CallFinished:
      {
        switch ((*it).first)
        {
          case ManagerModemAdded:
          {
            if (!_data.managerModemAdded && _data.modemPowered)
              _data.restartModem = true;
            _data.managerModemAdded = true;
          }
          break;
          case SimManagerAdded: _data.simManagerAdded = true; break;
          case NetworkRegistrationAdded: _data.networkRegistrationAdded = true; break;
          case ConnectionManagerAdded: _data.connectionManagerAdded = true; break;
          case ConnectionContextAdded: _data.connectionContextAdded = true; break;
          default: break;
        }
      }
      break;
      case State::CallError:
      {
        C("SCRIPT ERROR:" << toString((*it).first) << (*it).second->error().message());
        (*it).second->reset();
      }
      break;
      default: break;
    };
  }

  if (State::Signal == state.status())
  {
    switch (state.type())
    {
      case State::OfonoModemLockdown:
      {
        _data.modemLockdown = state.value().toBool();
        if (_data.modemLockdown)
        {
          _data.modemPowered = false;
          _data.modemOnline = false;
          _data.simManagerAdded = false;
          _data.networkRegistrationAdded = false;
          _data.connectionManagerAdded = false;
          _data.connectionManagerAttached = false;
          _data.connectionManagerPowered = false;
          _data.connectionContextAdded = false;
          _data.connectionContextActive = false;
          _data.simManagerCardIdentifier.clear();
          _data.simManagerServiceProviderName.clear();
          _data.networkRegistrationStatus.clear();
          for (const ScriptType type :
               {ModemPoweredDisable, ModemPoweredEnable, ModemOnlineDisable, ModemOnlineEnable, SimManagerAdded,
                NetworkRegistrationAdded, ConnectionManagerAdded, ConnectionContextAdded,
                ConnectionContextAccessPointName, ConnectionContextUsername, ConnectionContextPassword,
                ConnectionContextActiveDisable, ConnectionContextActiveEnable})
            _scripts.value(type)->reset();
        }
      }
      break;
      case State::OfonoModemPowered:
      {
        _data.modemPowered = state.value().toBool();
        if (!_data.connectionManagerPowered)
        {
          _data.modemOnline = false;
          _data.simManagerAdded = false;
          _data.networkRegistrationAdded = false;
          _data.connectionManagerAdded = false;
          _data.connectionManagerAttached = false;
          _data.connectionManagerPowered = false;
          _data.connectionContextAdded = false;
          _data.connectionContextActive = false;
          _data.simManagerCardIdentifier.clear();
          _data.simManagerServiceProviderName.clear();
          _data.networkRegistrationStatus.clear();
          _data.connectionContextAccessPointName.clear();
          _data.connectionContextUsername.clear();
          _data.connectionContextPassword.clear();
          for (const ScriptType type :
               {ModemOnlineDisable, ModemOnlineEnable, SimManagerAdded, NetworkRegistrationAdded,
                ConnectionManagerAdded, ConnectionContextAdded, ConnectionContextAccessPointName,
                ConnectionContextUsername, ConnectionContextPassword, ConnectionContextActiveDisable,
                ConnectionContextActiveEnable})
            _scripts.value(type)->reset();
        }
      }
      break;
      case State::OfonoModemOnline:
      {
        _data.modemOnline = state.value().toBool();
        if (!_data.modemOnline)
        {
          _data.networkRegistrationAdded = false;
          _data.connectionManagerAttached = false;
          _data.connectionManagerPowered = false;
          _data.connectionContextActive = false;
          //_data.simManagerServiceProviderName.clear();
          _data.networkRegistrationStatus.clear();
          for (const ScriptType type :
               {NetworkRegistrationAdded, ConnectionContextAccessPointName, ConnectionContextUsername,
                ConnectionContextPassword, ConnectionContextActiveDisable, ConnectionContextActiveEnable})
            _scripts.value(type)->reset();
        }
      }
      break;
      case State::OfonoSimManagerCardIdentifier: _data.simManagerCardIdentifier = state.value().toString(); break;
      case State::OfonoSimManagerServiceProviderName:
        _data.simManagerServiceProviderName = state.value().toString();
        break;
      case State::OfonoNetworkRegistrationStatus:
      {

        networkRegistrationDeregistered =
            ("registered" == _data.networkRegistrationStatus && "registered" != state.value().toString());
        _data.networkRegistrationStatus = state.value().toString();
        if (networkRegistrationDeregistered)
        {
          _data.connectionManagerAttached = false;
          _data.connectionContextActive = false;
          for (const ScriptType type : {ConnectionContextActiveDisable, ConnectionContextActiveEnable})
            _scripts.value(type)->reset();
        }
      }
      break;
      case State::OfonoConnectionManagerAttached:
      {
        connectionManagerDetached = (_data.connectionManagerAttached && !state.value().toBool());
        _data.connectionManagerAttached = state.value().toBool();
        if (connectionManagerDetached)
        {
          _data.connectionContextActive = false;
          for (const ScriptType type : {ConnectionContextActiveDisable, ConnectionContextActiveEnable})
            _scripts.value(type)->reset();
        }
      }
      break;
      case State::OfonoConnectionManagerPowered: _data.connectionManagerPowered = state.value().toBool(); break;
      case State::OfonoConnectionContextAccessPointName:
        _data.connectionContextAccessPointName = state.value().toString();
        break;
      case State::OfonoConnectionContextUsername: _data.connectionContextUsername = state.value().toString(); break;
      case State::OfonoConnectionContextPassword: _data.connectionContextPassword = state.value().toString(); break;
      case State::OfonoConnectionContextActive: _data.connectionContextActive = state.value().toBool(); break;
      case State::OfonoServiceUnregistered:
      case State::OfonoManagerModemRemoved:
      {
        _data.restartModem = false;
        _data.managerModemAdded = false;
        _data.modemLockdown = false;
        _data.modemPowered = false;
        _data.modemOnline = false;
        _data.simManagerAdded = false;
        _data.networkRegistrationAdded = false;
        _data.connectionManagerAdded = false;
        _data.connectionManagerAttached = false;
        _data.connectionManagerPowered = false;
        _data.connectionContextAdded = false;
        _data.connectionContextActive = false;
        _data.simManagerCardIdentifier.clear();
        _data.simManagerServiceProviderName.clear();
        _data.networkRegistrationStatus.clear();
        _data.connectionContextAccessPointName.clear();
        _data.connectionContextUsername.clear();
        _data.connectionContextPassword.clear();
        for (const ScriptType type :
             {ManagerModemAdded, ModemLockdownDisable, ModemLockdownEnable, ModemPoweredDisable, ModemPoweredEnable,
              ModemOnlineDisable, ModemOnlineEnable, SimManagerAdded, NetworkRegistrationAdded, ConnectionManagerAdded,
              ConnectionContextAdded, ConnectionContextAccessPointName, ConnectionContextUsername,
              ConnectionContextPassword, ConnectionContextActiveDisable, ConnectionContextActiveEnable})
          _scripts.value(type)->reset();
      }
      break;
      case State::OfonoModemInterfaceSimManagerRemoved:
      {
        D("XXX: State::OfonoModemInterfaceSimManagerRemoved");
        _data.simManagerAdded = false;
        _data.simManagerCardIdentifier.clear();
        _data.simManagerServiceProviderName.clear();
        _scripts.value(SimManagerAdded)->reset();
      }
      break;
      case State::OfonoModemInterfaceConnectionManagerRemoved:
      {
        D("XXX: State::OfonoModemInterfaceConnectionManagerRemoved");
        _data.connectionManagerAdded = false;
        _data.connectionManagerAttached = false;
        _data.connectionManagerPowered = false;
        _data.connectionContextAdded = false;
        _data.connectionContextActive = false;
        //        _data.connectionContextAccessPointName.clear();
        //        _data.connectionContextUsername.clear();
        //        _data.connectionContextPassword.clear();
        for (const ScriptType type : {ConnectionManagerAdded, ConnectionContextAdded, ConnectionContextAccessPointName,
                                      ConnectionContextUsername, ConnectionContextPassword,
                                      ConnectionContextActiveDisable, ConnectionContextActiveEnable})
          _scripts.value(type)->reset();
      }
      break;
      case State::OfonoModemInterfaceNetworkRegistrationRemoved:
      {
        _data.networkRegistrationAdded = false;
        _data.networkRegistrationStatus.clear();
        _scripts.value(NetworkRegistrationAdded)->reset();
      }
      break;
      case State::OfonoConnectionManagerContextRemoved:
      {
        D("XXX: State::OfonoConnectionManagerContextRemoved");
        _data.connectionManagerAttached = false;
        _data.connectionManagerPowered = false;
        _data.connectionContextAdded = false;
        _data.connectionContextActive = false;
        _data.connectionContextAccessPointName.clear();
        _data.connectionContextUsername.clear();
        _data.connectionContextPassword.clear();
        for (const ScriptType type :
             {ConnectionContextAdded, ConnectionContextAccessPointName, ConnectionContextUsername,
              ConnectionContextPassword, ConnectionContextActiveDisable, ConnectionContextActiveEnable})
          _scripts.value(type)->reset();
        break;
      }
      break;
      default: break;
    }
  }

  _data.debug();
  D(networkRegistrationDeregistered << connectionManagerDetached);

  // Autoconnection
  D(scriptsSignalCount << scriptsRunningCount);
  if (!_data.managerModemAdded || scriptsRunningCount || scriptsSignalCount)
    return;

  if (_data.restartModem)
  {
    D("--- 0 State::OfonoModemPowered, false");
    if (!_scripts.value(ModemPoweredDisable)->Signal())
    {
      _timer->start();
      return;
    }
    _data.restartModem = false;
    Q_EMIT Call(State::OfonoModemPowered, false);
    return;
  }

  if (!_data.modemPowered)
  {
    D("--- 1 State::OfonoModemPowered, true");
    if (!_scripts.value(ModemPoweredEnable)->Signal())
    {
      _timer->start();
      return;
    }
    Q_EMIT Call(State::OfonoModemPowered, true);
    return;
  }

  if (!(_data.simManagerAdded && _data.connectionManagerAdded && _data.connectionContextAdded))
    return;

  if (!_data.modemOnline)
  {
    D("--- 2 State::OfonoModemOnline, true");
    if (!_scripts.value(ModemOnlineEnable)->Signal())
    {
      _timer->start();
      return;
    }
    Q_EMIT Call(State::OfonoModemOnline, true);
    return;
  }

  if (_data.simManagerServiceProviderName.isEmpty())
    return;

  const ModemManagerData::Settings::Provider provider = _settings.providerSettings(_data.simManagerServiceProviderName);
  if (provider.accessPointName.isValid() &&
      _data.connectionContextAccessPointName != provider.accessPointName.toString())
  {
    if (!_scripts.value(ConnectionContextAccessPointName)->Signal())
    {
      _timer->start();
      return;
    }
    Q_EMIT Call(State::OfonoConnectionContextAccessPointName, provider.accessPointName.toString());
    return;
  }
  if (provider.username.isValid() && _data.connectionContextUsername != provider.username.toString())
  {
    if (!_scripts.value(ConnectionContextUsername)->Signal())
    {
      _timer->start();
      return;
    }
    Q_EMIT Call(State::OfonoConnectionContextUsername, provider.username.toString());
    return;
  }
  if (provider.password.isValid() && _data.connectionContextPassword != provider.password.toString())
  {
    if (!_scripts.value(ConnectionContextPassword)->Signal())
    {
      _timer->start();
      return;
    }
    Q_EMIT Call(State::OfonoConnectionContextPassword, provider.password.toString());
    return;
  }

  if (/*networkRegistrationDeregistered ||*/ connectionManagerDetached)
  {
    D("networkRegistrationDeregistered || connectionManagerDetached" << networkRegistrationDeregistered
                                                                     << connectionManagerDetached);
    if (!_scripts.value(ModemPoweredDisable)->Signal())
    {
      _timer->start();
      return;
    }
    Q_EMIT Call(State::OfonoModemPowered, false);
    return;
  }

  if ("registered" == _data.networkRegistrationStatus && _data.connectionManagerAttached &&
      !_data.connectionContextActive)
  {
    D("--- command: Call(State::OfonoConnectionContextActive, true)");
    if (!_scripts.value(ConnectionContextActiveEnable)->Signal())
    {
      _timer->start();
      return;
    }
    Q_EMIT Call(State::OfonoConnectionContextActive, true);
    return;
  }
}

void Automator::debugScriptsRunning()
{
  D("----------------------------------------------------------");
  for (auto &item : _scripts)
    D(*item);
}

void Automator::Data::debug()
{
  D("----------------------------------------------------------");
  D("+++ restartModem                :" << restartModem);
  D("+++ managerModemAdded           :" << managerModemAdded);
  D("+++ simManagerAdded             :" << simManagerAdded);
  D("+++ networkRegistrationAdded    :" << networkRegistrationAdded);
  D("+++ connectionManagerAdded      :" << connectionManagerAdded);
  D("+++ connectionContextAdded      :" << connectionContextAdded);
  D("ModemLockdown                   : " << modemLockdown);
  D("ModemPowered                    : " << modemPowered);
  D("ModemOnline                     : " << modemOnline);
  D("SimManagerCardIdentifier        : " << simManagerCardIdentifier);
  D("SimManagerServiceProviderName   :" << simManagerServiceProviderName);
  D("NetworkRegistrationStatus       :" << networkRegistrationStatus);
  D("ConnectionManagerAttached       :" << connectionManagerAttached);
  D("ConnectionManagerPowered        :" << connectionManagerPowered);
  D("ConnectionContextAccessPointName:" << connectionContextAccessPointName);
  D("ConnectionContextUsername       :" << connectionContextUsername);
  D("ConnectionContextPassword       :" << connectionContextPassword);
  D("ConnectionContextActive         :" << connectionContextActive);
  //  D("ConnectionContextInterface      :" << connectionContextInterface);
  //  D("ConnectionContextMethod         :" << connectionContextMethod);
  //  D("ConnectionContextAddress        :" << connectionContextAddress);
  //  D("ConnectionContextNetmask        :" << connectionContextNetmask);
  D("----------------------------------------------------------");
}
