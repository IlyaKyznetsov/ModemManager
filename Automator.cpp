#include "Automator.h"

Automator::Automator(QObject *parent)
    : QObject(parent),
      modemAdded(false),
      connectionContextConfigured(false),
      connectionContextCount(0),
      requestCall(State::_EMPTYTYPE_),
      modemLockdownStatus(State::_EMPTYSTATUS_),
      modemPoweredStatus(State::_EMPTYSTATUS_),
      modemOnlineStatus(State::_EMPTYSTATUS_),
      connectionManagerPoweredStatus(State::_EMPTYSTATUS_),
      connectionContextAccessPointNameStatus(State::_EMPTYSTATUS_),
      connectionContextUsernameStatus(State::_EMPTYSTATUS_),
      connectionContextPasswordStatus(State::_EMPTYSTATUS_),
      connectionContextActiveStatus(State::_EMPTYSTATUS_)
{
}

void Automator::reset()
{
  DF();
  modemAdded = false;
  connectionContextConfigured = false;
  connectionContextCount = 0;
  requestCall = State::_EMPTYTYPE_;
  requestValue.clear();
  modemLockdownStatus = State::_EMPTYSTATUS_;
  modemPoweredStatus = State::_EMPTYSTATUS_;
  modemOnlineStatus = State::_EMPTYSTATUS_;
  connectionManagerPoweredStatus = State::_EMPTYSTATUS_;
  connectionContextAccessPointNameStatus = State::_EMPTYSTATUS_;
  connectionContextUsernameStatus = State::_EMPTYSTATUS_;
  connectionContextPasswordStatus = State::_EMPTYSTATUS_;
  connectionContextActiveStatus = State::_EMPTYSTATUS_;
}

void Automator::processing(const ModemManagerData::Settings &settings, const State &state,
                           const ModemManagerData::OfonoState &ofonoState)
{
  if (State::CallError == state.status())
  {
    errorHandler(state, ofonoState);
    return;
  }

  switch (state.type())
  {
    case State::OfonoManagerModemAdded:
    {
      modemAdded = true;
      connectionContextCount = 0;
    }
    break;
    case State::OfonoModemLockdown:
    {
      modemLockdownStatus = state.status();
      if (State::CallFinished == modemLockdownStatus && requestValue == ofonoState.modem->lockdown)
      {
        modemLockdownStatus = State::Signal;
        requestValue.clear();
      }
    }
    break;
    case State::OfonoModemPowered:
    {
      modemPoweredStatus = state.status();
      if (State::CallFinished == modemPoweredStatus && requestValue == ofonoState.modem->powered)
      {
        modemPoweredStatus = State::Signal;
        requestValue.clear();
      }
    }
    break;
    case State::OfonoSimManagerCardIdentifier: break;
    case State::OfonoSimManagerServiceProviderName: break;
    case State::OfonoModemOnline:
    {
      modemOnlineStatus = state.status();
      if (State::CallFinished == modemOnlineStatus && requestValue == ofonoState.modem->online)
      {
        modemOnlineStatus = State::Signal;
        requestValue.clear();
      }
    }
    break;
    case State::OfonoNetworkRegistrationStatus:
    {
      if (State::OfonoNetworkRegistrationRegister == requestCall)
      {
        requestCall = State::_EMPTYTYPE_;
        requestValue.clear();
      }
    }
    break;
    case State::OfonoConnectionManagerContextAdded:
    {
      connectionContextCount += 1;
    }
    break;
    case State::OfonoConnectionManagerContextNone:
    {
      connectionContextCount = -1;
    }
    break;
    case State::OfonoConnectionManagerPowered:
    {
      connectionManagerPoweredStatus = state.status();
      if (State::CallFinished == connectionManagerPoweredStatus)
      {
        connectionManagerPoweredStatus = State::Signal;
        requestValue.clear();
      }
    }
    break;
    case State::OfonoConnectionManagerAttached: break;
    case State::OfonoConnectionContextAccessPointName:
    {
      connectionContextAccessPointNameStatus = state.status();
      if (State::CallFinished == connectionContextAccessPointNameStatus &&
          requestValue == ofonoState.connectionContext->accessPointName)
      {
        connectionContextAccessPointNameStatus = State::Signal;
        requestValue.clear();
      }
    }
    break;
    case State::OfonoConnectionContextUsername:
    {
      connectionContextUsernameStatus = state.status();
      if (State::CallFinished == connectionContextUsernameStatus &&
          requestValue == ofonoState.connectionContext->username)
      {
        connectionContextUsernameStatus = State::Signal;
        requestValue.clear();
      }
    }
    break;
    case State::OfonoConnectionContextPassword:
    {
      connectionContextPasswordStatus = state.status();
      if (State::CallFinished == connectionContextPasswordStatus &&
          requestValue == ofonoState.connectionContext->password)
      {
        connectionContextPasswordStatus = State::Signal;
        requestValue.clear();
      }
    }
    break;
    case State::OfonoConnectionContextActive:
    {
      connectionContextActiveStatus = state.status();
      if (State::CallFinished == connectionContextActiveStatus && requestValue == ofonoState.connectionContext->active)
      {
        connectionContextActiveStatus = State::Signal;
        requestValue.clear();
      }
    }
    break;
    default: return;
  }

  debug(state, ofonoState);

  // Automation

  if (requestCall == state.type() && state.status() == State::CallFinished)
    requestCall = State::_EMPTYTYPE_;

  if (State::_EMPTYTYPE_ != requestCall || ofonoState.modem.isNull() || State::Signal != modemLockdownStatus ||
      State::Signal != modemPoweredStatus || State::Signal != modemOnlineStatus)
    return;

  if (modemAdded && !ofonoState.modem->lockdown)
  {
    modemAdded = false;
    requestCall = State::OfonoModemLockdown;
    requestValue = true;
    Q_EMIT call(requestCall, requestValue);
    return;
  }
  modemAdded = false;

  D("1. Modem Lockdown");
  if (ofonoState.modem->lockdown)
  {
    requestCall = State::OfonoModemLockdown;
    requestValue = false;
    Q_EMIT call(requestCall, requestValue);
    return;
  }

  D("2. Modem PowerOn");
  if (!ofonoState.modem->powered)
  {
    requestCall = State::OfonoModemPowered;
    requestValue = true;
    Q_EMIT call(requestCall, requestValue);
    return;
  }

  D("3. Wait sim card");
  if (ofonoState.simManager.isNull() || ofonoState.simManager->cardIdentifier.isEmpty())
    return;

  D("4. Online modem");
  if (!ofonoState.modem->online)
  {
    requestCall = State::OfonoModemOnline;
    requestValue = true;
    Q_EMIT call(requestCall, requestValue);
    return;
  }

  if (ofonoState.connectionManager.isNull() || ofonoState.connectionContext.isNull() || 0 == connectionContextCount)
    return;

  if (-1 == connectionContextCount)
  {
    connectionContextConfigured = false;
    requestCall = State::OfonoConnectionManagerContextAdded;
    requestValue = "internet";
    Q_EMIT call(requestCall, "internet");
    return;
  }

  if (ofonoState.simManager->serviceProviderName.isEmpty() || State::Signal != connectionContextAccessPointNameStatus ||
      State::Signal != connectionContextUsernameStatus || State::Signal != connectionContextPasswordStatus)
    return;

  D("6. Configuration ConnectionContext");
  if (!connectionContextConfigured || ofonoState.networkRegistration.isNull())
  {
    const ModemManagerData::Settings::Provider &provider =
        settings.providerSettings(ofonoState.simManager->serviceProviderName);
    const QString apn = (provider.accessPointName.isValid() ? provider.accessPointName.toString() : QString());
    const QString user = (provider.username.isValid() ? provider.username.toString() : QString());
    const QString password = (provider.password.isValid() ? provider.password.toString() : QString());

    if (apn != ofonoState.connectionContext->accessPointName)
    {
      requestCall = State::OfonoConnectionContextAccessPointName;
      requestValue = apn;
      Q_EMIT call(requestCall, requestValue);
      return;
    }
    if (user != ofonoState.connectionContext->username)
    {
      requestCall = State::OfonoConnectionContextUsername;
      requestValue = user;
      Q_EMIT call(requestCall, requestValue);
      return;
    }
    if (password != ofonoState.connectionContext->password)
    {
      requestCall = State::OfonoConnectionContextPassword;
      requestValue = password;
      Q_EMIT call(requestCall, requestValue);
      return;
    }
    connectionContextConfigured = true;
  }

  if (!(State::Signal == connectionManagerPoweredStatus))
    return;

  D("5. NetworkRegistration");
  if ("registered" == ofonoState.networkRegistration->status)
  {
    if (!ofonoState.connectionManager->powered)
    {
      requestCall = State::OfonoConnectionManagerPowered;
      requestValue = true;
      Q_EMIT call(requestCall, requestValue);
      return;
    }
  }
  else
  {
    if (ofonoState.connectionManager->powered)
    {
      requestCall = State::OfonoConnectionManagerPowered;
      requestValue = false;
      Q_EMIT call(requestCall, requestValue);
      return;
    }
    if ("unregistered" == ofonoState.networkRegistration->status)
    {
      requestCall = State::OfonoNetworkRegistrationRegister;
      Q_EMIT call(requestCall, true);
      return;
    }
  }

  if (State::Signal != connectionContextActiveStatus)
    return;

  D("8. ConnectionContextActive");
  if (ofonoState.connectionManager->attached && !ofonoState.connectionContext->active)
  {
    requestCall = State::OfonoConnectionContextActive;
    requestValue = true;
    Q_EMIT call(requestCall, requestValue);
    return;
  }
}

void Automator::errorHandler(const State &state, const ModemManagerData::OfonoState &ofonoState)
{
  DF();
  debug(state, ofonoState);
}

void Automator::debug(const State &state, const ModemManagerData::OfonoState &ofonoState) const
{
  static long n = 0;
  //  D("-----------------------------------------------------------------");
  D("--- PROCESSING" << ++n << state);
  //  return;
  D("Autoconnection");
  D("modemInitialized                       :" << (modemAdded ? "true" : "false"));
  D("connectionContextCount                 :" << QString::number(connectionContextCount));
  D("requestCall                            :" << State::toString(requestCall));
  D("modemLockdownStatus                    :" << State::State::toString(modemLockdownStatus));
  D("modemPoweredStatus                     :" << State::State::toString(modemPoweredStatus));
  D("modemOnlineStatus                      :" << State::State::toString(modemOnlineStatus));
  D("connectionManagerPoweredStatus         :" << State::State::toString(connectionManagerPoweredStatus));
  D("connectionContextAccessPointNameStatus :" << State::State::toString(connectionContextAccessPointNameStatus));
  D("connectionContextUsernameStatus        :" << State::State::toString(connectionContextUsernameStatus));
  D("connectionContextPasswordStatus        :" << State::State::toString(connectionContextPasswordStatus));
  D("connectionContextActiveStatus          :" << State::State::toString(connectionContextActiveStatus));
  D("------------ OfonoState ------------");
  D("Ofono connected:" << ofonoState.isOfonoConnected);
  if (!ofonoState.isOfonoConnected)
    return;

  if (ofonoState.modem.isNull())
    return;

  D("----Modem----");
  D("Powered     :" << ofonoState.modem->powered);
  D("Online      :" << ofonoState.modem->online);
  D("Lockdown    :" << ofonoState.modem->lockdown);
  D("Manufacturer:" << ofonoState.modem->manufacturer);
  D("Model       :" << ofonoState.modem->model);
  D("Serial      :" << ofonoState.modem->serial);

  if (!ofonoState.simManager.isNull())
  {
    D("----SimManager----");
    D("CardIdentifier     :" << ofonoState.simManager->cardIdentifier);
    D("ServiceProviderName:" << ofonoState.simManager->serviceProviderName);
  }

  if (!ofonoState.networkRegistration.isNull())
  {
    D("----NetworkRegistration----");
    D("Status  :" << ofonoState.networkRegistration->status);
    D("Name    :" << ofonoState.networkRegistration->name);
    D("Strength:" << ofonoState.networkRegistration->strength);
  }

  if (!ofonoState.connectionManager.isNull())
  {
    D("----ConnectionManager----");
    D("Attached      :" << ofonoState.connectionManager->attached);
    D("RoamingAllowed:" << ofonoState.connectionManager->roamingAllowed);
    D("Powered       :" << ofonoState.connectionManager->powered);
  }

  if (!ofonoState.connectionContext.isNull())
  {
    D("--ConnectionContext--");
    D("Active              :" << ofonoState.connectionContext->active);
    D("AccessPointName     :" << ofonoState.connectionContext->accessPointName);
    D("Username            :" << ofonoState.connectionContext->username);
    D("Password            :" << ofonoState.connectionContext->password);
    D("Type                :" << ofonoState.connectionContext->type);
    D("AuthenticationMethod:" << ofonoState.connectionContext->authenticationMethod);
    D("Protocol            :" << ofonoState.connectionContext->protocol);
    D("Name                :" << ofonoState.connectionContext->name);
    D("Interface           :" << ofonoState.connectionContext->interface);
    D("Method              :" << ofonoState.connectionContext->method);
    D("Address             :" << ofonoState.connectionContext->address);
    D("Netmask             :" << ofonoState.connectionContext->netmask);
  }
  D("-----------------------------------------------------------------");
}

bool Automator::isTimeoutError(const QDBusError &error)
{
  const QVector<QDBusError::ErrorType> errotTypes{QDBusError::NoReply, QDBusError::Timeout, QDBusError::TimedOut};
  if (State::OfonoErrorType::DBusError == State::errorType(error) && errotTypes.contains(error.type()))
    return true;
  return false;
}
