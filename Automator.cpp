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
  //  connect(_timer.data(), &QTimer::timeout, [this]() { processing(State()); });
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
  bool connectionManagerDetached = false;
  bool networkRegistrationDeregistered = false;
  int scriptsSignalCount = 0;
  int scriptsRunningCount = 0;
  int scriptsErrorCount = 0;
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
            if (_data.modemPowered)
              _data.restartModem = true;
            _data.managerModemExist = true;
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
        ++scriptsErrorCount;
        QDBusError error = (*it).second->error();
        W("+++ ScriptError:" << toString((*it).first) << error.name() << error.message());
        (*it).second->reset();
      }
      break;
      default: break;
    };
  }
  //  debugScriptsRunning();

  switch (state.status())
  {
    case State::Signal:
    {
      switch (state.type())
      {
        case State::OfonoServiceUnregistered:
        case State::OfonoManagerModemRemoved:
        {
          _scripts.value(ManagerModemAdded)->reset();
          _scripts.value(ModemLockdownDisable)->reset();
          _scripts.value(ModemLockdownEnable)->reset();
          _scripts.value(ModemPoweredDisable)->reset();
          _scripts.value(ModemPoweredEnable)->reset();
          _scripts.value(ModemOnlineDisable)->reset();
          _scripts.value(ModemOnlineEnable)->reset();
          _scripts.value(SimManagerAdded)->reset();
          _scripts.value(NetworkRegistrationAdded)->reset();
          _scripts.value(ConnectionManagerAdded)->reset();
          _scripts.value(ConnectionContextAdded)->reset();
          _scripts.value(ConnectionContextAccessPointName)->reset();
          _scripts.value(ConnectionContextUsername)->reset();
          _scripts.value(ConnectionContextPassword)->reset();
          _scripts.value(ConnectionContextActiveDisable)->reset();
          _scripts.value(ConnectionContextActiveEnable)->reset();
          _data.restartModem = false;
          _data.managerModemExist = false;
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
        }
        break;
        case State::OfonoModemInterfaceSimManagerRemoved:
        {
          _scripts.value(SimManagerAdded)->reset();
          _data.simManagerAdded = false;
          _data.simManagerCardIdentifier.clear();
          _data.simManagerServiceProviderName.clear();
        }
        break;
        case State::OfonoModemInterfaceConnectionManagerRemoved:
        {
          _scripts.value(ConnectionManagerAdded)->reset();
          _scripts.value(ConnectionContextAdded)->reset();
          _scripts.value(ConnectionContextAccessPointName)->reset();
          _scripts.value(ConnectionContextUsername)->reset();
          _scripts.value(ConnectionContextPassword)->reset();
          _scripts.value(ConnectionContextActiveDisable)->reset();
          _scripts.value(ConnectionContextActiveEnable)->reset();
          _data.connectionManagerAdded = false;
          _data.connectionManagerAttached = false;
          _data.connectionManagerPowered = false;
          _data.connectionContextAdded = false;
          _data.connectionContextActive = false;
        }
        break;
        case State::OfonoModemInterfaceNetworkRegistrationRemoved:
        {
          _scripts.value(NetworkRegistrationAdded)->reset();
          _data.networkRegistrationAdded = false;
          _data.networkRegistrationStatus.clear();
        }
        break;
        case State::OfonoConnectionManagerContextRemoved:
        {
          _scripts.value(ConnectionContextAdded)->reset();
          _scripts.value(ConnectionContextAccessPointName)->reset();
          _scripts.value(ConnectionContextUsername)->reset();
          _scripts.value(ConnectionContextPassword)->reset();
          _scripts.value(ConnectionContextActiveDisable)->reset();
          _scripts.value(ConnectionContextActiveEnable)->reset();
          _data.connectionManagerAttached = false;
          _data.connectionManagerPowered = false;
          _data.connectionContextAdded = false;
          _data.connectionContextActive = false;
          _data.connectionContextAccessPointName.clear();
          _data.connectionContextUsername.clear();
          _data.connectionContextPassword.clear();
        }
        break;
        case State::OfonoModemLockdown:
        {
          _data.modemLockdown = state.value().toBool();
          if (_data.modemLockdown)
          {
            _scripts.value(ModemLockdownDisable)->reset();
            _scripts.value(ModemLockdownEnable)->reset();
            _scripts.value(ModemPoweredDisable)->reset();
            _scripts.value(ModemPoweredEnable)->reset();
            _scripts.value(ModemOnlineDisable)->reset();
            _scripts.value(ModemOnlineEnable)->reset();
            _scripts.value(SimManagerAdded)->reset();
            _scripts.value(NetworkRegistrationAdded)->reset();
            _scripts.value(ConnectionManagerAdded)->reset();
            _scripts.value(ConnectionContextAdded)->reset();
            _scripts.value(ConnectionContextAccessPointName)->reset();
            _scripts.value(ConnectionContextUsername)->reset();
            _scripts.value(ConnectionContextPassword)->reset();
            _scripts.value(ConnectionContextActiveDisable)->reset();
            _scripts.value(ConnectionContextActiveEnable)->reset();
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
          }
        }
        break;
        case State::OfonoModemPowered:
        {
          D("State::OfonoModemPowered:" << state.value().toBool());
          _data.modemPowered = state.value().toBool();
          if (!_data.connectionManagerPowered)
          {
            _scripts.value(ModemPoweredDisable)->reset();
            _scripts.value(ModemPoweredEnable)->reset();
            _scripts.value(ModemOnlineDisable)->reset();
            _scripts.value(ModemOnlineEnable)->reset();
            _scripts.value(SimManagerAdded)->reset();
            _scripts.value(NetworkRegistrationAdded)->reset();
            _scripts.value(ConnectionManagerAdded)->reset();
            _scripts.value(ConnectionContextAdded)->reset();
            _scripts.value(ConnectionContextAccessPointName)->reset();
            _scripts.value(ConnectionContextUsername)->reset();
            _scripts.value(ConnectionContextPassword)->reset();
            _scripts.value(ConnectionContextActiveDisable)->reset();
            _scripts.value(ConnectionContextActiveEnable)->reset();
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
          }
        }
        break;
        case State::OfonoModemOnline:
        {
          _data.modemOnline = state.value().toBool();
          if (!_data.modemOnline)
          {
            _scripts.value(NetworkRegistrationAdded)->reset();
            _scripts.value(ConnectionContextActiveDisable)->reset();
            _scripts.value(ConnectionContextActiveEnable)->reset();
            _data.networkRegistrationAdded = false;
            _data.connectionManagerAttached = false;
            _data.connectionManagerPowered = false;
            _data.connectionContextActive = false;
            //_data.simManagerServiceProviderName.clear();
          }
        }
        break;
        case State::OfonoSimManagerCardIdentifier:
        {
          _data.simManagerCardIdentifier = state.value().toString();
        }
        break;
        case State::OfonoSimManagerServiceProviderName:
        {
          _data.simManagerServiceProviderName = state.value().toString();
        }
        break;
        case State::OfonoNetworkRegistrationStatus:
        {
          networkRegistrationDeregistered =
              ("registered" == _data.networkRegistrationStatus && "registered" != state.value().toString());
          _data.networkRegistrationStatus = state.value().toString();
          if (networkRegistrationDeregistered)
          {
            _scripts.value(ConnectionContextActiveDisable)->reset();
            _scripts.value(ConnectionContextActiveEnable)->reset();
            _data.connectionManagerAttached = false;
            _data.connectionContextActive = false;
          }
        }
        break;
        case State::OfonoConnectionManagerAttached:
        {
          connectionManagerDetached = (_data.connectionManagerAttached && !state.value().toBool());
          _data.connectionManagerAttached = state.value().toBool();
          if (connectionManagerDetached)
          {
            _scripts.value(ConnectionContextActiveDisable)->reset();
            _scripts.value(ConnectionContextActiveEnable)->reset();
            _data.connectionContextActive = false;
          }
        }
        break;
        case State::OfonoConnectionManagerPowered:
        {
          _data.connectionManagerPowered = state.value().toBool();
        }
        break;
        case State::OfonoConnectionContextAccessPointName:
        {
          _data.connectionContextAccessPointName = state.value().toString();
        }
        break;
        case State::OfonoConnectionContextUsername:
        {
          _data.connectionContextUsername = state.value().toString();
        }
        break;
        case State::OfonoConnectionContextPassword:
        {
          _data.connectionContextPassword = state.value().toString();
        }
        break;
        case State::OfonoConnectionContextActive:
        {
          _data.connectionContextActive = state.value().toBool();
        }
        break;
        default: return; break;
      }
    }
    break;
    case State::CallStarted:
    {
      D("--- State::CallStarted:" << state);
    }
    break;
    case State::CallFinished:
    {
      D("--- State::CallFinished:" << state);
    }
    break;
    case State::CallError:
    {
      const QDBusError &error = state.error();
      switch (State::errorType(error))
      {
        case State::OfonoErrorType::DBusError:
        {
          switch (error.type())
          {
            // timeout
            case QDBusError::NoReply:
            case QDBusError::Timeout:
            case QDBusError::TimedOut:
            {
              D("--- State::CallError: " << error.name() << " Repeat");
              Q_EMIT Call(state.type(), state.value());
            }
            break;
            default: break;
          }
        }
        break;
        case State::OfonoErrorType::InProgress:
        case State::OfonoErrorType::Failed:
        {
          D("--- State::CallError:" << state);
        }
        break;
        default:
        {
          D("--- State::CallError:" << state);
          throw astr_global::Exception("--- State::CallError: " + state);
        }
        break;
      }
    }
    break;
    default: break;
  }

  static long n = 0;
  D("|STATE|" << n++ << ":" << state);

  // Autoconnection
  if (!_data.managerModemExist || scriptsSignalCount || scriptsRunningCount || scriptsErrorCount)
  {
    QString msg = QString("MODEM: %1|SIGNAL: %2|RUNNING: %3|ERROR: %4")
                      .arg((_data.managerModemExist ? "true" : "false"), QString::number(scriptsSignalCount),
                           QString::number(scriptsRunningCount), QString::number(scriptsErrorCount));
    if (scriptsErrorCount)
      W(msg);
    else
      D(msg);
    debugScriptsRunning();
    return;
  }

  // 1. Restart new modem
  if (_data.restartModem)
  {
    _scripts.value(ModemPoweredDisable)->Signal();
    _data.restartModem = false;
    Q_EMIT Call(State::OfonoModemPowered, false);
    return;
  }

  // 1. PowerOn modem
  if (!_data.modemPowered)
  {
    _scripts.value(ModemPoweredEnable)->Signal();
    Q_EMIT Call(State::OfonoModemPowered, true);
    return;
  }

  // 2. Wait modem interfaces
  if (!(_data.simManagerAdded && _data.connectionManagerAdded && _data.connectionContextAdded))
    return;

  // 3. Online modem
  if (!_data.modemOnline)
  {
    _scripts.value(ModemOnlineEnable)->Signal();
    Q_EMIT Call(State::OfonoModemOnline, true);
    return;
  }

  // 4. Wait sim provider
  if (_data.simManagerServiceProviderName.isEmpty())
    return;

  // 5. Configuration ConnectionContext
  const ModemManagerData::Settings::Provider provider = _settings.providerSettings(_data.simManagerServiceProviderName);
  if (provider.accessPointName.isValid() &&
      _data.connectionContextAccessPointName != provider.accessPointName.toString())
  {
    _scripts.value(ConnectionContextAccessPointName)->Signal();
    Q_EMIT Call(State::OfonoConnectionContextAccessPointName, provider.accessPointName.toString());
    return;
  }
  if (provider.username.isValid() && _data.connectionContextUsername != provider.username.toString())
  {
    _scripts.value(ConnectionContextUsername)->Signal();
    Q_EMIT Call(State::OfonoConnectionContextUsername, provider.username.toString());
    return;
  }
  if (provider.password.isValid() && _data.connectionContextPassword != provider.password.toString())
  {
    _scripts.value(ConnectionContextPassword)->Signal();
    Q_EMIT Call(State::OfonoConnectionContextPassword, provider.password.toString());
    return;
  }

  if (connectionManagerDetached)
  {
    _scripts.value(ModemPoweredDisable)->Signal();
    Q_EMIT Call(State::OfonoModemPowered, false);
    return;
  }

  if ("registered" == _data.networkRegistrationStatus && _data.connectionManagerAttached &&
      !_data.connectionContextActive)
  {
    _scripts.value(ConnectionContextActiveEnable)->Signal();
    Q_EMIT Call(State::OfonoConnectionContextActive, true);
    return;
  }
}

void Automator::debugScriptsRunning()
{
  D("----------------------------------------------------------");
  D("ManagerModemAdded                :" << *_scripts.value(ManagerModemAdded));
  D("ModemLockdownDisable             :" << *_scripts.value(ModemLockdownDisable));
  D("ModemLockdownEnable              :" << *_scripts.value(ModemLockdownEnable));
  D("ModemPoweredDisable              :" << *_scripts.value(ModemPoweredDisable));
  D("ModemPoweredEnable               :" << *_scripts.value(ModemPoweredEnable));
  D("ModemOnlineDisable               :" << *_scripts.value(ModemOnlineDisable));
  D("ModemOnlineEnable                :" << *_scripts.value(ModemOnlineEnable));
  D("SimManagerAdded                  :" << *_scripts.value(SimManagerAdded));
  D("NetworkRegistrationAdded         :" << *_scripts.value(NetworkRegistrationAdded));
  D("ConnectionManagerAdded           :" << *_scripts.value(ConnectionManagerAdded));
  D("ConnectionContextAdded           :" << *_scripts.value(ConnectionContextAdded));
  D("ConnectionContextAccessPointName :" << *_scripts.value(ConnectionContextAccessPointName));
  D("ConnectionContextUsername        :" << *_scripts.value(ConnectionContextUsername));
  D("ConnectionContextPassword        :" << *_scripts.value(ConnectionContextPassword));
  D("ConnectionContextActiveDisable   :" << *_scripts.value(ConnectionContextActiveDisable));
  D("ConnectionContextActiveEnable    :" << *_scripts.value(ConnectionContextActiveEnable));
}

void Automator::Data::debug()
{
  D("----------------------------------------------------------");
  D("+++ restartModem                :" << restartModem);
  D("+++ managerModemAdded           :" << managerModemExist);
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
