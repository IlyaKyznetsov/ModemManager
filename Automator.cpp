#include "Automator.h"
#include "adapters/Modem.h"

Automator::Automator(const ModemManagerData::Settings &settings, QObject *parent)
: QObject(parent), _settings(settings), _timer(new QTimer()), _scripts{
    new AutomatorScript(AutomatorScript::ManagerModemAdded,
                        { AutomatorScript::emptyState,
                          AutomatorScript::Item(State(State::OfonoModemGetProperties, State::CallStarted)),
                          AutomatorScript::Item(State(State::OfonoModemGetProperties, State::CallFinished)) },
                        this),
    new AutomatorScript(AutomatorScript::ModemLockdownDisable,
                        { AutomatorScript::emptyState,
                          AutomatorScript::Item(State(State::OfonoModemLockdown, State::CallStarted, false)),
                          AutomatorScript::Item(State(State::OfonoModemLockdown, State::CallFinished)),
                          AutomatorScript::Item(State(State::OfonoModemLockdown, State::Signal, false)) },
                        this),
    new AutomatorScript(AutomatorScript::ModemLockdownEnable,
                        { AutomatorScript::emptyState,
                          AutomatorScript::Item(State(State::OfonoModemLockdown, State::CallStarted, true)),
                          AutomatorScript::Item(State(State::OfonoModemLockdown, State::CallFinished)),
                          AutomatorScript::Item(State(State::OfonoModemLockdown, State::Signal, true)) },
                        this),

    new AutomatorScript(AutomatorScript::ModemPoweredDisable,
                        { AutomatorScript::emptyState,
                          AutomatorScript::Item(State(State::OfonoModemPowered, State::CallStarted, false)),
                          AutomatorScript::Item(State(State::OfonoModemPowered, State::CallFinished)),
                          AutomatorScript::Item(State(State::OfonoModemPowered, State::Signal, false)) },
                        this),
    new AutomatorScript(AutomatorScript::ModemPoweredEnable,
                        { AutomatorScript::emptyState,
                          AutomatorScript::Item(State(State::OfonoModemPowered, State::CallStarted, true)),
                          AutomatorScript::Item(State(State::OfonoModemPowered, State::CallFinished)),
                          AutomatorScript::Item(State(State::OfonoModemPowered, State::Signal, true)) },
                        this),
    new AutomatorScript(AutomatorScript::ModemOnlineDisable,
                        { AutomatorScript::emptyState,
                          AutomatorScript::Item(State(State::OfonoModemOnline, State::CallStarted, false)),
                          AutomatorScript::Item(State(State::OfonoModemOnline, State::CallFinished)),
                          AutomatorScript::Item(State(State::OfonoModemOnline, State::Signal, false)) },
                        this),
    new AutomatorScript(AutomatorScript::ModemOnlineEnable,
                        { AutomatorScript::emptyState,
                          AutomatorScript::Item(State(State::OfonoModemOnline, State::CallStarted, true)),
                          AutomatorScript::Item(State(State::OfonoModemOnline, State::CallFinished)),
                          AutomatorScript::Item(State(State::OfonoModemOnline, State::Signal, true)) },
                        this),
    new AutomatorScript(AutomatorScript::SimManagerAdded,
                        { AutomatorScript::emptyState,
                          AutomatorScript::Item(State(State::OfonoSimManagerGetProperties, State::CallStarted)),
                          AutomatorScript::Item(State(State::OfonoSimManagerGetProperties, State::CallFinished)) },
                        this),
    new AutomatorScript(
    AutomatorScript::NetworkRegistrationAdded,
    { AutomatorScript::emptyState,
      AutomatorScript::Item(State(State::OfonoNetworkRegistrationGetProperties, State::CallStarted)),
      AutomatorScript::Item(State(State::OfonoNetworkRegistrationGetProperties, State::CallFinished)) },
    this),
    new AutomatorScript(
    AutomatorScript::ConnectionManagerAdded,
    { AutomatorScript::emptyState,
      AutomatorScript::Item(State(State::OfonoConnectionManagerGetProperties, State::CallStarted)),
      AutomatorScript::Item(State(State::OfonoConnectionManagerGetProperties, State::CallFinished)),
      AutomatorScript::Item(State(State::OfonoConnectionManagerGetContexts, State::CallStarted)),
      AutomatorScript::Item(State(State::OfonoConnectionManagerGetContexts, State::CallFinished)) },
    this),
    new AutomatorScript(
    AutomatorScript::ConnectionContextAdded,
    { AutomatorScript::emptyState,
      AutomatorScript::Item(State(State::OfonoConnectionContextGetProperties, State::CallStarted)),
      //           AutomatorScript::Item(State(State::OfonoConnectionManagerContextAdded, State::Signal)),
      AutomatorScript::Item(State(State::OfonoConnectionContextGetProperties, State::CallFinished)) },
    this),
    new AutomatorScript(
    AutomatorScript::ConnectionContextAccessPointName,
    { AutomatorScript::emptyState,
      AutomatorScript::Item(State(State::OfonoConnectionContextAccessPointName, State::CallStarted)),
      AutomatorScript::Item(State(State::OfonoConnectionContextAccessPointName, State::CallFinished)),
      AutomatorScript::Item(State(State::OfonoConnectionContextAccessPointName, State::Signal)) },
    this),
    new AutomatorScript(AutomatorScript::ConnectionContextUsername,
                        { AutomatorScript::emptyState,
                          AutomatorScript::Item(State(State::OfonoConnectionContextUsername, State::CallStarted)),
                          AutomatorScript::Item(State(State::OfonoConnectionContextUsername, State::CallFinished)),
                          AutomatorScript::Item(State(State::OfonoConnectionContextUsername, State::Signal)) },
                        this),
    new AutomatorScript(AutomatorScript::ConnectionContextPassword,
                        { AutomatorScript::emptyState,
                          AutomatorScript::Item(State(State::OfonoConnectionContextPassword, State::CallStarted)),
                          AutomatorScript::Item(State(State::OfonoConnectionContextPassword, State::CallFinished)),
                          AutomatorScript::Item(State(State::OfonoConnectionContextPassword, State::Signal)) },
                        this),
    new AutomatorScript(
    AutomatorScript::ConnectionContextActiveDisable,
    { AutomatorScript::emptyState,
      AutomatorScript::Item(State(State::OfonoConnectionContextActive, State::CallStarted, false)),
      AutomatorScript::Item(State(State::OfonoConnectionContextActive, State::CallFinished)),
      AutomatorScript::Item(State(State::OfonoConnectionContextActive, State::Signal, false)) },
    this),
    new AutomatorScript(
    AutomatorScript::ConnectionContextActiveEnable,
    { AutomatorScript::emptyState,
      AutomatorScript::Item(State(State::OfonoConnectionContextActive, State::CallStarted, true)),
      AutomatorScript::Item(State(State::OfonoConnectionContextActive, State::CallFinished)),
      AutomatorScript::Item(State(State::OfonoConnectionContextActive, State::Signal, true)) },
    this)
  }
{
  _timer->setSingleShot(false);
  connect(_timer.data(), &QTimer::timeout,
          [this]() { automatorScriptStatusChanged(State::_EMPTYSTATUS_, State()); });
  for (AutomatorScript *item : _scripts)
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

      for (AutomatorScript::Type type :
           { AutomatorScript::ManagerModemAdded, AutomatorScript::ModemLockdownDisable, AutomatorScript::ModemLockdownEnable,
             AutomatorScript::ModemPoweredDisable, AutomatorScript::ModemPoweredEnable,
             AutomatorScript::ModemOnlineDisable, AutomatorScript::ModemOnlineEnable })
      {
        AutomatorScript *item = script(type);
        if (item) item->reset();
      }
    }
      [[clang::fallthrough]];
    case State::OfonoModemInterfaceSimManagerRemoved:
    {
      _data.simManagerCardIdentifier.clear();
      _data.simManagerServiceProviderName.clear();
      AutomatorScript *item = script(AutomatorScript::SimManagerAdded);
      if (item) item->reset();
    }
      [[clang::fallthrough]];
    case State::OfonoModemInterfaceNetworkRegistrationRemoved:
    {
      _data.networkRegistrationStatus.clear();
      AutomatorScript *item = script(AutomatorScript::NetworkRegistrationAdded);
      if (item) item->reset();
    }
      [[clang::fallthrough]];
    case State::OfonoModemInterfaceConnectionManagerRemoved:
    {
      _data.connectionManagerAttached.clear();
      _data.connectionManagerPowered.clear();
      AutomatorScript *item = script(AutomatorScript::ConnectionManagerAdded);
      if (item) item->reset();
    }
      [[clang::fallthrough]];
    case State::OfonoConnectionManagerContextRemoved:
    {
      _data.connectionContextAccessPointName.clear();
      _data.connectionContextUsername.clear();
      _data.connectionContextPassword.clear();
      _data.connectionContextActive.clear();

      for (AutomatorScript::Type type :
           { AutomatorScript::ConnectionContextAdded, AutomatorScript::ConnectionContextAccessPointName,
             AutomatorScript::ConnectionContextUsername, AutomatorScript::ConnectionContextPassword,
             AutomatorScript::ConnectionContextActiveDisable, AutomatorScript::ConnectionContextActiveEnable })
      {
        AutomatorScript *item = script(type);
        if (item) item->reset();
      }
      automatorScriptStatusChanged(State::Signal, state);
    }
    break;
    default:
      break;
    }
  }

  for (AutomatorScript *item : _scripts)
  {
    //    if (item->processing(sender, state, _data)) return;
    item->processing(sender, state, _data);
  }
}

void Automator::automatorScriptStatusChanged(const State::Status scriptStatus, const State &state)
{
  if (State::CallError == scriptStatus)
  {
    C("ScriptStatus: State::CallError" << state.error().message());
    throw astr_global::Exception("ScriptStatus: State::CallError");
    // return;
  }

  bool isScriptsRunning = false;
  for (auto iterator = _scripts.cbegin(); !isScriptsRunning && iterator != _scripts.cend(); ++iterator)
    if (State::CallStarted == (*iterator)->status()) isScriptsRunning = true;
  bool isTimerActive = _timer->isActive();
  if (!_call.isEmpty()) goto CALL;

  if (State::CallFinished == scriptStatus)
  {
    AutomatorScript *item = static_cast<AutomatorScript *>(sender());
    switch (item->type())
    {
    case AutomatorScript::ManagerModemAdded:
    {
      D("Call: State::OfonoModemPowered, false");
      _call.reset(State::OfonoModemPowered, false);
      item->reset();
      goto CALL;
    }
    case AutomatorScript::_EMPTYTYPE_:
    {
      C("AutomatorScript::_EMPTYTYPE_");
      throw astr_global::Exception("AutomatorScript::_EMPTYTYPE_");
    }
    default:
      break;
    }
  }

  if (_data.modemLockdown.isValid() && _data.modemLockdown.toBool())
  {
    _call.reset(State::OfonoModemLockdown, false);
    goto CALL;
  }

  if (_data.modemPowered.isValid() && !_data.modemPowered.toBool())
  {
    _call.reset(State::OfonoModemPowered, true);
    goto CALL;
  }

  if (_data.modemOnline.isValid() && !_data.modemOnline.toBool())
  {
    _call.reset(State::OfonoModemOnline, true);
    goto CALL;
  }

CALL:
  if (!_call.isEmpty() && !isScriptsRunning && !isTimerActive)
  {
    State::Type type = _call.type();
    QVariant value = _call.value();
    _call.reset();
    Q_EMIT Call(type, value);
  }
}

void Automator::debugScriptsRunning()
{
  for (auto &item : _scripts)
    D(*item);
}

AutomatorScript *Automator::script(const AutomatorScript::Type type)
{
  for (AutomatorScript *item : _scripts)
  {
    if (item->type() == type) return item;
  }
  return nullptr;
}
