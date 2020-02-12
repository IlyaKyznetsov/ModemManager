#include "ModemManagerData.h"
#include "Global.h"

ModemManagerData::Settings::DBusTimeouts::DBusTimeouts(const int &Manager, const int &Modem, const int &SimManager,
                                                       const int &NetworkRegistration, const int &ConnectionManager,
                                                       const int &ConnectionContext)
    : manager(Manager),
      modem(Modem),
      simManager(SimManager),
      networkRegistration(NetworkRegistration),
      connectionManager(ConnectionManager),
      connectionContext(ConnectionContext)
{
}

ModemManagerData::Settings::ModemManagerTimeouts::ModemManagerTimeouts(const int &DeferredCall, const int &WaitState)
    : deferredCall(DeferredCall), waitState(WaitState)
{
}

ModemManagerData::Settings::Provider::Provider(const QString &AccessPointName, const QString &Username,
                                               const QString &Password)
    : accessPointName(AccessPointName), username(Username), password(Password)
{
}

ModemManagerData::Settings::Settings(const ModemManagerData::Settings::DBusTimeouts &DBusTimeouts,
                                     const ModemManagerData::Settings::ModemManagerTimeouts &ModemManagerTimeouts)
    : dBusTimeouts(DBusTimeouts), modemManagerimeouts(ModemManagerTimeouts)
{
}

bool ModemManagerData::Settings::addProvider(const QString &name, const ModemManagerData::Settings::Provider &settings)
{
  return providers.insert(name, settings) != providers.end();
}

ModemManagerData::Settings::Provider ModemManagerData::Settings::providerSettings(const QString &provider) const
{
  return providers.value(provider, Provider(QString(), QString(), QString()));
}

void ModemManagerData::Settings::debug() const
{
  D(QString("DBus: manager %1 | modem %2 | simManager %3 | networkRegistration %4 | connectionManager %5 | "
            "connectionContext %6")
        .arg(QString::number(dBusTimeouts.manager), QString::number(dBusTimeouts.modem),
             QString::number(dBusTimeouts.simManager), QString::number(dBusTimeouts.networkRegistration),
             QString::number(dBusTimeouts.connectionManager), QString::number(dBusTimeouts.connectionContext)));
  D(QString("ModemManager: deferredCall %1 | waitState %2")
        .arg(QString::number(modemManagerimeouts.deferredCall), QString::number(modemManagerimeouts.waitState)));

  for (auto iterator = providers.keyValueBegin(); iterator != providers.keyValueEnd(); ++iterator)
  {
    const ModemManagerData::Settings::Provider &p = (*iterator).second;
    D((*iterator).first << ":" << p.accessPointName << p.username << p.password);
  }
}
