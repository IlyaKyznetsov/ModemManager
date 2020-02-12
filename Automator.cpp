#include "Automator.h"
#include "adapters/Modem.h"

Automator::Automator(const ModemManagerData::Settings &settings, QObject *parent)
: QObject(parent), _settings(settings),
  _managerModemRemoved({ AutomatorScript::Item(State(State::_EMPTYTYPE_, State::_EMPTYSTATUS_)),
                         AutomatorScript::Item(State(State::OfonoManagerModemRemoved, State::Signal)) },
                       this),
  _managerModemAdded({ AutomatorScript::Item(State(State::_EMPTYTYPE_, State::_EMPTYSTATUS_)),
                       AutomatorScript::Item(State(State::OfonoModemGetProperties, State::CallStarted)),
                       AutomatorScript::Item(State(State::OfonoManagerModemAdded, State::Signal)),
                       AutomatorScript::Item(State(State::OfonoModemGetProperties, State::CallFinished)) },
                     this),
  _modemLockdownDisable({ AutomatorScript::Item(State(State::_EMPTYTYPE_, State::_EMPTYSTATUS_)),
                          AutomatorScript::Item(State(State::OfonoModemLockdown, State::CallStarted, false)),
                          AutomatorScript::Item(State(State::OfonoModemLockdown, State::CallFinished)),
                          AutomatorScript::Item(State(State::OfonoModemLockdown, State::Signal, false)) },
                        this),
  _modemLockdownEnable({ AutomatorScript::Item(State(State::_EMPTYTYPE_, State::_EMPTYSTATUS_)),
                         AutomatorScript::Item(State(State::OfonoModemLockdown, State::CallStarted, true)),
                         AutomatorScript::Item(State(State::OfonoModemLockdown, State::CallFinished)),
                         AutomatorScript::Item(State(State::OfonoModemLockdown, State::Signal, true)) },
                       this),
  _modemPoweredDisable({ AutomatorScript::Item(State(State::_EMPTYTYPE_, State::_EMPTYSTATUS_)),
                         AutomatorScript::Item(State(State::OfonoModemPowered, State::CallStarted, false)),
                         AutomatorScript::Item(State(State::OfonoModemPowered, State::CallFinished)),
                         AutomatorScript::Item(State(State::OfonoModemPowered, State::Signal, false)) },
                       this),
  _modemPoweredEnable({ AutomatorScript::Item(State(State::_EMPTYTYPE_, State::_EMPTYSTATUS_)),
                        AutomatorScript::Item(State(State::OfonoModemPowered, State::CallStarted, true)),
                        AutomatorScript::Item(State(State::OfonoModemPowered, State::CallFinished)),
                        AutomatorScript::Item(State(State::OfonoModemPowered, State::Signal, true)) },
                      this),
  _modemOnlineDisable({ AutomatorScript::Item(State(State::_EMPTYTYPE_, State::_EMPTYSTATUS_)),
                        AutomatorScript::Item(State(State::OfonoModemOnline, State::CallStarted, false)),
                        AutomatorScript::Item(State(State::OfonoModemOnline, State::CallFinished)),
                        AutomatorScript::Item(State(State::OfonoModemOnline, State::Signal, false)) },
                      this),
  _modemOnlineEnable({ AutomatorScript::Item(State(State::_EMPTYTYPE_, State::_EMPTYSTATUS_)),
                       AutomatorScript::Item(State(State::OfonoModemOnline, State::CallStarted, true)),
                       AutomatorScript::Item(State(State::OfonoModemOnline, State::CallFinished)),
                       AutomatorScript::Item(State(State::OfonoModemOnline, State::Signal, true)) },
                     this),
  _simManagerAdded({ AutomatorScript::Item(State(State::_EMPTYTYPE_, State::_EMPTYSTATUS_)),
                     AutomatorScript::Item(State(State::OfonoModemInterfaceSimManagerAdded, State::Signal)),
                     AutomatorScript::Item(State(State::OfonoSimManagerGetProperties, State::CallStarted)),
                     AutomatorScript::Item(State(State::OfonoSimManagerGetProperties, State::CallFinished)) },
                   this),
  _networkRegistrationAdded(
  { AutomatorScript::Item(State(State::_EMPTYTYPE_, State::_EMPTYSTATUS_)),
    AutomatorScript::Item(State(State::OfonoModemInterfaceNetworkRegistrationAdded, State::Signal)),
    AutomatorScript::Item(State(State::OfonoNetworkRegistrationGetProperties, State::CallStarted)),
    AutomatorScript::Item(State(State::OfonoNetworkRegistrationGetProperties, State::CallFinished)) },
  this),
  _connectionManagerAdded(
  { AutomatorScript::Item(State(State::_EMPTYTYPE_, State::_EMPTYSTATUS_)),
    AutomatorScript::Item(State(State::OfonoModemInterfaceConnectionManagerAdded, State::Signal)),
    AutomatorScript::Item(State(State::OfonoConnectionManagerGetProperties, State::CallStarted)),
    AutomatorScript::Item(State(State::OfonoConnectionManagerGetProperties, State::CallFinished)),
    AutomatorScript::Item(State(State::OfonoConnectionManagerGetContexts, State::CallStarted)),
    AutomatorScript::Item(State(State::OfonoConnectionManagerGetContexts, State::CallFinished)) },
  this),
  _connectionContextAdded(
  { AutomatorScript::Item(State(State::_EMPTYTYPE_, State::_EMPTYSTATUS_)),
    AutomatorScript::Item(State(State::OfonoConnectionManagerContextAdded, State::Signal)),
    AutomatorScript::Item(State(State::OfonoConnectionContextGetProperties, State::CallStarted)),
    AutomatorScript::Item(State(State::OfonoConnectionContextGetProperties, State::CallFinished)) },
  this)

{
  connect(&_managerModemRemoved, &AutomatorScript::StatusChanged, this, &Automator::onStatusChanged);
  connect(&_managerModemAdded, &AutomatorScript::StatusChanged, this, &Automator::onStatusChanged);
  connect(&_modemLockdownDisable, &AutomatorScript::StatusChanged, this, &Automator::onStatusChanged);
  connect(&_modemLockdownEnable, &AutomatorScript::StatusChanged, this, &Automator::onStatusChanged);
  connect(&_modemPoweredDisable, &AutomatorScript::StatusChanged, this, &Automator::onStatusChanged);
  connect(&_modemPoweredEnable, &AutomatorScript::StatusChanged, this, &Automator::onStatusChanged);
  connect(&_modemOnlineDisable, &AutomatorScript::StatusChanged, this, &Automator::onStatusChanged);
  connect(&_modemOnlineEnable, &AutomatorScript::StatusChanged, this, &Automator::onStatusChanged);
  connect(&_networkRegistrationAdded, &AutomatorScript::StatusChanged, this, &Automator::onStatusChanged);
  connect(&_connectionManagerAdded, &AutomatorScript::StatusChanged, this, &Automator::onStatusChanged);
  connect(&_connectionContextAdded, &AutomatorScript::StatusChanged, this, &Automator::onStatusChanged);
}

void Automator::processing(QObject *sender, const State &state)
{

  if (State::Signal == state.status())
  {
    switch (state.type())
    {
      /*
        case State::OfonoManagerModemRemoved:
        {
          _automatorInitializationIterator = _automatorScriptInitialization.begin();
          _automatorConnectionContextIterator = _automatorScriptInitializationConnectionContext.begin();
          _data.modemLockdown.clear();
          _data.modemPowered.clear();
          _data.modemOnline.clear();
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
          _data.simManagerCardIdentifier.clear();
          _data.simManagerServiceProviderName.clear();
        }
        break;
        case State::OfonoModemInterfaceNetworkRegistrationRemoved:
        {
          _data.networkRegistrationStatus.clear();
        }
        break;
        case State::OfonoModemInterfaceConnectionManagerRemoved:
        {
          _automatorConnectionContextIterator = _automatorScriptInitializationConnectionContext.begin();
          _data.connectionContextAccessPointName.clear();
          _data.connectionContextUsername.clear();
          _data.connectionContextPassword.clear();
          _data.connectionManagerAttached.clear();
          _data.connectionManagerPowered.clear();
          _data.connectionContextActive.clear();
        }
        break;
        */
    case State::OfonoModemPowered:
    {
      _data.modemPowered = state.value();
    }
    break;
    case State::OfonoModemOnline:
    {
      _data.modemOnline = state.value();
    }
    break;
    case State::OfonoModemLockdown:
    {
      _data.modemLockdown = state.value();
    }
    break;
    case State::OfonoSimManagerCardIdentifier:
    {
      _data.simManagerCardIdentifier = state.value();
    }
    break;
    case State::OfonoSimManagerServiceProviderName:
    {
      _data.simManagerServiceProviderName = state.value();
      //        _data.provider = _settings.providerSettings(_data.simManagerServiceProviderName.toString());
      //        D("---PROVIDER---:" << _data.provider.accessPointName << _data.provider.username
      //                            << _data.provider.password);
    }
    break;
    case State::OfonoNetworkRegistrationStatus:
    {
      _data.networkRegistrationStatus = state.value();
    }
    break;

    case State::OfonoConnectionContextAccessPointName:
    {
      _data.connectionContextAccessPointName = state.value();
    }
    break;
    case State::OfonoConnectionContextUsername:
    {
      _data.connectionContextUsername = state.value();
    }
    break;
    case State::OfonoConnectionContextPassword:
    {
      _data.connectionContextPassword = state.value();
    }
    break;

    case State::OfonoConnectionManagerAttached:
    {
      _data.connectionManagerAttached = state.value();
    }
    break;
    case State::OfonoConnectionManagerPowered:
    {
      _data.connectionManagerPowered = state.value();
    }
    break;
    case State::OfonoConnectionContextActive:
    {
      _data.connectionContextActive = state.value();
    }
    break;

    default:
      break;
    }
  }

  _managerModemRemoved.processing(sender, state, _data);
  _managerModemAdded.processing(sender, state, _data);
  _modemLockdownDisable.processing(sender, state, _data);
  _modemLockdownEnable.processing(sender, state, _data);
  _modemPoweredDisable.processing(sender, state, _data);
  _modemPoweredEnable.processing(sender, state, _data);
  _modemOnlineDisable.processing(sender, state, _data);
  _modemOnlineEnable.processing(sender, state, _data);
}

void Automator::run(QObject *adapter, const State::Type type, const QVariant &value)
{
  switch (type)
  {
  case State::OfonoModemLockdown:
  case State::OfonoModemPowered:
  case State::OfonoModemOnline:
  {
    static_cast<Modem *>(adapter)->call(type, value.toBool());
  }
  break;
  default:
    break;
  }
}

void Automator::onStatusChanged(const AutomatorScript::ScriptStatus status)
{
  DF() << sender() << status;
  AutomatorScript *script = static_cast<AutomatorScript *>(sender());
  switch (status)
  {
  case AutomatorScript::NotStarted:
  {
  }
  break;
  case AutomatorScript::Started:
  {
  }
  break;
  case AutomatorScript::Finished:
  {
    script->reset();
  }
  break;
  case AutomatorScript::Error:
  {
    script->reset();
  }
  break;
  }
}
