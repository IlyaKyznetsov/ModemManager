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

State::OfonoErrorType State::errorType(const QDBusError &error) const
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
      return codes.value(error.name(), OfonoErrorType::DBusError);
    }
    default: return OfonoErrorType::DBusError;
  }
}

bool State::operator==(const State &state) const
{
  //  DF() << *this << state;

  return _type == state._type && _status == state._status &&
         (_value.isValid() && state._value.isValid() ? _value == state._value : true);
}

State::operator QString() const
{
  const QMap<Status, QString> statuses = {{Status::Signal, "Signal"},
                                          {Status::CallStarted, "CallStarted"},
                                          {Status::CallFinished, "CallFinished"},
                                          {Status::CallError, "CallError"}};

  const QMap<Type, QString> types = {
      {Type::_EMPTYTYPE_, "_EMPTYTYPE_"},
      {Type::Reset, "Reset"},
      {Type::OfonoServiceRegistered, "OfonoServiceRegistered"},
      {Type::OfonoServiceUnregistered, "OfonoServiceUnregistered"},
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
      {Type::OfonoConnectionManagerContextAdded, "OfonoConnectionManagerContextAdded"},
      {Type::OfonoConnectionManagerContextRemoved, "OfonoConnectionManagerContextRemoved"},
      {Type::OfonoConnectionManagerAttached, "OfonoConnectionManagerAttached"},
      {Type::OfonoConnectionManagerRoamingAllowed, "OfonoConnectionManagerRoamingAllowed"},
      {Type::OfonoConnectionManagerPowered, "OfonoConnectionManagerPowered"},
      {Type::OfonoConnectionContextGetProperties, "OfonoConnectionContextGetProperties"},
      {Type::OfonoConnectionContextActive, "OfonoConnectionContextActive"},
      {Type::OfonoConnectionContextSettings, "Type::OfonoConnectionContextSettings"},
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
      {Type::OfonoConnectionContextNetmask, "OfonoConnectionContextNetmask"}};

  QString type = types.value(_type);
  if (type.isEmpty())
    throw astr_global::Exception("Bad State::Type: " + QString::number(_type));

  return "(State::Type::" + type + "|State::Status::" + statuses.value(_status, "Invalid") +
         "|value=" + value().toString() + ")";
}
