#include "Automator.h"
#include "adapters/Modem.h"

Automator::Automator(const ModemManagerData::Settings &settings, QObject *parent) : QObject(parent), _settings(settings)
{
}

Automator::~Automator()
{
}

void Automator::processing(const State &state)
{
  D("--- PROCESSING ---" << state);
  bool connectionManagerDetached = false;
  bool networkRegistrationDeregistered = false;
  switch (state.type())
  {
    case State::OfonoManagerModemRemoved: _data.resetModem(); break;
    case State::OfonoModemInterfaceSimManagerRemoved: _data.resetSimManager(); break;
    case State::OfonoModemInterfaceConnectionManagerRemoved: _data.resetConnectionManager(); break;
    case State::OfonoModemInterfaceNetworkRegistrationRemoved: _data.resetNetworkRegistration(); break;
    case State::OfonoConnectionManagerContextRemoved: _data.resetConnectionContext(); break;
    case State::OfonoSimManagerCardIdentifier: _data.simManagerCardIdentifier = state.value().toString(); break;
    case State::OfonoSimManagerServiceProviderName: _data.simManagerProviderName = state.value().toString(); break;
    case State::OfonoConnectionManagerPowered: _data.connectionManagerPowered = state.value().toBool(); break;
    case State::OfonoSimManagerGetProperties:
      if (state.status() == State::CallFinished)
        _data.simManagerInitialized = true;
      break;
    case State::OfonoNetworkRegistrationGetProperties:
      if (state.status() == State::CallFinished)
        _data.networkRegistrationInitialized = true;
      break;
    case State::OfonoConnectionManagerGetProperties:
      if (state.status() == State::CallFinished)
        _data.connectionManagerInitialized = true;
      break;
    case State::OfonoConnectionContextGetProperties:
      if (state.status() == State::CallFinished)
        _data.connectionContextInitialized = true;
      break;

    case State::OfonoModemLockdown:
    {
      _data.modemLockdown = state.value().toBool();
      _data.modemLockdownStatus = state.status();
    }
    break;
    case State::OfonoModemPowered:
    {
      _data.modemPowered = state.value().toBool();
      if (_data.modemPowered && State::_EMPTYSTATUS_ == _data.modemPoweredStatus && State::Signal == state.status())
        _data.needPowerOff = true;
      _data.modemPoweredStatus = state.status();
    }
    break;
    case State::OfonoModemOnline:
    {
      _data.modemOnline = state.value().toBool();
      _data.modemOnlineStatus = state.status();
    }
    break;
    case State::OfonoNetworkRegistrationStatus:
    {
      networkRegistrationDeregistered =
          ("registered" == _data.networkRegistrationStatus && "registered" != state.value().toString());
      _data.networkRegistrationStatus = state.value().toString();
    }
    break;
    case State::OfonoConnectionManagerAttached:
    {
      connectionManagerDetached = (_data.connectionManagerAttached && !state.value().toBool());
      _data.connectionManagerAttached = state.value().toBool();
    }
    break;
    case State::OfonoConnectionContextAccessPointName:
    {
      _data.connectionContextAccessPointName = state.value().toString();
      _data.connectionContextAccessPointNameStatus = state.status();
    }
    break;
    case State::OfonoConnectionContextUsername:
    {
      _data.connectionContextUsername = state.value().toString();
      _data.connectionContextUsernameStatus = state.status();
    }
    break;
    case State::OfonoConnectionContextPassword:
    {
      _data.connectionContextPassword = state.value().toString();
      _data.connectionContextPasswordStatus = state.status();
    }
    break;
    case State::OfonoConnectionContextActive:
    {
      _data.connectionContextActive = state.value().toBool();
      _data.connectionContextActiveStatus = state.status();
    }
    break;
    default: break;
  }

  if (State::CallError == state.status())
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
            //            Q_EMIT Call(state.type(), state.value());
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
        //          debugScriptsRunning();
        throw astr_global::Exception("--- State::CallError: " + state);
      }
      break;
    }
  }

  _data.debug();

  if (!(State::Signal == _data.modemLockdownStatus && State::Signal == _data.modemPoweredStatus &&
        State::Signal == _data.modemOnlineStatus))
    return;

  // Autoconnection

  // 1. Restart new modem
  if (_data.needPowerOff)
  {
    _data.needPowerOff = false;
    Q_EMIT Call(State::OfonoModemPowered, false);
    return;
  }

  // 1. PowerOn modem
  if (!_data.modemPowered)
  {
    Q_EMIT Call(State::OfonoModemPowered, true);
    return;
  }

  // 2. Wait modem interfaces
  if (!(_data.simManagerInitialized && _data.connectionManagerInitialized && _data.connectionContextInitialized))
    return;

  // 3. Online modem
  if (!_data.modemOnline)
  {
    Q_EMIT Call(State::OfonoModemOnline, true);
    return;
  }

  // 4. Wait sim provider

  if (!_data.networkRegistrationInitialized ||
      !(State::Signal == _data.connectionContextAccessPointNameStatus &&
        State::Signal == _data.connectionContextUsernameStatus &&
        State::Signal == _data.connectionContextPasswordStatus &&
        State::Signal == _data.connectionContextActiveStatus) ||
      _data.simManagerProviderName.isEmpty())
    return;

  // 5. Configuration ConnectionContext
  const ModemManagerData::Settings::Provider provider = _settings.providerSettings(_data.simManagerProviderName);
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

  if (connectionManagerDetached)
  {
    Q_EMIT Call(State::OfonoModemPowered, false);
    return;
  }

  if ("registered" == _data.networkRegistrationStatus && _data.connectionManagerAttached &&
      !_data.connectionContextActive)
  {
    Q_EMIT Call(State::OfonoConnectionContextActive, true);
    return;
  }
}

void Automator::reset()
{
  DF();
  _data.resetModem();
}

void Automator::Data::resetConnectionContext()
{
  connectionContextInitialized = false;
  connectionContextActive = false;
  connectionContextAccessPointName.clear();
  connectionContextUsername.clear();
  connectionContextPassword.clear();
  connectionContextAccessPointNameStatus = State::_EMPTYSTATUS_;
  connectionContextUsernameStatus = State::_EMPTYSTATUS_;
  connectionContextPasswordStatus = State::_EMPTYSTATUS_;
  connectionContextActiveStatus = State::_EMPTYSTATUS_;
}

void Automator::Data::resetConnectionManager()
{
  connectionManagerInitialized = false;
  connectionManagerAttached = false;
  connectionManagerPowered = false;
  connectionContextInitialized = false;
  connectionContextActive = false;
}

void Automator::Data::resetNetworkRegistration()
{
  networkRegistrationInitialized = false;
  networkRegistrationStatus.clear();
}

void Automator::Data::resetSimManager()
{
  simManagerInitialized = false;
  simManagerCardIdentifier.clear();
  simManagerProviderName.clear();
}

void Automator::Data::resetModem()
{
  needPowerOff = false;
  modemLockdown = false;
  modemPowered = false;
  modemOnline = false;
  modemLockdownStatus = State::_EMPTYSTATUS_;
  modemPoweredStatus = State::_EMPTYSTATUS_;
  modemOnlineStatus = State::_EMPTYSTATUS_;
  resetConnectionContext();
  resetConnectionManager();
  resetNetworkRegistration();
  resetSimManager();
}

void Automator::Data::debug()
{
  D("----------------------------------------------------------");
  D("SSS modemLockdownStatus                   :" << State::toString(modemLockdownStatus));
  D("SSS modemPoweredStatus                    :" << State::toString(modemPoweredStatus));
  D("SSS modemOnlineStatus                     :" << State::toString(modemOnlineStatus));
  D("SSS connectionContextAccessPointNameStatus:" << State::toString(connectionContextAccessPointNameStatus));
  D("SSS connectionContextUsernameStatus       :" << State::toString(connectionContextUsernameStatus));
  D("SSS connectionContextPasswordStatus       :" << State::toString(connectionContextPasswordStatus));
  D("SSS connectionContextActiveStatus         :" << State::toString(connectionContextActiveStatus));
  D("+++ simManagerInitialized             :" << simManagerInitialized);
  D("+++ networkRegistrationInitialized    :" << networkRegistrationInitialized);
  D("+++ connectionManagerInitialized      :" << connectionManagerInitialized);
  D("+++ connectionContextInitialized      :" << connectionContextInitialized);
  D("ModemLockdown                   : " << modemLockdown);
  D("ModemPowered                    : " << modemPowered);
  D("ModemOnline                     : " << modemOnline);
  D("SimManagerCardIdentifier        : " << simManagerCardIdentifier);
  D("SimManagerServiceProviderName   :" << simManagerProviderName);
  D("NetworkRegistrationStatus       :" << networkRegistrationStatus);
  D("ConnectionManagerAttached       :" << connectionManagerAttached);
  D("ConnectionManagerPowered        :" << connectionManagerPowered);
  D("ConnectionContextAccessPointName:" << connectionContextAccessPointName);
  D("ConnectionContextUsername       :" << connectionContextUsername);
  D("ConnectionContextPassword       :" << connectionContextPassword);
  D("ConnectionContextActive         :" << connectionContextActive);
  D("----------------------------------------------------------");
}
