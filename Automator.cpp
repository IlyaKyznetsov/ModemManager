#include "Automator.h"
#include "adapters/Modem.h"

Automator::Automator(const ModemManagerData::Settings &settings, QObject *parent)
: QObject(parent), _settings(settings), _timer(new QTimer()), _scriptsRunningCount(0),
  _managerModemAdded({ AutomatorScript::emptyState,
                       AutomatorScript::Item(State(State::OfonoModemGetProperties, State::CallStarted)),
                       AutomatorScript::Item(State(State::OfonoModemGetProperties, State::CallFinished)) },
                     this),
  _modemLockdownDisable({ AutomatorScript::emptyState,
                          AutomatorScript::Item(State(State::OfonoModemLockdown, State::CallStarted, false)),
                          AutomatorScript::Item(State(State::OfonoModemLockdown, State::CallFinished)),
                          AutomatorScript::Item(State(State::OfonoModemLockdown, State::Signal, false)) },
                        this),
  _modemLockdownEnable({ AutomatorScript::emptyState,
                         AutomatorScript::Item(State(State::OfonoModemLockdown, State::CallStarted, true)),
                         AutomatorScript::Item(State(State::OfonoModemLockdown, State::CallFinished)),
                         AutomatorScript::Item(State(State::OfonoModemLockdown, State::Signal, true)) },
                       this),
  _modemPoweredDisable({ AutomatorScript::emptyState,
                         AutomatorScript::Item(State(State::OfonoModemPowered, State::CallStarted, false)),
                         AutomatorScript::Item(State(State::OfonoModemPowered, State::CallFinished)),
                         AutomatorScript::Item(State(State::OfonoModemPowered, State::Signal, false)) },
                       this),
  _modemPoweredEnable({ AutomatorScript::emptyState,
                        AutomatorScript::Item(State(State::OfonoModemPowered, State::CallStarted, true)),
                        AutomatorScript::Item(State(State::OfonoModemPowered, State::CallFinished)),
                        AutomatorScript::Item(State(State::OfonoModemPowered, State::Signal, true)) },
                      this),
  _modemOnlineDisable({ AutomatorScript::emptyState,
                        AutomatorScript::Item(State(State::OfonoModemOnline, State::CallStarted, false)),
                        AutomatorScript::Item(State(State::OfonoModemOnline, State::CallFinished)),
                        AutomatorScript::Item(State(State::OfonoModemOnline, State::Signal, false)) },
                      this),
  _modemOnlineEnable({ AutomatorScript::emptyState,
                       AutomatorScript::Item(State(State::OfonoModemOnline, State::CallStarted, true)),
                       AutomatorScript::Item(State(State::OfonoModemOnline, State::CallFinished)),
                       AutomatorScript::Item(State(State::OfonoModemOnline, State::Signal, true)) },
                     this),
  _simManagerAdded({ AutomatorScript::emptyState,
                     AutomatorScript::Item(State(State::OfonoSimManagerGetProperties, State::CallStarted)),
                     AutomatorScript::Item(State(State::OfonoSimManagerGetProperties, State::CallFinished)) },
                   this),
  _networkRegistrationAdded(
  { AutomatorScript::emptyState,
    AutomatorScript::Item(State(State::OfonoNetworkRegistrationGetProperties, State::CallStarted)),
    AutomatorScript::Item(State(State::OfonoNetworkRegistrationGetProperties, State::CallFinished)) },
  this),
  _connectionManagerAdded(
  { AutomatorScript::emptyState,
    AutomatorScript::Item(State(State::OfonoConnectionManagerGetProperties, State::CallStarted)),
    AutomatorScript::Item(State(State::OfonoConnectionManagerGetProperties, State::CallFinished)),
    AutomatorScript::Item(State(State::OfonoConnectionManagerGetContexts, State::CallStarted)),
    AutomatorScript::Item(State(State::OfonoConnectionManagerGetContexts, State::CallFinished)) },
  this),
  _connectionContextAdded(
  { AutomatorScript::emptyState,
    AutomatorScript::Item(State(State::OfonoConnectionContextGetProperties, State::CallStarted)),
    AutomatorScript::Item(State(State::OfonoConnectionManagerContextAdded, State::Signal)),
    AutomatorScript::Item(State(State::OfonoConnectionContextGetProperties, State::CallFinished)) },
  this),
  _connectionContextAccessPointName(
  { AutomatorScript::emptyState,
    AutomatorScript::Item(State(State::OfonoConnectionContextAccessPointName, State::CallStarted)),
    AutomatorScript::Item(State(State::OfonoConnectionContextAccessPointName, State::CallFinished)),
    AutomatorScript::Item(State(State::OfonoConnectionContextAccessPointName, State::Signal)) },
  this),
  _connectionContextUsername(
  { AutomatorScript::emptyState,
    AutomatorScript::Item(State(State::OfonoConnectionContextUsername, State::CallStarted)),
    AutomatorScript::Item(State(State::OfonoConnectionContextUsername, State::CallFinished)),
    AutomatorScript::Item(State(State::OfonoConnectionContextUsername, State::Signal)) },
  this),
  _connectionContextPassword(
  { AutomatorScript::emptyState,
    AutomatorScript::Item(State(State::OfonoConnectionContextPassword, State::CallStarted)),
    AutomatorScript::Item(State(State::OfonoConnectionContextPassword, State::CallFinished)),
    AutomatorScript::Item(State(State::OfonoConnectionContextPassword, State::Signal)) },
  this),
  _connectionContextActiveDisable(
  { AutomatorScript::emptyState,
    AutomatorScript::Item(State(State::OfonoConnectionContextActive, State::CallStarted, false)),
    AutomatorScript::Item(State(State::OfonoConnectionContextActive, State::CallFinished)),
    AutomatorScript::Item(State(State::OfonoConnectionContextActive, State::Signal, false)) },
  this),
  _connectionContextActiveEnable(
  { AutomatorScript::emptyState,
    AutomatorScript::Item(State(State::OfonoConnectionContextActive, State::CallStarted, true)),
    AutomatorScript::Item(State(State::OfonoConnectionContextActive, State::CallFinished)),
    AutomatorScript::Item(State(State::OfonoConnectionContextActive, State::Signal, true)) },
  this)

{
  _timer->setSingleShot(false);
  _timer->setInterval(_settings.modemManagerimeouts.waitState);
  connect(_timer.data(), &QTimer::timeout, this, &Automator::onTimeout);

  connect(&_managerModemAdded, &AutomatorScript::StatusChanged, this, &Automator::onStatusChanged);
  connect(&_modemLockdownDisable, &AutomatorScript::StatusChanged, this, &Automator::onStatusChanged);
  connect(&_modemLockdownEnable, &AutomatorScript::StatusChanged, this, &Automator::onStatusChanged);
  connect(&_modemPoweredDisable, &AutomatorScript::StatusChanged, this, &Automator::onStatusChanged);
  connect(&_modemPoweredEnable, &AutomatorScript::StatusChanged, this, &Automator::onStatusChanged);
  connect(&_modemOnlineDisable, &AutomatorScript::StatusChanged, this, &Automator::onStatusChanged);
  connect(&_modemOnlineEnable, &AutomatorScript::StatusChanged, this, &Automator::onStatusChanged);
  connect(&_simManagerAdded, &AutomatorScript::StatusChanged, this, &Automator::onStatusChanged);
  connect(&_networkRegistrationAdded, &AutomatorScript::StatusChanged, this, &Automator::onStatusChanged);
  connect(&_connectionManagerAdded, &AutomatorScript::StatusChanged, this, &Automator::onStatusChanged);
  connect(&_connectionContextAdded, &AutomatorScript::StatusChanged, this, &Automator::onStatusChanged);
  connect(&_connectionContextAccessPointName, &AutomatorScript::StatusChanged, this, &Automator::onStatusChanged);
  connect(&_connectionContextUsername, &AutomatorScript::StatusChanged, this, &Automator::onStatusChanged);
  connect(&_connectionContextPassword, &AutomatorScript::StatusChanged, this, &Automator::onStatusChanged);
  connect(&_connectionContextActiveDisable, &AutomatorScript::StatusChanged, this, &Automator::onStatusChanged);
  connect(&_connectionContextActiveEnable, &AutomatorScript::StatusChanged, this, &Automator::onStatusChanged);
}

Automator::~Automator()
{
  if (!_timer.isNull())
  {
    _timer->stop();
    _timer.reset();
  }
}

void Automator::processing(QObject *sender, const State &state)
{
  DF() << sender << state;

  if (State::Signal == state.status())
  {
    switch (state.type())
    {
    case State::OfonoModemPowered:
      _data.modemPowered = state.value();
      break;
    case State::OfonoModemOnline:
      _data.modemOnline = state.value();
      break;
    case State::OfonoModemLockdown:
      _data.modemLockdown = state.value();
      break;
    case State::OfonoSimManagerCardIdentifier:
      _data.simManagerCardIdentifier = state.value();
      break;
    case State::OfonoSimManagerServiceProviderName:
      _data.simManagerServiceProviderName = state.value();
      break;
    case State::OfonoNetworkRegistrationStatus:
      _data.networkRegistrationStatus = state.value();
      break;
    case State::OfonoConnectionContextAccessPointName:
      _data.connectionContextAccessPointName = state.value();
      break;
    case State::OfonoConnectionContextUsername:
      _data.connectionContextUsername = state.value();
      break;
    case State::OfonoConnectionContextPassword:
      _data.connectionContextPassword = state.value();
      break;
    case State::OfonoConnectionManagerAttached:
      _data.connectionManagerAttached = state.value();
      break;
    case State::OfonoConnectionManagerPowered:
      _data.connectionManagerPowered = state.value();
      break;
    case State::OfonoConnectionContextActive:
      _data.connectionContextActive = state.value();
      break;
    case State::OfonoManagerModemRemoved:
    {
      _timer->stop();
      _data.clearConnectionContext();
      _data.clearConnectionManager();
      _data.clearNetworkRegistration();
      _data.clearSimManager();
      _data.clearModem();
      return;
    }
    case State::OfonoModemInterfaceSimManagerRemoved:
    {
      _data.clearConnectionContext();
      _data.clearConnectionManager();
      _data.clearNetworkRegistration();
      _data.clearSimManager();
      return;
    }
    case State::OfonoModemInterfaceNetworkRegistrationRemoved:
    {
      _data.clearConnectionContext();
      _data.clearConnectionManager();
      _data.clearNetworkRegistration();
      return;
    }
    case State::OfonoModemInterfaceConnectionManagerRemoved:
    {
      _data.clearConnectionContext();
      _data.clearConnectionManager();
      return;
    }
    case State::OfonoConnectionManagerContextRemoved:
    {
      _data.clearConnectionContext();
      return;
    }
    default:
      break;
    }
  }

  _managerModemAdded.processing(sender, state, _data);

  _modemLockdownDisable.processing(sender, state, _data);
  _modemLockdownEnable.processing(sender, state, _data);

  _modemPoweredDisable.processing(sender, state, _data);
  _modemPoweredEnable.processing(sender, state, _data);

  _modemOnlineDisable.processing(sender, state, _data);
  _modemOnlineEnable.processing(sender, state, _data);

  _simManagerAdded.processing(sender, state, _data);

  _networkRegistrationAdded.processing(sender, state, _data);

  _connectionManagerAdded.processing(sender, state, _data);

  _connectionContextAdded.processing(sender, state, _data);
  _connectionContextAccessPointName.processing(sender, state, _data);
  _connectionContextUsername.processing(sender, state, _data);
  _connectionContextPassword.processing(sender, state, _data);

  _connectionContextActiveDisable.processing(sender, state, _data);
  _connectionContextActiveEnable.processing(sender, state, _data);
}

void Automator::onStatusChanged(const AutomatorScript::ScriptStatus status)
{
  AutomatorScript *script = static_cast<AutomatorScript *>(sender());
  switch (status)
  {
  case AutomatorScript::Started:
    ++_scriptsRunningCount;
    break;
  case AutomatorScript::Finished:
  {
    --_scriptsRunningCount;
    script->reset();
  }
  break;
  case AutomatorScript::Error:
  {
    --_scriptsRunningCount;
    C("AutomatorScript::Error");
    throw astr_global::Exception("AutomatorScript::Error");
  }
  break;
  default:
    break;
  }

  if (_scriptsRunningCount < 0)
  {
    C("_scriptsRunningCount:" << _scriptsRunningCount);
    throw astr_global::Exception("scriptsRunningCount<0");
  }

  if (_scriptsRunningCount)
    _timer->stop();
  else if (!_timer->isActive())
    _timer->start();

  if (script == &_managerModemAdded)
  {
    D("--- AutomatorScript::StatusChanged ---: ModemAdded");
    //    if (_data.modemPowered.toBool()) Q_EMIT Call(State::OfonoModemPowered, false);
    //    _timer->start();
  }
  else if (script == &_modemLockdownDisable)
  {
    D("--- AutomatorScript::StatusChanged ---: ModemLockdownDisable");
  }
  else if (script == &_modemLockdownEnable)
  {
    D("--- AutomatorScript::StatusChanged ---: ModemLockdownEnable");
  }
  else if (script == &_modemPoweredDisable)
  {
    D("--- AutomatorScript::StatusChanged ---: ModemPoweredDisable");
  }
  else if (script == &_modemPoweredEnable)
  {
    D("--- AutomatorScript::StatusChanged ---: ModemPoweredEnable");
  }
  else if (script == &_modemOnlineDisable)
  {
    D("--- AutomatorScript::StatusChanged ---: ModemOnlineDisable");
  }
  else if (script == &_modemOnlineEnable)
  {
    D("--- AutomatorScript::StatusChanged ---: ModemOnlineEnable");
  }
  else if (script == &_simManagerAdded)
  {
    D("--- AutomatorScript::StatusChanged ---: SimManagerAdded");
  }
  else if (script == &_networkRegistrationAdded)
  {
    D("--- AutomatorScript::StatusChanged ---: NetworkRegistrationAdded");
  }
  else if (script == &_connectionManagerAdded)
  {
    D("--- AutomatorScript::StatusChanged ---: ConnectionManagerAdded");
  }
  else if (script == &_connectionContextAdded)
  {
    D("--- AutomatorScript::StatusChanged ---: ConnectionContextAdded");
    /*
    if (!(_data.connectionContextAccessPointName.isValid() &&
    _data.connectionContextUsername.isValid() && _data.connectionContextPassword.isValid()))
    {
      throw astr_global::Exception("Invalid ConnectionContextAdded");
    }
    */
  }
}

void Automator::onTimeout()
{
  DF();

  debugScriptsRunning();

  if (_scriptsRunningCount) return;

  if (!(_data.modemLockdown.isValid() && _data.modemPowered.isValid() && _data.modemOnline.isValid()))
    return;

  if (!_data.modemPowered.toBool())
  {
    Q_EMIT Call(State::OfonoModemPowered, true);
    return;
  }

  if (_data.simManagerServiceProviderName.isValid())
  {
    const ModemManagerData::Settings::Provider &provider =
    _settings.providerSettings(_data.simManagerServiceProviderName.toString());

    if (provider.accessPointName.isValid() && _data.connectionContextAccessPointName.isValid() &&
        provider.accessPointName != _data.connectionContextAccessPointName)
    {
      D("--- SETTINGS --- :" << provider.accessPointName);
      Q_EMIT Call(State::OfonoConnectionContextAccessPointName, provider.accessPointName);
      return;
    }

    if (provider.username.isValid() && _data.connectionContextUsername.isValid() &&
        provider.username != _data.connectionContextUsername)
    {
      D("--- SETTINGS --- :" << provider.username);
      Q_EMIT Call(State::OfonoConnectionContextUsername, provider.username);
      return;
    }

    if (provider.password.isValid() && _data.connectionContextPassword.isValid() &&
        provider.password != _data.connectionContextPassword)
    {
      D("--- SETTINGS --- :" << provider.password);
      Q_EMIT Call(State::OfonoConnectionContextPassword, provider.password);
      return;
    }
  }

  if (!_data.modemOnline.toBool())
  {
    Q_EMIT Call(State::OfonoModemOnline, true);
    return;
  }

  if (!(_data.connectionManagerPowered.isValid() && _data.connectionManagerAttached.isValid() &&
        _data.connectionContextActive.isValid()))
    return;

  if (!_data.connectionContextActive.toBool())
  {
    Q_EMIT Call(State::OfonoConnectionContextActive, true);
    return;
  }

  D("--- AutomatorScript::StatusChanged ---: TimerTimeout");
  _data.debug();

  return;
}

void Automator::debugScriptsRunning()
{
  D("--- Automator::debugScriptsRunning ---");
  D("_managerModemAdded:" << _managerModemAdded.status());
  D("modemLockdownDisable:" << _modemLockdownDisable.status());
  D("modemLockdownEnable:" << _modemLockdownEnable.status());
  D("modemPoweredDisable:" << _modemPoweredDisable.status());
  D("modemPoweredEnable:" << _modemPoweredEnable.status());
  D("modemOnlineDisable:" << _modemOnlineDisable.status());
  D("modemOnlineEnable:" << _modemOnlineEnable.status());
  D("simManagerAdded:" << _simManagerAdded.status());
  D("networkRegistrationAdded:" << _networkRegistrationAdded.status());
  D("connectionManagerAdded:" << _connectionManagerAdded.status());
  D("connectionContextAdded:" << _connectionContextAdded.status());
  D("connectionContextAccessPointName:" << _connectionContextAccessPointName.status());
  D("connectionContextUsername:" << _connectionContextUsername.status());
  D("connectionContextPassword:" << _connectionContextPassword.status());
  D("connectionContextActiveDisable:" << _connectionContextActiveDisable.status());
  D("connectionContextActiveEnable:" << _connectionContextActiveEnable.status());
}
