#ifndef MODEMMANAGERDATA_H
#define MODEMMANAGERDATA_H

#include <QMap>
#include <QSharedPointer>
#include <QString>

class Manager;
class Modem;
class SimManager;
class NetworkRegistration;
class ConnectionManager;
class ConnectionContext;

namespace ModemManagerData
{
struct Settings
{
  struct DBusTimeouts
  {
    int manager;
    int modem;
    int simManager;
    int networkRegistration;
    int connectionManager;
    int connectionContext;
    DBusTimeouts(const int &Manager, const int &Modem, const int &SimManager, const int &NetworkRegistration,
                 const int &ConnectionManager, const int &ConnectionContext);
  };
  struct ModemManagerTimeouts
  {
    int deferredCall;
    int waitState;
    ModemManagerTimeouts(const int &DeferredCall, const int &WaitState);
  };
  struct Provider
  {
    QString accessPointName;
    QString username;
    QString password;
    Provider(const QString &AccessPointName = QString(), const QString &Username = QString(),
             const QString &Password = QString());
  };
  Settings(const Settings::DBusTimeouts &DBusTimeouts, const Settings::ModemManagerTimeouts &ModemManagerTimeouts);
  bool addProvider(const QString &name, const Settings::Provider &settings);
  Provider providerSettings(const QString &provider) const;
  void debug() const;

  const DBusTimeouts dBusTimeouts;
  const ModemManagerTimeouts modemManagerimeouts;
  QMap<QString, Provider> providers;
};

struct OfonoState
{
  struct Modem
  {
    bool lockdown = false;
    bool powered = false;
    bool online = false;
    QString manufacturer; // readonly, optional
    QString model;        // readonly, optional
    QString serial;       // readonly, optional
  };
  struct SimManager
  {
    QString cardIdentifier;      // readonly
    QString serviceProviderName; // readonly, optional
  };
  struct NetworkRegistration
  {
    QString status;   // [readonly]
                      // "unregistered"  Not registered to any network
                      // "registered"    Registered to home network
                      // "searching"     Not registered, but searching
                      // "denied"        Registration has been denied
                      // "unknown"       Status is unknown
                      // "roaming"       Registered, but roaming
    QString name;     // [readonly]
    QString strength; // [readonly, optional between 0-100 percent]
  };
  struct ConnectionManager
  {
    bool attached = false;       // [readonly]
    bool roamingAllowed = false; // [readwrite]
    bool powered = false;        // [readwrite]
    // QString connectionContextPath;
  };
  struct ConnectionContext
  {
    bool active = false; // [readwrite]
    QString accessPointName;
    QString username;
    QString password;
    QString type;
    QString authenticationMethod;
    QString protocol;
    QString name;
    QString interface; // [readonly, optional]
    QString method;    // [readonly, optional]
    QString address;   // [readonly, optional]
    QString netmask;   // [readonly, optional]
  };
  bool isOfonoConnected = false;
  QSharedPointer<Modem> modem;
  QSharedPointer<SimManager> simManager;
  QSharedPointer<NetworkRegistration> networkRegistration;
  QSharedPointer<ConnectionManager> connectionManager;
  QSharedPointer<ConnectionContext> connectionContext;
};

enum AutoConnectionState
{
  TERMINATE = -1,
  STOP,
  START,
  MODEMGETPROPERTIES,
  MODEMGETLOCKDOWN,
};

} // namespace ModemManagerData

#include <qmetatype.h>
Q_DECLARE_METATYPE(ModemManagerData::OfonoState)

#endif // MODEMMANAGERDATA_H
