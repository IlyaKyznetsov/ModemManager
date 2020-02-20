#include "types.h"

State::State() : _type(_EMPTYTYPE_), _status(_EMPTYSTATUS_)
{
  // _error.type() = NoError
  // _value = QVariant(Invalid);
}

State::State(Type type, Status status) : _type(type), _status(status)
{
}

State::State(Type type, Status status, const QVariant &value) : _type(type), _status(status), _value(value)
{
}

State::State(Type type, Status status, const QDBusError &error) : _type(type), _status(status), _error(error)
{
}

State::State(Type type, Status status, const QVariant &value, const QDBusError &error)
    : _type(type), _status(status), _value(value), _error(error)
{
}

QString State::toString(State::Type type)
{
  const QMap<Type, QString> map = {
      {Type::_EMPTYTYPE_, "_EMPTYTYPE_"},
      {AdapterManagerReset, "AdapterManagerReset"},
      {AdapterModemReset, "AdapterModemReset"},
      {AdapterSimManagerReset, "AdapterSimManagerReset"},
      {AdapterNetworkRegistrationReset, "AdapterNetworkRegistrationReset"},
      {AdapterConnectionManagerReset, "AdapterConnectionManagerReset"},
      {AdapterConnectionContextReset, "AdapterConnectionContextReset"},
      {Type::OfonoManagerModemAdded, "OfonoManagerModemAdded"},
      {Type::OfonoManagerModemRemoved, "OfonoManagerModemRemoved"},
      {Type::OfonoManagerGetModems, "OfonoGetModems"},
      {Type::OfonoModemGetProperties, "OfonoModemGetProperties"},
      {Type::OfonoModemPowered, "OfonoModemPowered"},
      {Type::OfonoModemOnline, "OfonoModemOnline"},
      {Type::OfonoModemLockdown, "OfonoModemLockdown"},
      {Type::OfonoModemInterfaces, "OfonoModemInterfaces"},
      {Type::OfonoModemInterfaceSimManagerAdded, "OfonoModemInterfaceSimManagerAdded"},
      {Type::OfonoModemInterfaceSimManagerRemoved, "OfonoModemInterfaceSimManagerRemoved"},
      {Type::OfonoModemInterfaceNetworkRegistrationAdded, "OfonoModemInterfaceNetworkRegistrationAdded"},
      {Type::OfonoModemInterfaceNetworkRegistrationRemoved, "OfonoModemInterfaceNetworkRegistrationRemoved"},
      {Type::OfonoModemInterfaceConnectionManagerAdded, "OfonoModemInterfaceConnectionManagerAdded"},
      {Type::OfonoModemInterfaceConnectionManagerRemoved, "OfonoModemInterfaceConnectionManagerRemoved"},
      {Type::OfonoModemManufacturer, "OfonoModemManufacturer"},
      {Type::OfonoModemModel, "OfonoModemModel"},
      {Type::OfonoModemSerial, "OfonoModemSerial"},
      {Type::OfonoSimManagerGetProperties, "OfonoSimManagerGetProperties"},
      {Type::OfonoSimManagerCardIdentifier, "OfonoSimManagerCardIdentifier"},
      {Type::OfonoSimManagerServiceProviderName, "OfonoSimManagerServiceProviderName"},
      {Type::OfonoNetworkRegistrationGetProperties, "OfonoNetworkRegistrationGetProperties"},
      {Type::OfonoNetworkRegistrationGetOperators, "OfonoNetworkRegistrationGetOperators"},
      {Type::OfonoNetworkRegistrationRegister, "OfonoNetworkRegistrationRegister"},
      {Type::OfonoNetworkRegistrationScan, "OfonoNetworkRegistrationScan"},
      {Type::OfonoNetworkRegistrationDeregister, "OfonoNetworkRegistrationDeregister"},
      {Type::OfonoNetworkRegistrationStatus, "OfonoNetworkRegistrationStatus"},
      {Type::OfonoNetworkRegistrationName, "OfonoNetworkRegistrationName"},
      {Type::OfonoNetworkRegistrationStrength, "OfonoNetworkRegistrationStrength"},
      {Type::OfonoConnectionManagerGetProperties, "OfonoConnectionManagerGetProperties"},
      {Type::OfonoConnectionManagerGetContexts, "OfonoConnectionManagerGetContexts"},
      {Type::OfonoConnectionManagerAddContext, "OfonoConnectionManagerAddContext"},
      {Type::OfonoConnectionManagerRemoveContext, "OfonoConnectionManagerRemoveContext"},
      {Type::OfonoConnectionManagerContextAdded, "OfonoConnectionManagerContextAdded"},
      {Type::OfonoConnectionManagerContextRemoved, "OfonoConnectionManagerContextRemoved"},
      {Type::OfonoConnectionManagerAttached, "OfonoConnectionManagerAttached"},
      {Type::OfonoConnectionManagerRoamingAllowed, "OfonoConnectionManagerRoamingAllowed"},
      {Type::OfonoConnectionManagerPowered, "OfonoConnectionManagerPowered"},
      {Type::OfonoConnectionContextGetProperties, "OfonoConnectionContextGetProperties"},
      {Type::OfonoConnectionContextActive, "OfonoConnectionContextActive"},
      {Type::OfonoConnectionContextSettings, "OfonoConnectionContextSettings"},
      {Type::OfonoConnectionContextAccessPointName, "OfonoConnectionContextAccessPointName"},
      {Type::OfonoConnectionContextUsername, "OfonoConnectionContextUsername"},
      {Type::OfonoConnectionContextPassword, "OfonoConnectionContextPassword"},
      {Type::OfonoConnectionContextType, "OfonoConnectionContextType"},
      {Type::OfonoConnectionContextAuthenticationMethod, "OfonoConnectionContextAuthenticationMethod"},
      {Type::OfonoConnectionContextProtocol, "OfonoConnectionContextProtocol"},
      {Type::OfonoConnectionContextName, "OfonoConnectionContextName"},
      {Type::OfonoConnectionContextInterface, "OfonoConnectionContextInterface"},
      {Type::OfonoConnectionContextMethod, "OfonoConnectionContextMethod"},
      {Type::OfonoConnectionContextAddress, "OfonoConnectionContextAddress"},
      {Type::OfonoConnectionContextNetmask, "OfonoConnectionContextNetmask"},
  };
  const QString msg = map.value(type);
  if (msg.isEmpty())
    throw astr_global::Exception("Unknown State::Type: " + QString::number(type));
  return msg;
}

QString State::toString(State::Status status)
{
  const QMap<Status, QString> map = {{Status::_EMPTYSTATUS_, "_EMPTYSTATUS_"},
                                     {Status::Signal, "Signal"},
                                     {Status::CallStarted, "CallStarted"},
                                     {Status::CallFinished, "CallFinished"},
                                     {Status::CallError, "CallError"}};
  const QString msg = map.value(status);
  if (msg.isEmpty())
    throw astr_global::Exception("Unknown State::Status: " + QString::number(status));
  return msg;
}

State::OfonoErrorType State::errorType(const QDBusError &error)
{
  switch (error.type())
  {
    case QDBusError::NoError: return OfonoErrorType::NoError;
    case QDBusError::Other:
    {
      const QString service = "org.ofono";
      const QMap<QString, OfonoErrorType> codes{
          {service + ".Error.InProgress", OfonoErrorType::InProgress},
          {service + ".Error.NotImplemented", OfonoErrorType::NotImplemented},
          {service + ".Error.InvalidArguments", OfonoErrorType::InvalidArguments},
          {service + ".Error.NotAvailable", OfonoErrorType::NotAvailable},
          {service + ".Error.AccessDenied", OfonoErrorType::AccessDenied},
          {service + ".Error.Failed", OfonoErrorType::Failed},
          {service + ".Error.InvalidFormat", OfonoErrorType::InvalidFormat},
          {service + ".Error.NotFound", OfonoErrorType::NotFound},
          {service + ".Error.NotAllowed", OfonoErrorType::NotAllowed},
          {service + ".Error.NotAttached", OfonoErrorType::NotAttached},
          {service + ".Error.AttachInProgress", OfonoErrorType::AttachInProgress}};
      return codes.value(error.name(), (error.message() == "Running another call") ? OfonoErrorType::RunningAnotherCall
                                                                                   : OfonoErrorType::DBusError);
    }
    default: return OfonoErrorType::DBusError;
  }
}

bool State::operator==(const State &state) const
{
  if (!(_type == state._type && _status == state._status))
    return false;

  if (true == _value.isValid() && true == state._value.isValid())
    return _value == state._value;

  return true;
}

State::operator QString() const
{
  if (errorType(error()) == State::OfonoErrorType::NoError)
  {
    return "(State::Type::" + toString(_type) + "|State::Status::" + toString(_status) +
           "|value=" + value().toString() + ")";
  }
  return "(State::Type::" + toString(_type) + "|State::Status::" + toString(_status) + "|value=" + value().toString() +
         "|error=" + error().name() + " : " + error().message();
}
