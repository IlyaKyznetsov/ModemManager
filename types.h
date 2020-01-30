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
    OfonoNetworkRegistrationStatus, //[readonly: "unregistered" "registered" "searching" "denied" "unknown" "roaming"];
    OfonoNetworkRegistrationName,   //[readonly];
    OfonoNetworkRegistrationStrength, //[readonly, optional between 0-100 percent]
    // ConnectionManager
    OfonoConnectionManagerAttached,       // [readonly]
    OfonoConnectionManagerRoamingAllowed, // [readwrite]
    OfonoConnectionManagerPowered,        // [readwrite]
    //    --- OfonoConnectionManagerContexts,
    // ConnectionContext
    OfonoConnectionContextActive,               // [readwrite]
    OfonoConnectionContextAccessPointName,      // [readwrite]
    OfonoConnectionContextUsername,             // [readwrite]
    OfonoConnectionContextPassword,             // [readwrite]
    OfonoConnectionContextType,                 // [readwrite]
    OfonoConnectionContextAuthenticationMethod, // [readwrite]
    OfonoConnectionContextProtocol,             // [readwrite]
    OfonoConnectionContextName,                 // [readwrite]
    OfonoConnectionContextInterface,            // [readonly, optional]
    OfonoConnectionContextMethod,               // [readonly, optional]
    OfonoConnectionContextAddress,              // [readonly, optional]
    OfonoConnectionContextNetmask               // [readonly, optional]
  } _type;

  enum Status
  {
    _EMPTYSTATUS_ = 0,
    Signal,
    CallStarted,
    CallFinished,
    CallError
  } _status;

  QVariant _value;
  QDBusError _error;

  State() : _type(_EMPTYTYPE_), _status(_EMPTYSTATUS_)
  {
    // _error.type() = NoError
    // _value = QVariant(Invalid);
  }

  State(Type type, Status state) : _type(type), _status(state) {}

  State(Type type, Status state, const QVariant &value) : _type(type), _status(state), _value(value) {}

  State(Type type, Status state, const QDBusError &error) : _type(type), _status(state), _error(error) {}

  State(Type type, Status state, const QVariant &value, const QDBusError &error)
      : _type(type), _status(state), _value(value), _error(error)
  {
  }

  operator QString() const
  {
    const QMap<Status, QString> statuses = {{Status::Signal, "Signal"},
                                            {Status::CallStarted, "CallStarted"},
                                            {Status::CallFinished, "CallFinished"},
                                            {Status::CallError, "CallError"}};

    const QMap<Type, QString> types = {
        {Type::Reset, "Reset"},
        {Type::OfonoServiceRegistered, "OfonoServiceRegistered"},
        {Type::OfonoServiceUnregistered, "OfonoServiceUnregistered"},
        {Type::OfonoManagerModemAdded, "OfonoModemAdded"},
        {Type::OfonoManagerModemRemoved, "Ofono::ModemRemoved"},
        {Type::OfonoManagerGetModems, "Ofono::GetModems"},
        {Type::OfonoModemGetProperties, "Ofono::ModemGetProperties"},
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
        {Type::OfonoSimManagerGetProperties, "OfonoModemSerial"},
        {Type::OfonoSimManagerCardIdentifier, "OfonoSimManagerSimCardIdentifier"},
        {Type::OfonoSimManagerServiceProviderName, "OfonoSimManagerSimServiceProviderName"},
        {Type::OfonoNetworkRegistrationStatus, "OfonoNetworkRegistrationStatus"},
        {Type::OfonoNetworkRegistrationName, "OfonoNetworkRegistrationName"},
        {Type::OfonoNetworkRegistrationStrength, "OfonoNetworkRegistrationStrength"},
        {Type::OfonoConnectionManagerAttached, "OfonoConnectionManagerAttached"},
        {Type::OfonoConnectionManagerRoamingAllowed, "OfonoConnectionManagerRoamingAllowed"},
        {Type::OfonoConnectionManagerPowered, "OfonoConnectionManagerPowered"},
        {Type::OfonoConnectionContextActive, "OfonoConnectionContextActive"},
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

    return "(Type:" + type + "|Status:" + statuses.value(_status, "Invalid") + ")";
  }
};

#include <qmetatype.h>
Q_DECLARE_METATYPE(State)

#endif // TYPES_H
