#include "Automator.h"
#include "adapters/Modem.h"

Automator::Automator(const ModemManagerData::Settings &settings, QObject *parent)
: QObject(parent), _settings(settings), _timer(new QTimer()), _scripts{
    { AutomatorScript::ManagerModemAdded,
      new AutomatorScript(AutomatorScript::ManagerModemAdded,
                          { AutomatorScript::emptyState,
                            AutomatorScript::Item(State(State::OfonoModemGetProperties, State::CallStarted)),
                            AutomatorScript::Item(State(State::OfonoModemGetProperties, State::CallFinished)) },
                          this) },
    { AutomatorScript::ModemLockdownDisable,
      new AutomatorScript(AutomatorScript::ModemLockdownDisable,
                          { AutomatorScript::emptyState,
                            AutomatorScript::Item(State(State::OfonoModemLockdown, State::CallStarted, false)),
                            AutomatorScript::Item(State(State::OfonoModemLockdown, State::CallFinished)),
                            AutomatorScript::Item(State(State::OfonoModemLockdown, State::Signal, false)) },
                          this) },
    { AutomatorScript::ModemLockdownEnable,
      new AutomatorScript(AutomatorScript::ModemLockdownEnable,
                          { AutomatorScript::emptyState,
                            AutomatorScript::Item(State(State::OfonoModemLockdown, State::CallStarted, true)),
                            AutomatorScript::Item(State(State::OfonoModemLockdown, State::CallFinished)),
                            AutomatorScript::Item(State(State::OfonoModemLockdown, State::Signal, true)) },
                          this) },

    { AutomatorScript::ModemPoweredDisable,
      new AutomatorScript(AutomatorScript::ModemPoweredDisable,
                          { AutomatorScript::emptyState,
                            AutomatorScript::Item(State(State::OfonoModemPowered, State::CallStarted, false)),
                            AutomatorScript::Item(State(State::OfonoModemPowered, State::CallFinished)),
                            AutomatorScript::Item(State(State::OfonoModemPowered, State::Signal, false)) },
                          this) },
    { AutomatorScript::ModemPoweredEnable,
      new AutomatorScript(AutomatorScript::ModemPoweredEnable,
                          { AutomatorScript::emptyState,
                            AutomatorScript::Item(State(State::OfonoModemPowered, State::CallStarted, true)),
                            AutomatorScript::Item(State(State::OfonoModemPowered, State::CallFinished)),
                            AutomatorScript::Item(State(State::OfonoModemPowered, State::Signal, true)) },
                          this) },
    { AutomatorScript::ModemOnlineDisable,
      new AutomatorScript(AutomatorScript::ModemOnlineDisable,
                          { AutomatorScript::emptyState,
                            AutomatorScript::Item(State(State::OfonoModemOnline, State::CallStarted, false)),
                            AutomatorScript::Item(State(State::OfonoModemOnline, State::CallFinished)),
                            AutomatorScript::Item(State(State::OfonoModemOnline, State::Signal, false)) },
                          this) },
    { AutomatorScript::ModemOnlineEnable,
      new AutomatorScript(AutomatorScript::ModemOnlineEnable,
                          { AutomatorScript::emptyState,
                            AutomatorScript::Item(State(State::OfonoModemOnline, State::CallStarted, true)),
                            AutomatorScript::Item(State(State::OfonoModemOnline, State::CallFinished)),
                            AutomatorScript::Item(State(State::OfonoModemOnline, State::Signal, true)) },
                          this) },
    { AutomatorScript::SimManagerAdded,
      new AutomatorScript(AutomatorScript::SimManagerAdded,
                          { AutomatorScript::emptyState,
                            AutomatorScript::Item(State(State::OfonoSimManagerGetProperties, State::CallStarted)),
                            AutomatorScript::Item(State(State::OfonoSimManagerGetProperties, State::CallFinished)) },
                          this) },
    { AutomatorScript::NetworkRegistrationAdded,
      new AutomatorScript(
      AutomatorScript::NetworkRegistrationAdded,
      { AutomatorScript::emptyState,
        AutomatorScript::Item(State(State::OfonoNetworkRegistrationGetProperties, State::CallStarted)),
        AutomatorScript::Item(State(State::OfonoNetworkRegistrationGetProperties, State::CallFinished)) },
      this) },
    { AutomatorScript::ConnectionManagerAdded,
      new AutomatorScript(
      AutomatorScript::ConnectionManagerAdded,
      { AutomatorScript::emptyState,
        AutomatorScript::Item(State(State::OfonoConnectionManagerGetProperties, State::CallStarted)),
        AutomatorScript::Item(State(State::OfonoConnectionManagerGetProperties, State::CallFinished)),
        AutomatorScript::Item(State(State::OfonoConnectionManagerGetContexts, State::CallStarted)),
        AutomatorScript::Item(State(State::OfonoConnectionManagerGetContexts, State::CallFinished)) },
      this) },
    { AutomatorScript::ConnectionContextAdded,
      new AutomatorScript(
      AutomatorScript::ConnectionContextAdded,
      { AutomatorScript::emptyState,
        AutomatorScript::Item(State(State::OfonoConnectionContextGetProperties, State::CallStarted)),
        //           AutomatorScript::Item(State(State::OfonoConnectionManagerContextAdded, State::Signal)),
        AutomatorScript::Item(State(State::OfonoConnectionContextGetProperties, State::CallFinished)) },
      this) },
    { AutomatorScript::ConnectionContextAccessPointName,
      new AutomatorScript(
      AutomatorScript::ConnectionContextAccessPointName,
      { AutomatorScript::emptyState,
        AutomatorScript::Item(State(State::OfonoConnectionContextAccessPointName, State::CallStarted)),
        AutomatorScript::Item(State(State::OfonoConnectionContextAccessPointName, State::CallFinished)),
        AutomatorScript::Item(State(State::OfonoConnectionContextAccessPointName, State::Signal)) },
      this) },
    { AutomatorScript::ConnectionContextUsername,
      new AutomatorScript(AutomatorScript::ConnectionContextUsername,
                          { AutomatorScript::emptyState,
                            AutomatorScript::Item(State(State::OfonoConnectionContextUsername, State::CallStarted)),
                            AutomatorScript::Item(State(State::OfonoConnectionContextUsername, State::CallFinished)),
                            AutomatorScript::Item(State(State::OfonoConnectionContextUsername, State::Signal)) },
                          this) },
    { AutomatorScript::ConnectionContextPassword,
      new AutomatorScript(AutomatorScript::ConnectionContextPassword,
                          { AutomatorScript::emptyState,
                            AutomatorScript::Item(State(State::OfonoConnectionContextPassword, State::CallStarted)),
                            AutomatorScript::Item(State(State::OfonoConnectionContextPassword, State::CallFinished)),
                            AutomatorScript::Item(State(State::OfonoConnectionContextPassword, State::Signal)) },
                          this) },
    { AutomatorScript::ConnectionContextActiveDisable,
      new AutomatorScript(
      AutomatorScript::ConnectionContextActiveDisable,
      { AutomatorScript::emptyState,
        AutomatorScript::Item(State(State::OfonoConnectionContextActive, State::CallStarted, false)),
        AutomatorScript::Item(State(State::OfonoConnectionContextActive, State::CallFinished)),
        AutomatorScript::Item(State(State::OfonoConnectionContextActive, State::Signal, false)) },
      this) },
    { AutomatorScript::ConnectionContextActiveEnable,
      new AutomatorScript(
      AutomatorScript::ConnectionContextActiveEnable,
      { AutomatorScript::emptyState,
        AutomatorScript::Item(State(State::OfonoConnectionContextActive, State::CallStarted, true)),
        AutomatorScript::Item(State(State::OfonoConnectionContextActive, State::CallFinished)),
        AutomatorScript::Item(State(State::OfonoConnectionContextActive, State::Signal, true)) },
      this) }
  }
{
  _timer->setSingleShot(false);
  connect(_timer.data(), &QTimer::timeout,
          [this]() { automatorScriptStatusChanged(State::_EMPTYSTATUS_, State()); });
  for (AutomatorScript *item : _scripts.values())
  {
    connect(item, &AutomatorScript::StatusChanged, this, &Automator::automatorScriptStatusChanged);
  }
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
      _scripts.value(AutomatorScript::ManagerModemAdded)->reset();
      _scripts.value(AutomatorScript::ModemLockdownDisable)->reset();
      _scripts.value(AutomatorScript::ModemLockdownEnable)->reset();
      _scripts.value(AutomatorScript::ModemPoweredDisable)->reset();
      _scripts.value(AutomatorScript::ModemPoweredEnable)->reset();
      _scripts.value(AutomatorScript::ModemOnlineDisable)->reset();
      _scripts.value(AutomatorScript::ModemOnlineEnable)->reset();
    }
      [[clang::fallthrough]];
    case State::OfonoModemInterfaceSimManagerRemoved:
    {
      _data.simManagerCardIdentifier.clear();
      _data.simManagerServiceProviderName.clear();
      _scripts.value(AutomatorScript::SimManagerAdded)->reset();
    }
      [[clang::fallthrough]];
    case State::OfonoModemInterfaceNetworkRegistrationRemoved:
    {
      _data.networkRegistrationStatus.clear();
      _scripts.value(AutomatorScript::NetworkRegistrationAdded)->reset();
    }
      [[clang::fallthrough]];
    case State::OfonoModemInterfaceConnectionManagerRemoved:
    {
      _data.connectionManagerAttached.clear();
      _data.connectionManagerPowered.clear();
      _scripts.value(AutomatorScript::ConnectionManagerAdded)->reset();
    }
      [[clang::fallthrough]];
    case State::OfonoConnectionManagerContextRemoved:
    {
      _data.connectionContextAccessPointName.clear();
      _data.connectionContextUsername.clear();
      _data.connectionContextPassword.clear();
      _data.connectionContextActive.clear();
      _scripts.value(AutomatorScript::ConnectionContextAdded)->reset();
      _scripts.value(AutomatorScript::ConnectionContextAccessPointName)->reset();
      _scripts.value(AutomatorScript::ConnectionContextUsername)->reset();
      _scripts.value(AutomatorScript::ConnectionContextPassword)->reset();
      _scripts.value(AutomatorScript::ConnectionContextActiveDisable)->reset();
      _scripts.value(AutomatorScript::ConnectionContextActiveEnable)->reset();
      automatorScriptStatusChanged(State::Signal, state);
    }
    break;
    default:
      break;
    }
  }

  for (AutomatorScript *item : _scripts.values())
  {
    item->processing(sender, state, _data);
  }
}

void Automator::automatorScriptStatusChanged(const State::Status status, const State &state)
{
  AutomatorScript::Type type = AutomatorScript::_EMPTYTYPE_;
  if (nullptr == sender() && State::Signal == status)
  {
    D("| --- SIGNAL --- |" << state);
  }
  else
  {
    AutomatorScript *script = static_cast<AutomatorScript *>(sender());
    type = script->type();
    D("/ --- SCRIPT --- /") << *script;
    switch (status)
    {
    case State::CallStarted:
    {
      return;
    }
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

  if (_timer->isActive()) return;
  for (const AutomatorScript *item : _scripts.values())
  {
    if (State::CallStarted == item->status()) return;
  }

  // Autoconnection
  autoConnection(status, type, state);
}

void Automator::autoConnection(const State::Status status, const AutomatorScript::Type type, const State &state)
{
#warning Добавить сюда логику по автоподключению + запуск таймера
  if (AutomatorScript::_EMPTYTYPE_ == type)
  {
    DF() << "SIGNAL, NOT SCRIPT" <<status << state;
  }
  else
    DF() << status << *_scripts.value(type) << state;
}

void Automator::debugScriptsRunning()
{
  for (auto &item : _scripts.values())
    D(*item);
}
