#include "Automator.h"
#include "adapters/Modem.h"

Automator::Automator(const ModemManagerData::Settings &settings, QObject *parent)
: QObject(parent), _settings(settings), _networkRegistrationTimer(new QTimer()),
  _managerModemRemoved({ AutomatorScript::Item(State(State::OfonoManagerModemRemoved, State::Signal)) }, this),
  _managerModemAdded({ AutomatorScript::emptyState,
                       AutomatorScript::Item(State(State::OfonoModemGetProperties, State::CallStarted)),
                       AutomatorScript::Item(State(State::OfonoModemGetProperties, State::CallFinished)) },
                     this),
  _modemLockdownSignal({ AutomatorScript::Item(State(State::OfonoModemLockdown, State::Signal)) }, this),
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
  _modemPoweredSignal({ AutomatorScript::Item(State(State::OfonoModemPowered, State::Signal)) }, this),
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
  _modemOnlineSignal({ AutomatorScript::Item(State(State::OfonoModemOnline, State::Signal)) }, this),
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
  _simManagerRemoved({ AutomatorScript::Item(State(State::OfonoModemInterfaceSimManagerRemoved, State::Signal)) },
                     this),
  _simManagerAdded({ AutomatorScript::emptyState,
                     AutomatorScript::Item(State(State::OfonoSimManagerGetProperties, State::CallStarted)),
                     AutomatorScript::Item(State(State::OfonoSimManagerGetProperties, State::CallFinished)) },
                   this),
  _networkRegistrationRemoved(
  { AutomatorScript::Item(State(State::OfonoModemInterfaceNetworkRegistrationRemoved, State::Signal)) }, this),
  _networkRegistrationAdded(
  { AutomatorScript::emptyState,
    AutomatorScript::Item(State(State::OfonoNetworkRegistrationGetProperties, State::CallStarted)),
    AutomatorScript::Item(State(State::OfonoNetworkRegistrationGetProperties, State::CallFinished)) },
  this),
  _connectionManagerRemoved({ AutomatorScript::Item(
                            State(State::OfonoModemInterfaceConnectionManagerRemoved, State::Signal)) },
                            this),
  _connectionManagerAdded(
  { AutomatorScript::emptyState,
    AutomatorScript::Item(State(State::OfonoConnectionManagerGetProperties, State::CallStarted)),
    AutomatorScript::Item(State(State::OfonoConnectionManagerGetProperties, State::CallFinished)),
    AutomatorScript::Item(State(State::OfonoConnectionManagerGetContexts, State::CallStarted)),
    AutomatorScript::Item(State(State::OfonoConnectionManagerGetContexts, State::CallFinished)) },
  this),
  _connectionContextRemoved(
  { AutomatorScript::Item(State(State::OfonoConnectionManagerContextRemoved, State::Signal)) }, this),
  _connectionContextAdded(
  { AutomatorScript::emptyState,
    AutomatorScript::Item(State(State::OfonoConnectionContextGetProperties, State::CallStarted)),
    //           AutomatorScript::Item(State(State::OfonoConnectionManagerContextAdded, State::Signal)),
    AutomatorScript::Item(State(State::OfonoConnectionContextGetProperties, State::CallFinished)) },
  this),
  _connectionContextAccessPointNameSignal(
  { AutomatorScript::Item(State(State::OfonoConnectionContextAccessPointName, State::Signal)) }, this),
  _connectionContextAccessPointName(
  { AutomatorScript::emptyState,
    AutomatorScript::Item(State(State::OfonoConnectionContextAccessPointName, State::CallStarted)),
    AutomatorScript::Item(State(State::OfonoConnectionContextAccessPointName, State::CallFinished)),
    AutomatorScript::Item(State(State::OfonoConnectionContextAccessPointName, State::Signal)) },
  this),
  _connectionContextUsernameSignal(
  { AutomatorScript::Item(State(State::OfonoConnectionContextUsername, State::Signal)) }, this),
  _connectionContextUsername(
  { AutomatorScript::emptyState,
    AutomatorScript::Item(State(State::OfonoConnectionContextUsername, State::CallStarted)),
    AutomatorScript::Item(State(State::OfonoConnectionContextUsername, State::CallFinished)),
    AutomatorScript::Item(State(State::OfonoConnectionContextUsername, State::Signal)) },
  this),
  _connectionContextPasswordSignal(
  { AutomatorScript::Item(State(State::OfonoConnectionContextPassword, State::Signal)) }, this),
  _connectionContextPassword(
  { AutomatorScript::emptyState,
    AutomatorScript::Item(State(State::OfonoConnectionContextPassword, State::CallStarted)),
    AutomatorScript::Item(State(State::OfonoConnectionContextPassword, State::CallFinished)),
    AutomatorScript::Item(State(State::OfonoConnectionContextPassword, State::Signal)) },
  this),
  _connectionContextActiveSignal({ AutomatorScript::Item(State(State::OfonoConnectionContextActive, State::Signal)) },
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

  connect(&_managerModemRemoved, &AutomatorScript::StatusChanged, this, &Automator::automatorScriptStatusChanged);
  connect(&_managerModemAdded, &AutomatorScript::StatusChanged, this, &Automator::automatorScriptStatusChanged);
  connect(&_modemLockdownSignal, &AutomatorScript::StatusChanged, this, &Automator::automatorScriptStatusChanged);
  connect(&_modemLockdownDisable, &AutomatorScript::StatusChanged, this, &Automator::automatorScriptStatusChanged);
  connect(&_modemLockdownEnable, &AutomatorScript::StatusChanged, this, &Automator::automatorScriptStatusChanged);
  connect(&_modemPoweredSignal, &AutomatorScript::StatusChanged, this, &Automator::automatorScriptStatusChanged);
  connect(&_modemPoweredDisable, &AutomatorScript::StatusChanged, this, &Automator::automatorScriptStatusChanged);
  connect(&_modemPoweredEnable, &AutomatorScript::StatusChanged, this, &Automator::automatorScriptStatusChanged);
  connect(&_modemOnlineSignal, &AutomatorScript::StatusChanged, this, &Automator::automatorScriptStatusChanged);
  connect(&_modemOnlineDisable, &AutomatorScript::StatusChanged, this, &Automator::automatorScriptStatusChanged);
  connect(&_modemOnlineEnable, &AutomatorScript::StatusChanged, this, &Automator::automatorScriptStatusChanged);
  connect(&_simManagerRemoved, &AutomatorScript::StatusChanged, this, &Automator::automatorScriptStatusChanged);
  connect(&_simManagerAdded, &AutomatorScript::StatusChanged, this, &Automator::automatorScriptStatusChanged);
  connect(&_networkRegistrationRemoved, &AutomatorScript::StatusChanged, this,
          &Automator::automatorScriptStatusChanged);
  connect(&_networkRegistrationAdded, &AutomatorScript::StatusChanged, this, &Automator::automatorScriptStatusChanged);
  connect(&_connectionManagerRemoved, &AutomatorScript::StatusChanged, this, &Automator::automatorScriptStatusChanged);
  connect(&_connectionManagerAdded, &AutomatorScript::StatusChanged, this, &Automator::automatorScriptStatusChanged);
  connect(&_connectionContextRemoved, &AutomatorScript::StatusChanged, this, &Automator::automatorScriptStatusChanged);
  connect(&_connectionContextAdded, &AutomatorScript::StatusChanged, this, &Automator::automatorScriptStatusChanged);
  connect(&_connectionContextAccessPointNameSignal, &AutomatorScript::StatusChanged, this,
          &Automator::automatorScriptStatusChanged);
  connect(&_connectionContextAccessPointName, &AutomatorScript::StatusChanged, this,
          &Automator::automatorScriptStatusChanged);
  connect(&_connectionContextUsernameSignal, &AutomatorScript::StatusChanged, this,
          &Automator::automatorScriptStatusChanged);
  connect(&_connectionContextUsername, &AutomatorScript::StatusChanged, this,
          &Automator::automatorScriptStatusChanged);
  connect(&_connectionContextPasswordSignal, &AutomatorScript::StatusChanged, this,
          &Automator::automatorScriptStatusChanged);
  connect(&_connectionContextPassword, &AutomatorScript::StatusChanged, this,
          &Automator::automatorScriptStatusChanged);
  connect(&_connectionContextActiveSignal, &AutomatorScript::StatusChanged, this,
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
#warning Добавить в _data.setValue сигналы для очистки данных
  if (State::Signal == state.status()) _data.setValue(state.type(), state.value());

  _managerModemRemoved.processing(sender, state, _data);
  _managerModemAdded.processing(sender, state, _data);
  _modemLockdownSignal.processing(sender, state, _data);
  _modemLockdownDisable.processing(sender, state, _data);
  _modemLockdownEnable.processing(sender, state, _data);
  _modemPoweredSignal.processing(sender, state, _data);
  _modemPoweredDisable.processing(sender, state, _data);
  _modemPoweredEnable.processing(sender, state, _data);
  _modemOnlineSignal.processing(sender, state, _data);
  _modemOnlineDisable.processing(sender, state, _data);
  _modemOnlineEnable.processing(sender, state, _data);
  _simManagerRemoved.processing(sender, state, _data);
  _simManagerAdded.processing(sender, state, _data);
  _networkRegistrationRemoved.processing(sender, state, _data);
  _networkRegistrationAdded.processing(sender, state, _data);
  _connectionManagerRemoved.processing(sender, state, _data);
  _connectionManagerAdded.processing(sender, state, _data);
  _connectionContextRemoved.processing(sender, state, _data);
  _connectionContextAdded.processing(sender, state, _data);
  _connectionContextAccessPointNameSignal.processing(sender, state, _data);
  _connectionContextAccessPointName.processing(sender, state, _data);
  _connectionContextUsernameSignal.processing(sender, state, _data);
  _connectionContextUsername.processing(sender, state, _data);
  _connectionContextPasswordSignal.processing(sender, state, _data);
  _connectionContextPassword.processing(sender, state, _data);
  _connectionContextActiveSignal.processing(sender, state, _data);
  _connectionContextActiveDisable.processing(sender, state, _data);
  _connectionContextActiveEnable.processing(sender, state, _data);
}

void Automator::automatorScriptStatusChanged(const State::Status status, const QDBusError &error)
{
  const QMap<State::Status, QString> statuses = { { State::Status::Signal, "Signal" },
                                                  { State::Status::CallStarted, "CallStarted" },
                                                  { State::Status::CallFinished, "CallFinished" },
                                                  { State::Status::CallError, "CallError" } };

  AutomatorScript *script = static_cast<AutomatorScript *>(sender());
  D("+--- ScriptStatusChanged ---+") << script << statuses.value(status) << error.name();
   _data.debug();
  switch (status)
  {
  case State::Signal:
  {
#warning Добавить сброс скриптов в начальное состояние
  }
  break;
  case State::CallStarted:
  {
  }
  break;
  case State::CallFinished:
  {
#warning need reset before call
    script->reset();
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
