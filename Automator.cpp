#include "Automator.h"
#include "adapters/Modem.h"

Automator::Automator(const ModemManagerData::Settings &settings, QObject *parent)
: QObject(parent), _settings(settings), _networkRegistrationTimer(new QTimer()),
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
    //           AutomatorScript::Item(State(State::OfonoConnectionManagerContextAdded, State::Signal)),
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
  //  _timer->setSingleShot(false);
  //  _timer->setInterval(_settings.modemManagerimeouts.waitState);
  //  connect(_timer.data(), &QTimer::timeout, this, &Automator::onTimeout);

  connect(&_managerModemAdded, &AutomatorScript::StatusChanged, this, &Automator::automatorScriptStatusChanged);
  connect(&_modemLockdownDisable, &AutomatorScript::StatusChanged, this, &Automator::automatorScriptStatusChanged);
  connect(&_modemLockdownEnable, &AutomatorScript::StatusChanged, this, &Automator::automatorScriptStatusChanged);
  connect(&_modemPoweredDisable, &AutomatorScript::StatusChanged, this, &Automator::automatorScriptStatusChanged);
  connect(&_modemPoweredEnable, &AutomatorScript::StatusChanged, this, &Automator::automatorScriptStatusChanged);
  connect(&_modemOnlineDisable, &AutomatorScript::StatusChanged, this, &Automator::automatorScriptStatusChanged);
  connect(&_modemOnlineEnable, &AutomatorScript::StatusChanged, this, &Automator::automatorScriptStatusChanged);
  connect(&_simManagerAdded, &AutomatorScript::StatusChanged, this, &Automator::automatorScriptStatusChanged);
  connect(&_networkRegistrationAdded, &AutomatorScript::StatusChanged, this, &Automator::automatorScriptStatusChanged);
  connect(&_connectionManagerAdded, &AutomatorScript::StatusChanged, this, &Automator::automatorScriptStatusChanged);
  connect(&_connectionContextAdded, &AutomatorScript::StatusChanged, this, &Automator::automatorScriptStatusChanged);
  connect(&_connectionContextAccessPointName, &AutomatorScript::StatusChanged, this,
          &Automator::automatorScriptStatusChanged);
  connect(&_connectionContextUsername, &AutomatorScript::StatusChanged, this,
          &Automator::automatorScriptStatusChanged);
  connect(&_connectionContextPassword, &AutomatorScript::StatusChanged, this,
          &Automator::automatorScriptStatusChanged);
  connect(&_connectionContextActiveDisable, &AutomatorScript::StatusChanged, this,
          &Automator::automatorScriptStatusChanged);
  connect(&_connectionContextActiveEnable, &AutomatorScript::StatusChanged, this,
          &Automator::automatorScriptStatusChanged);
}

Automator::~Automator()
{
  if (!_networkRegistrationTimer.isNull())
  {
    _networkRegistrationTimer->stop();
    _networkRegistrationTimer.reset();
  }
}

void Automator::processing(QObject *sender, const State &state)
{

  if (State::Signal == state.status())
  {
    switch (state.type())
    {
    case State::OfonoModemLockdown:
    {
      _data.modemLockdown = state.value();
      automatorScriptStatusChanged(State::Signal, state);
    }
    break;
    case State::OfonoModemPowered:
    {
      _data.modemPowered = state.value();
      automatorScriptStatusChanged(State::Signal, state);
    }
    break;
    case State::OfonoModemOnline:
    {
      _data.modemOnline = state.value();
      automatorScriptStatusChanged(State::Signal, state);
    }
    break;

    case State::OfonoSimManagerCardIdentifier:
    {
      _data.simManagerCardIdentifier = state.value();
      automatorScriptStatusChanged(State::Signal, state);
    }
    break;
    case State::OfonoSimManagerServiceProviderName:
    {
      _data.simManagerServiceProviderName = state.value();
      automatorScriptStatusChanged(State::Signal, state);
    }
    break;

    case State::OfonoNetworkRegistrationStatus:
    {
      _data.networkRegistrationStatus = state.value();
      automatorScriptStatusChanged(State::Signal, state);
    }
    break;

    case State::OfonoConnectionManagerAttached:
    {
      _data.connectionManagerAttached = state.value();
      automatorScriptStatusChanged(State::Signal, state);
    }
    break;
    case State::OfonoConnectionManagerPowered:
    {
      _data.connectionManagerPowered = state.value();
      automatorScriptStatusChanged(State::Signal, state);
    }
    break;

    case State::OfonoConnectionContextAccessPointName:
    {
      _data.connectionContextAccessPointName = state.value();
      automatorScriptStatusChanged(State::Signal, state);
    }
    break;
    case State::OfonoConnectionContextUsername:
    {
      _data.connectionContextUsername = state.value();
      automatorScriptStatusChanged(State::Signal, state);
    }
    break;
    case State::OfonoConnectionContextPassword:
    {
      _data.connectionContextPassword = state.value();
      automatorScriptStatusChanged(State::Signal, state);
    }
    break;
    case State::OfonoConnectionContextActive:
    {
      _data.connectionContextActive = state.value();
      automatorScriptStatusChanged(State::Signal, state);
    }
    break;
    case State::OfonoServiceUnregistered:
    case State::OfonoManagerModemRemoved:
    {
      _data.modemLockdown.clear();
      _data.modemPowered.clear();
      _data.modemOnline.clear();
      _managerModemAdded.reset();
      _modemLockdownDisable.reset();
      _modemLockdownEnable.reset();
      _modemPoweredDisable.reset();
      _modemPoweredEnable.reset();
      _modemOnlineDisable.reset();
      _modemOnlineEnable.reset();
    }
      [[clang::fallthrough]];
    case State::OfonoModemInterfaceSimManagerRemoved:
    {
      _data.simManagerCardIdentifier.clear();
      _data.simManagerServiceProviderName.clear();
      _simManagerAdded.reset();
    }
      [[clang::fallthrough]];
    case State::OfonoModemInterfaceNetworkRegistrationRemoved:
    {
      _data.networkRegistrationStatus.clear();
      _networkRegistrationAdded.reset();
    }
      [[clang::fallthrough]];
    case State::OfonoModemInterfaceConnectionManagerRemoved:
    {
      _data.connectionManagerAttached.clear();
      _data.connectionManagerPowered.clear();
      _connectionManagerAdded.reset();
    }
      [[clang::fallthrough]];
    case State::OfonoConnectionManagerContextRemoved:
    {
      _data.connectionContextAccessPointName.clear();
      _data.connectionContextUsername.clear();
      _data.connectionContextPassword.clear();
      _data.connectionContextActive.clear();
      _connectionContextAdded.reset();
      _connectionContextAccessPointName.reset();
      _connectionContextUsername.reset();
      _connectionContextPassword.reset();
      _connectionContextActiveDisable.reset();
      _connectionContextActiveEnable.reset();
      automatorScriptStatusChanged(State::Signal, state);
    }
    break;
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

void Automator::automatorScriptStatusChanged(const State::Status status, const State &state)
{
  const QMap<State::Status, QString> statuses = { { State::Status::Signal, "Signal" },
                                                  { State::Status::CallStarted, "CallStarted" },
                                                  { State::Status::CallFinished, "CallFinished" },
                                                  { State::Status::CallError, "CallError" } };
  if (nullptr == sender() && State::Signal == status)
  {
    D("| --- SIGNAL --- |" << state);
  }
  else
  {
    AutomatorScript *script = static_cast<AutomatorScript *>(sender());
    D("/ --- SCRIPT --- /") << statuses.value(status) << state;
    switch (status)
    {
    case State::CallStarted:
    {
    }
    break;
    case State::CallFinished:
    {
#warning need reset before call
      script->reset();
#warning повторный вызов команды не приводит к отправке сигнала от ofono:
#warning при Online call(Online) приведет к последовательности CallStarted, CallFinished. Signal с Online == true не придет
    }
    break;
    case State::CallError:
    {
      script->reset();
    }
    break;
    default:
      break;
    }
  }
  _data.debug();
  debugScriptsRunning();
}

void Automator::debugScriptsRunning()
{
  D("--- Automator::debugScriptsRunning ---");
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
