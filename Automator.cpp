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
  _timer->setInterval(60 * 1000);
  connect(_timer.data(), &QTimer::timeout, [this]() {
    D("Timer timeout");
    processing(State());
  });
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
  if (State::Signal == state.status())
  {
    switch (state.type())
    {
      case State::OfonoModemLockdown: _data.modemLockdown = state.value().toBool(); break;
      case State::OfonoModemPowered: _data.modemPowered = state.value().toBool(); break;
      case State::OfonoModemOnline: _data.modemOnline = state.value().toBool(); break;
      case State::OfonoSimManagerCardIdentifier: _data.simManagerCardIdentifier = state.value().toString(); break;
      case State::OfonoSimManagerServiceProviderName:
        _data.simManagerServiceProviderName = state.value().toString();
        break;
      case State::OfonoNetworkRegistrationStatus: _data.networkRegistrationStatus = state.value().toString(); break;
      case State::OfonoConnectionManagerAttached: _data.connectionManagerAttached = state.value().toBool(); break;
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
        _data.managerModemAdded = _data.modemLockdown = _data.modemPowered = _data.modemOnline = false;
        for (const ScriptType type : {ManagerModemAdded, ModemLockdownDisable, ModemLockdownEnable, ModemPoweredDisable,
                                      ModemPoweredEnable, ModemOnlineDisable, ModemOnlineEnable})
          _scripts.value(type)->reset();
      }
      case State::OfonoModemInterfaceSimManagerRemoved:
      {
        _data.simManagerAdded = false;
        _data.simManagerCardIdentifier.clear();
        _data.simManagerServiceProviderName.clear();
        _scripts.value(SimManagerAdded)->reset();
      }
      case State::OfonoModemInterfaceNetworkRegistrationRemoved:
      {
        _data.networkRegistrationAdded = false;
        _data.networkRegistrationStatus.clear();
        _scripts.value(NetworkRegistrationAdded)->reset();
      }
      case State::OfonoModemInterfaceConnectionManagerRemoved:
      {
        _data.connectionManagerAdded = _data.connectionManagerAttached = _data.connectionManagerPowered = false;
        _scripts.value(ConnectionManagerAdded)->reset();
      }
      case State::OfonoConnectionManagerContextRemoved:
      {
        _data.connectionManagerAdded = _data.connectionContextActive = false;
        _data.connectionContextAccessPointName.clear();
        _data.connectionContextUsername.clear();
        _data.connectionContextPassword.clear();
        for (ScriptType type :
             {ConnectionContextAdded, ConnectionContextAccessPointName, ConnectionContextUsername,
              ConnectionContextPassword, ConnectionContextActiveDisable, ConnectionContextActiveEnable})
          _scripts.value(type)->reset();
      }
      break;
      default: break;
    }
  }

  int scriptsRunningCount = 0;
  for (auto it = _scripts.keyValueBegin(); it != _scripts.keyValueEnd(); ++it)
  {
    switch ((*it).second->processing(state))
    {
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
        C(toString((*it).first) << (*it).second->error().message());
        throw astr_global::Exception("SCRIPT ERROR");
      }
      break;
      default: break;
    };
  }

  // Autoconnection
  if (scriptsRunningCount)
    return;
  DF() << scriptsRunningCount;

  if (!_data.managerModemAdded)
    return;

  if (_data.restartModem)
  {
    _data.restartModem = false;
    Q_EMIT Call(State::OfonoModemPowered, false);
    _timer->start();
    return;
  }

  if (!_data.modemPowered)
  {
    Q_EMIT Call(State::OfonoModemPowered, true);
    return;
  }

  if (!(_data.simManagerAdded && _data.connectionManagerAdded && _data.connectionManagerAdded))
    return;

  if (!_data.modemOnline)
  {
    Q_EMIT Call(State::OfonoModemOnline, true);
    return;
  }

  if (_data.simManagerServiceProviderName.isEmpty() && !_data.connectionContextAdded)
    return;

  const ModemManagerData::Settings::Provider &provider =
      _settings.providerSettings(_data.simManagerServiceProviderName);
  if (provider.accessPointName.isValid() &&
      _data.connectionContextAccessPointName != provider.accessPointName.toString())
  {
    Q_EMIT Call(State::OfonoConnectionContextAccessPointName, provider.accessPointName.toString());
    return;
  }

  if (provider.username.isValid() && _data.connectionContextUsername != provider.username.toString())
  {
    Q_EMIT Call(State::OfonoConnectionContextUsername, provider.username.toString());
    return;
  }

  if (provider.password.isValid() && _data.connectionContextPassword != provider.password.toString())
  {
    Q_EMIT Call(State::OfonoConnectionContextPassword, provider.password.toString());
    return;
  }

  if ("registered" == _data.networkRegistrationStatus && _data.connectionManagerAttached &&
      !_data.connectionContextActive)
  {
    Q_EMIT Call(State::OfonoConnectionContextActive, true);
    return;
  }

  if ("registered" == _data.networkRegistrationStatus && _data.connectionManagerAttached)
  {
    _timer->stop();
  }
  else if (!_timer->isActive())
  {
    Q_EMIT Call(State::OfonoModemPowered, false);
    _timer->start();
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
  D("ModemLockdown                   : " << modemLockdown);
  D("ModemPowered                    : " << modemPowered);
  D("ModemOnline                     : " << modemOnline);
  D("SimManagerCardIdentifier        : " << simManagerCardIdentifier);
  D("SimManagerServiceProviderName   :" << simManagerServiceProviderName);
  D("NetworkRegistrationStatus       :" << networkRegistrationStatus);
  D("ConnectionContextAccessPointName:" << connectionContextAccessPointName);
  D("ConnectionContextUsername       :" << connectionContextUsername);
  D("ConnectionContextPassword       :" << connectionContextPassword);
  D("ConnectionManagerAttached       :" << connectionManagerAttached);
  D("ConnectionManagerPowered        :" << connectionManagerPowered);
  D("ConnectionContextActive         :" << connectionContextActive);
}
