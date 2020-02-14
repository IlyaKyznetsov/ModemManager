#include "Automator.h"
#include "adapters/Modem.h"

Automator::Automator(const ModemManagerData::Settings &settings, QObject *parent)
    : QObject(parent),
      _settings(settings),
      _timer(new QTimer()),
      _scriptsRunningCount(0),
      _isModem(false),
      _managerModemAdded({AutomatorScript::emptyState,
                          AutomatorScript::Item(State(State::OfonoModemGetProperties, State::CallStarted)),
                          AutomatorScript::Item(State(State::OfonoModemGetProperties, State::CallFinished))},
                         this),
      _modemLockdownDisable({AutomatorScript::emptyState,
                             AutomatorScript::Item(State(State::OfonoModemLockdown, State::CallStarted, false)),
                             AutomatorScript::Item(State(State::OfonoModemLockdown, State::CallFinished)),
                             AutomatorScript::Item(State(State::OfonoModemLockdown, State::Signal, false))},
                            this),
      _modemLockdownEnable({AutomatorScript::emptyState,
                            AutomatorScript::Item(State(State::OfonoModemLockdown, State::CallStarted, true)),
                            AutomatorScript::Item(State(State::OfonoModemLockdown, State::CallFinished)),
                            AutomatorScript::Item(State(State::OfonoModemLockdown, State::Signal, true))},
                           this),
      _modemPoweredDisable({AutomatorScript::emptyState,
                            AutomatorScript::Item(State(State::OfonoModemPowered, State::CallStarted, false)),
                            AutomatorScript::Item(State(State::OfonoModemPowered, State::CallFinished)),
                            AutomatorScript::Item(State(State::OfonoModemPowered, State::Signal, false))},
                           this),
      _modemPoweredEnable({AutomatorScript::emptyState,
                           AutomatorScript::Item(State(State::OfonoModemPowered, State::CallStarted, true)),
                           AutomatorScript::Item(State(State::OfonoModemPowered, State::CallFinished)),
                           AutomatorScript::Item(State(State::OfonoModemPowered, State::Signal, true))},
                          this),
      _modemOnlineDisable({AutomatorScript::emptyState,
                           AutomatorScript::Item(State(State::OfonoModemOnline, State::CallStarted, false)),
                           AutomatorScript::Item(State(State::OfonoModemOnline, State::CallFinished)),
                           AutomatorScript::Item(State(State::OfonoModemOnline, State::Signal, false))},
                          this),
      _modemOnlineEnable({AutomatorScript::emptyState,
                          AutomatorScript::Item(State(State::OfonoModemOnline, State::CallStarted, true)),
                          AutomatorScript::Item(State(State::OfonoModemOnline, State::CallFinished)),
                          AutomatorScript::Item(State(State::OfonoModemOnline, State::Signal, true))},
                         this),
      _simManagerAdded({AutomatorScript::emptyState,
                        AutomatorScript::Item(State(State::OfonoSimManagerGetProperties, State::CallStarted)),
                        AutomatorScript::Item(State(State::OfonoSimManagerGetProperties, State::CallFinished))},
                       this),
      _networkRegistrationAdded(
          {AutomatorScript::emptyState,
           AutomatorScript::Item(State(State::OfonoNetworkRegistrationGetProperties, State::CallStarted)),
           AutomatorScript::Item(State(State::OfonoNetworkRegistrationGetProperties, State::CallFinished))},
          this),
      _connectionManagerAdded(
          {AutomatorScript::emptyState,
           AutomatorScript::Item(State(State::OfonoConnectionManagerGetProperties, State::CallStarted)),
           AutomatorScript::Item(State(State::OfonoConnectionManagerGetProperties, State::CallFinished)),
           AutomatorScript::Item(State(State::OfonoConnectionManagerGetContexts, State::CallStarted)),
           AutomatorScript::Item(State(State::OfonoConnectionManagerGetContexts, State::CallFinished))},
          this),
      _connectionContextAdded(
          {AutomatorScript::emptyState,
           AutomatorScript::Item(State(State::OfonoConnectionContextGetProperties, State::CallStarted)),
           //           AutomatorScript::Item(State(State::OfonoConnectionManagerContextAdded, State::Signal)),
           AutomatorScript::Item(State(State::OfonoConnectionContextGetProperties, State::CallFinished))},
          this),
      _connectionContextAccessPointName(
          {AutomatorScript::emptyState,
           AutomatorScript::Item(State(State::OfonoConnectionContextAccessPointName, State::CallStarted)),
           AutomatorScript::Item(State(State::OfonoConnectionContextAccessPointName, State::CallFinished)),
           AutomatorScript::Item(State(State::OfonoConnectionContextAccessPointName, State::Signal))},
          this),
      _connectionContextUsername(
          {AutomatorScript::emptyState,
           AutomatorScript::Item(State(State::OfonoConnectionContextUsername, State::CallStarted)),
           AutomatorScript::Item(State(State::OfonoConnectionContextUsername, State::CallFinished)),
           AutomatorScript::Item(State(State::OfonoConnectionContextUsername, State::Signal))},
          this),
      _connectionContextPassword(
          {AutomatorScript::emptyState,
           AutomatorScript::Item(State(State::OfonoConnectionContextPassword, State::CallStarted)),
           AutomatorScript::Item(State(State::OfonoConnectionContextPassword, State::CallFinished)),
           AutomatorScript::Item(State(State::OfonoConnectionContextPassword, State::Signal))},
          this),
      _connectionContextActiveDisable(
          {AutomatorScript::emptyState,
           AutomatorScript::Item(State(State::OfonoConnectionContextActive, State::CallStarted, false)),
           AutomatorScript::Item(State(State::OfonoConnectionContextActive, State::CallFinished)),
           AutomatorScript::Item(State(State::OfonoConnectionContextActive, State::Signal, false))},
          this),
      _connectionContextActiveEnable(
          {AutomatorScript::emptyState,
           AutomatorScript::Item(State(State::OfonoConnectionContextActive, State::CallStarted, true)),
           AutomatorScript::Item(State(State::OfonoConnectionContextActive, State::CallFinished)),
           AutomatorScript::Item(State(State::OfonoConnectionContextActive, State::Signal, true))},
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
  D("--- PROCESSING ---:" << sender << state);

  //  const bool isTimerActive = _timer->isActive();
  //  _timer->stop();
  if (State::Signal == state.status())
  {
    switch (state.type())
    {
      case State::OfonoModemPowered: _data.modemPowered = state.value(); break;
      case State::OfonoModemOnline: _data.modemOnline = state.value(); break;
      case State::OfonoModemLockdown: _data.modemLockdown = state.value(); break;
      case State::OfonoSimManagerCardIdentifier: _data.simManagerCardIdentifier = state.value(); break;
      case State::OfonoSimManagerServiceProviderName: _data.simManagerServiceProviderName = state.value(); break;
      case State::OfonoNetworkRegistrationStatus: _data.networkRegistrationStatus = state.value(); break;
      case State::OfonoConnectionContextAccessPointName: _data.connectionContextAccessPointName = state.value(); break;
      case State::OfonoConnectionContextUsername: _data.connectionContextUsername = state.value(); break;
      case State::OfonoConnectionContextPassword: _data.connectionContextPassword = state.value(); break;
      case State::OfonoConnectionManagerAttached: _data.connectionManagerAttached = state.value(); break;
      case State::OfonoConnectionManagerPowered: _data.connectionManagerPowered = state.value(); break;
      case State::OfonoConnectionContextActive: _data.connectionContextActive = state.value(); break;
      case State::OfonoManagerModemRemoved:
      {
        _isModem = false;
        _timer->stop();
        _data.clearConnectionContext();
        _data.clearConnectionManager();
        _data.clearNetworkRegistration();
        _data.clearSimManager();
        _data.clearModem();
        resetConnectionContext();
        resetConnectionManager();
        resetNetworkRegistration();
        resetSimManager();
        resetModem();
        resetManager();
        return;
      }
      case State::OfonoModemInterfaceSimManagerRemoved:
      {
        _data.clearConnectionContext();
        _data.clearConnectionManager();
        _data.clearNetworkRegistration();
        _data.clearSimManager();
        resetConnectionContext();
        resetConnectionManager();
        resetNetworkRegistration();
        resetSimManager();
        return;
      }
      case State::OfonoModemInterfaceNetworkRegistrationRemoved:
      {
        _data.clearConnectionContext();
        _data.clearConnectionManager();
        _data.clearNetworkRegistration();
        resetConnectionContext();
        resetConnectionManager();
        resetNetworkRegistration();
        return;
      }
      case State::OfonoModemInterfaceConnectionManagerRemoved:
      {
        _data.clearConnectionContext();
        _data.clearConnectionManager();
        resetConnectionContext();
        resetConnectionManager();
        return;
      }
      case State::OfonoConnectionManagerContextRemoved:
      {
        _data.clearConnectionContext();
        resetConnectionContext();
        return;
      }
      default: break;
    }
  }
  //  if (isTimerActive)
  //  {
  //    _timer->start();
  //  }

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

void Automator::onStatusChanged(const AutomatorScript::ScriptStatus status, const State &state)
{
  DF();
  AutomatorScript *script = static_cast<AutomatorScript *>(sender());
  switch (status)
  {
    case AutomatorScript::Started: ++_scriptsRunningCount; break;
    case AutomatorScript::Finished:
    {
      --_scriptsRunningCount;
      script->reset();
      if (script == &_managerModemAdded)
      {
        _isModem = true;
        if (_data.modemPowered.isValid() && _data.modemPowered.toBool())
        {
          Q_EMIT Call(State::OfonoModemPowered, false);
        }
      }
    }
    break;
    case AutomatorScript::Error:
    {
      W(state.error().message());
      if (_scriptsRunningCount > 0)
        --_scriptsRunningCount;
      script->reset();
      errorHandler(state);
      return;
    }
    break;
    default: break;
  }

  if (_scriptsRunningCount < 0)
  {
    C("_scriptsRunningCount:" << _scriptsRunningCount);
    throw astr_global::Exception("scriptsRunningCount<0");
  }

  if (_scriptsRunningCount)
  {
    DF() << "STOP TIMER";
    _timer->stop();
  }
  else if (_isModem && !_timer->isActive())
  {
    DF() << "START TIMER";
    _timer->start();
  }
}

void Automator::onTimeout()
{
  DF();

  if (_scriptsRunningCount)
    return;

  // Powered
  if (!_data.modemPowered.isValid())
    return;

  if (!_data.modemPowered.toBool())
  {
    Q_EMIT Call(State::OfonoModemPowered, true);
    return;
  }

  // Online
  if (!_data.modemOnline.isValid())
    return;

  if (!_data.modemOnline.toBool())
  {
    Q_EMIT Call(State::OfonoModemOnline, true);
    return;
  }

  // ConnectionContext configuration

  if (!(_data.simManagerServiceProviderName.isValid() && _data.networkRegistrationStatus.isValid()))
    return;

  const ModemManagerData::Settings::Provider provider =
      _settings.providerSettings(_data.simManagerServiceProviderName.toString());
  if (_data.connectionContextAccessPointName.isValid() && provider.accessPointName.isValid() &&
      provider.accessPointName != _data.connectionContextAccessPointName)
  {
    Q_EMIT Call(State::OfonoConnectionContextAccessPointName, provider.accessPointName);
    return;
  }
  if (_data.connectionContextUsername.isValid() && provider.username.isValid() &&
      provider.username != _data.connectionContextUsername)
  {
    Q_EMIT Call(State::OfonoConnectionContextUsername, provider.username);
    return;
  }
  if (_data.connectionContextPassword.isValid() && provider.password.isValid() &&
      provider.password != _data.connectionContextPassword)
  {
    Q_EMIT Call(State::OfonoConnectionContextPassword, provider.password);
    return;
  }

  if (!_data.networkRegistrationStatus.isValid())
  {
    return;
  }
  else if (_data.networkRegistrationStatus.toString() == "unregistered" && _data.modemOnline.toBool())
  {
    Q_EMIT Call(State::OfonoModemOnline, false);
    //    Q_EMIT Call(State::OfonoNetworkRegistrationScan, QVariant());
    return;
  }
  else if (_data.networkRegistrationStatus.toString() != "registered")
  {
    return;
  }

  if (_data.connectionContextAccessPointName.isValid() && _data.connectionContextUsername.isValid() &&
      _data.connectionContextPassword.isValid() && _data.connectionContextActive.isValid() &&
      _data.connectionManagerAttached.isValid() && _data.connectionManagerAttached.toBool() &&
      !_data.connectionContextActive.toBool())
  {
    Q_EMIT Call(State::OfonoConnectionContextActive, true);
    return;
  }
}

void Automator::debugScriptsRunning()
{
  D("--- Automator::debugScriptsRunning ---");
  D("scriptsRunningCount:" << _scriptsRunningCount);
  D("managerModemAdded:" << _managerModemAdded.status());
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

void Automator::resetConnectionContext()
{
  if (AutomatorScript::Started == _connectionContextAdded.status())
    --_scriptsRunningCount;
  if (AutomatorScript::Started == _connectionContextAccessPointName.status())
    --_scriptsRunningCount;
  if (AutomatorScript::Started == _connectionContextUsername.status())
    --_scriptsRunningCount;
  if (AutomatorScript::Started == _connectionContextPassword.status())
    --_scriptsRunningCount;
  if (AutomatorScript::Started == _connectionContextActiveDisable.status())
    --_scriptsRunningCount;
  if (AutomatorScript::Started == _connectionContextActiveEnable.status())
    --_scriptsRunningCount;

  _connectionContextAdded.reset();
  _connectionContextAccessPointName.reset();
  _connectionContextUsername.reset();
  _connectionContextPassword.reset();
  _connectionContextActiveDisable.reset();
  _connectionContextActiveEnable.reset();

  if (_scriptsRunningCount < 0)
  {
    DF() << "Bad scriptsRunningCount:" << _scriptsRunningCount;
    throw astr_global::Exception("Bad scriptsRunningCount");
  }
}

void Automator::resetConnectionManager()
{
  DF();
  debugScriptsRunning();

  if (AutomatorScript::Started == _connectionManagerAdded.status())
    --_scriptsRunningCount;

  _connectionManagerAdded.reset();

  if (_scriptsRunningCount < 0)
  {
    DF() << "Bad scriptsRunningCount:" << _scriptsRunningCount;
    throw astr_global::Exception("Bad scriptsRunningCount");
  }
}

void Automator::resetNetworkRegistration()
{
  DF();
  debugScriptsRunning();

  if (AutomatorScript::Started == _networkRegistrationAdded.status())
    --_scriptsRunningCount;

  _networkRegistrationAdded.reset();

  if (_scriptsRunningCount < 0)
  {
    DF() << "Bad scriptsRunningCount:" << _scriptsRunningCount;
    throw astr_global::Exception("Bad scriptsRunningCount");
  }
}

void Automator::resetSimManager()
{
  DF();
  debugScriptsRunning();

  if (AutomatorScript::Started == _simManagerAdded.status())
    --_scriptsRunningCount;

  _simManagerAdded.reset();

  if (_scriptsRunningCount < 0)
  {
    DF() << "Bad scriptsRunningCount:" << _scriptsRunningCount;
    throw astr_global::Exception("Bad scriptsRunningCount");
  }
}

void Automator::resetModem()
{
  DF();
  debugScriptsRunning();

  if (AutomatorScript::Started == _modemLockdownDisable.status())
    --_scriptsRunningCount;
  if (AutomatorScript::Started == _modemLockdownEnable.status())
    --_scriptsRunningCount;
  if (AutomatorScript::Started == _modemPoweredDisable.status())
    --_scriptsRunningCount;
  if (AutomatorScript::Started == _modemPoweredEnable.status())
    --_scriptsRunningCount;
  if (AutomatorScript::Started == _modemOnlineDisable.status())
    --_scriptsRunningCount;
  if (AutomatorScript::Started == _modemOnlineEnable.status())
    --_scriptsRunningCount;

  _modemLockdownDisable.reset();
  _modemLockdownEnable.reset();
  _modemPoweredDisable.reset();
  _modemPoweredEnable.reset();
  _modemOnlineDisable.reset();
  _modemOnlineEnable.reset();

  if (_scriptsRunningCount < 0)
  {
    DF() << "Bad scriptsRunningCount:" << _scriptsRunningCount;
    throw astr_global::Exception("Bad scriptsRunningCount");
  }
}

void Automator::resetManager()
{
  DF();
  debugScriptsRunning();

  if (AutomatorScript::Started == _managerModemAdded.status())
    --_scriptsRunningCount;

  _managerModemAdded.reset();

  if (_scriptsRunningCount < 0)
  {
    DF() << "Bad scriptsRunningCount:" << _scriptsRunningCount;
    throw astr_global::Exception("Bad scriptsRunningCount");
  }
}

void Automator::errorHandler(const State &state)
{
  if (State::CallError == state.status())
  {
    switch (state.error().type())
    {
      case QDBusError::NoReply: return; break;
      default: break;
    }
    _timer->stop();
  }
}
