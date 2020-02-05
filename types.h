#ifndef TYPES_H
#define TYPES_H

#include <QVariant>
#include <QtDBus/QDBusError>

#include "Global.h"
namespace Ofono
{
static const QString SERVICE("org.ofono");
}

class State
{
public:
  enum Type
  {
    _EMPTYTYPE_ = 0, // for default constructor only
    // Local,
    Reset,
    // Ofono Service
    OfonoServiceRegistered,
    OfonoServiceUnregistered,
    // Ofono Manager
    OfonoManagerModemAdded,
    OfonoManagerModemRemoved,
    OfonoManagerGetModems,
    // OfonoModem
    OfonoModemGetProperties, // call
    OfonoModemPowered,       // [readwrite]
    OfonoModemOnline,        // [readwrite]
    OfonoModemLockdown,      // [readwrite]
    OfonoModemInterfaces,
    OfonoModemInterfaceSimManagerAdded,
    OfonoModemInterfaceSimManagerRemoved,
    OfonoModemInterfaceNetworkRegistrationAdded,
    OfonoModemInterfaceNetworkRegistrationRemoved,
    OfonoModemInterfaceConnectionManagerAdded,
    OfonoModemInterfaceConnectionManagerRemoved,
    OfonoModemManufacturer, // [readonly, optional]
    OfonoModemModel,        // [readonly, optional]
    OfonoModemSerial,       // [readonly, optional]
    // OfonoSimManager
    OfonoSimManagerGetProperties,
    OfonoSimManagerCardIdentifier,      // [readonly]
    OfonoSimManagerServiceProviderName, // [readonly, optional]
    // OfonoNetworkRegistration
    OfonoNetworkRegistrationGetProperties,
    OfonoNetworkRegistrationGetOperators,
    OfonoNetworkRegistrationRegister,
    OfonoNetworkRegistrationScan,
    OfonoNetworkRegistrationDeregister,
    OfonoNetworkRegistrationStatus, //[readonly: "unregistered" "registered" "searching" "denied" "unknown" "roaming"];
    OfonoNetworkRegistrationName,   //[readonly];
    OfonoNetworkRegistrationStrength, //[readonly, optional between 0-100 percent]
    // ConnectionManager
    OfonoConnectionManagerGetProperties,
    OfonoConnectionManagerGetContexts,
    OfonoConnectionManagerAddContext,
    OfonoConnectionManagerRemoveContext,
    OfonoConnectionManagerContextAdded,
    OfonoConnectionManagerContextRemoved,
    OfonoConnectionManagerAttached,       // [readonly]
    OfonoConnectionManagerRoamingAllowed, // [readwrite]
    OfonoConnectionManagerPowered,        // [readwrite]
    //    --- OfonoConnectionManagerContexts,
    OfonoConnectionContextGetProperties,
    OfonoConnectionContextActive,               // [readwrite]
    OfonoConnectionContextAccessPointName,      // [readwrite]
    OfonoConnectionContextUsername,             // [readwrite]
    OfonoConnectionContextPassword,             // [readwrite]
    OfonoConnectionContextType,                 // [readwrite]
    OfonoConnectionContextAuthenticationMethod, // [readwrite]
    OfonoConnectionContextProtocol,             // [readwrite]
    OfonoConnectionContextName,                 // [readwrite]
    OfonoConnectionContextSettings,
    OfonoConnectionContextInterface, // [readonly, optional]
    OfonoConnectionContextMethod,    // [readonly, optional]
    OfonoConnectionContextAddress,   // [readonly, optional]
    OfonoConnectionContextNetmask    // [readonly, optional]
  };

  enum Status
  {
    _EMPTYSTATUS_ = 0,
    Signal,
    CallStarted,
    CallFinished,
    CallError
  };

  enum OfonoErrorType
  {
    NoError = 0,
    DBusError,
    InProgress,
    NotImplemented,
    InvalidArguments,
    NotAvailable,
    AccessDenied,
    Failed,
    InvalidFormat,
    NotFound,
    NotAllowed,
    NotAttached,
    AttachInProgress
  };

  State();
  State(Type type, Status status);
  State(Type type, Status status, const QVariant &value);
  State(Type type, Status status, const QDBusError &error);
  State(Type type, Status status, const QVariant &value, const QDBusError &error);
  Type type() const { return _type; }
  Status status() const { return _status; }
  QVariant value() const { return _value; }
  QDBusError error() const { return _error; }
  OfonoErrorType errorType(const QDBusError &error) const;
  bool operator==(const State &state) const;
  operator QString() const;

private:
  Type _type;
  Status _status;
  QVariant _value;
  QDBusError _error;
};

#include <qmetatype.h>
Q_DECLARE_METATYPE(State)

#endif // TYPES_H
