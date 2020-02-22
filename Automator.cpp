#include "Automator.h"

Automator::Automator(QObject *parent)
: QObject(parent), requestCall(State::_EMPTYTYPE_), modemLockdownStatus(State::_EMPTYSTATUS_),
  modemPoweredStatus(State::_EMPTYSTATUS_), modemOnlineStatus(State::_EMPTYSTATUS_),
  connectionManagerPoweredStatus(State::_EMPTYSTATUS_),
  connectionContextAccessPointNameStatus(State::_EMPTYSTATUS_),
  connectionContextUsernameStatus(State::_EMPTYSTATUS_),
  connectionContextPasswordStatus(State::_EMPTYSTATUS_), connectionContextActiveStatus(State::_EMPTYSTATUS_)
{
}

void Automator::reset()
{
  DF();
  requestCall = State::_EMPTYTYPE_;
  modemLockdownStatus = State::_EMPTYSTATUS_;
  modemPoweredStatus = State::_EMPTYSTATUS_;
  modemOnlineStatus = State::_EMPTYSTATUS_;
  connectionManagerPoweredStatus = State::_EMPTYSTATUS_;
  connectionContextAccessPointNameStatus = State::_EMPTYSTATUS_;
  connectionContextUsernameStatus = State::_EMPTYSTATUS_;
  connectionContextPasswordStatus = State::_EMPTYSTATUS_;
  connectionContextActiveStatus = State::_EMPTYSTATUS_;
}

void Automator::processing(const State &state, const ModemManagerData::OfonoState &ofonoState)
{
  if (State::CallError == state.status())
  {
    errorHandler(state, ofonoState);
    return;
  }

  switch (state.type())
  {
  case State::OfonoModemLockdown:
  {
    modemLockdownStatus = state.status();
  }
  break;
  case State::OfonoModemPowered:
  {
    modemPoweredStatus = state.status();
  }
  break;
  case State::OfonoSimManagerCardIdentifier:
  {
  }
  break;
  case State::OfonoSimManagerServiceProviderName:
  {
  }
  break;
  case State::OfonoModemOnline:
  {
    modemOnlineStatus = state.status();
  }
  break;
  case State::OfonoNetworkRegistrationStatus:
  {
    //    if ("registered" == ofonoState.networkRegistration->status)
#warning Может потребоваться для определения потери связи
  }
  break;
  case State::OfonoConnectionManagerContextAdded:
  {
#warning Добавлен ConnectionContext
  }
  break;
  case State::OfonoConnectionManagerContextNone:
  {
#warning ConnectionContext-ы отсутствуют
  }
  break;
  case State::OfonoConnectionManagerPowered:
  {
    connectionManagerPoweredStatus = state.status();
  }
  break;
  case State::OfonoConnectionManagerAttached:
  {
    //    if (ofonoState.connectionManager->attached) connectionManagerAtached = true;
#warning Может потребоваться для определения потери связи
  }
  break;
  case State::OfonoConnectionContextAccessPointName:
  {
    connectionContextAccessPointNameStatus = state.status();
  }
  break;
  case State::OfonoConnectionContextUsername:
  {
    connectionContextUsernameStatus = state.status();
  }
  break;
  case State::OfonoConnectionContextPassword:
  {
    connectionContextPasswordStatus = state.status();
  }
  break;
  case State::OfonoConnectionContextActive:
  {
    connectionContextActiveStatus = state.status();
    //    if (ofonoState.connectionContext->active) connectionContextActive = true;
#warning Может потребоваться для определения потери связи
  }
  break;
  default:
    return;
  }

  debug(state, ofonoState);

  // Automation
  if (ofonoState.modem.isNull()) return;

  if (requestCall == state.type() && state.status() == State::CallFinished)
    requestCall = State::_EMPTYTYPE_;

  if (State::_EMPTYTYPE_ != requestCall || State::Signal != modemLockdownStatus ||
      State::Signal != modemPoweredStatus || State::Signal != modemOnlineStatus)
    return;

  D("1. Modem Lockdown");
  if (ofonoState.modem->lockdown)
  {
    requestCall = State::OfonoModemLockdown;
    Q_EMIT call(requestCall, false);
    return;
  }

  D("2. Modem PowerOn");
  if (!ofonoState.modem->powered)
  {
    requestCall = State::OfonoModemPowered;
    Q_EMIT call(requestCall, true);
    return;
  }

  D("3. Wait sim card");
  if (ofonoState.simManager.isNull() || ofonoState.simManager->cardIdentifier.isEmpty()) return;

  D("4. Online modem");
  if (!ofonoState.modem->online)
  {
    requestCall = State::OfonoModemOnline;
    Q_EMIT call(requestCall, true);
    return;
  }

  if (ofonoState.connectionManager.isNull()) return;

  D("5. ConnectionManager Powered (GPRS enable)");
  if (!ofonoState.connectionManager->powered)
  {
    requestCall = State::OfonoConnectionManagerPowered;
    Q_EMIT call(requestCall, true);
    return;
  }

  //  requestCall = State::OfonoConnectionManagerPowered;
  //  Q_EMIT call(requestCall, false);

  //  D("3. ")


  //  if (!ofonoState.modem->online)
  //  {
  //    requestCall = State::OfonoModemOnline;
  //    Q_EMIT call(State::OfonoModemOnline, true);
  //    return;
  //  }
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
  D("Autoconnection");
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
  if (!ofonoState.isOfonoConnected) return;

  if (ofonoState.modem.isNull()) return;

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
  const QVector<QDBusError::ErrorType> errotTypes{ QDBusError::NoReply, QDBusError::Timeout, QDBusError::TimedOut };
  if (State::OfonoErrorType::DBusError == State::errorType(error) && errotTypes.contains(error.type()))
    return true;
  return false;
}
