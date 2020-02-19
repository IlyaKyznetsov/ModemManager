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

ModemManagerData::Settings::AutomatorTimeouts::AutomatorTimeouts(const int &ConnectionManagerAttached,
                                                                 const int &ErrorRepeat)
    : connectionManagerAttached(ConnectionManagerAttached), errorRepeat(ErrorRepeat)
{
}

ModemManagerData::Settings::Provider::Provider(const QVariant &AccessPointName, const QVariant &Username,
                                               const QVariant &Password)
    : accessPointName(AccessPointName), username(Username), password(Password)
{
}

ModemManagerData::Settings::Settings(const ModemManagerData::Settings::DBusTimeouts &DBusTimeouts,
                                     const ModemManagerData::Settings::AutomatorTimeouts &AutomatorTimeouts)
    : dBusTimeouts(DBusTimeouts), automatorTimeouts(AutomatorTimeouts)
{
}

bool ModemManagerData::Settings::addProvider(const QString &name, const ModemManagerData::Settings::Provider &settings)
{
  return providers.insert(name, settings) != providers.end();
}

ModemManagerData::Settings::Provider ModemManagerData::Settings::providerSettings(const QString &provider) const
{
  return providers.value(provider, Provider());
}

void ModemManagerData::Settings::debug() const
{
  D(QString("DBus: manager %1 | modem %2 | simManager %3 | networkRegistration %4 | connectionManager %5 | "
            "connectionContext %6")
        .arg(QString::number(dBusTimeouts.manager), QString::number(dBusTimeouts.modem),
             QString::number(dBusTimeouts.simManager), QString::number(dBusTimeouts.networkRegistration),
             QString::number(dBusTimeouts.connectionManager), QString::number(dBusTimeouts.connectionContext)));
  D(QString("ModemManager: ConnectionManagerAttached %1 | ErrorRepeat %2")
        .arg(QString::number(automatorTimeouts.connectionManagerAttached),
             QString::number(automatorTimeouts.errorRepeat)));

  for (auto iterator = providers.keyValueBegin(); iterator != providers.keyValueEnd(); ++iterator)
  {
    const ModemManagerData::Settings::Provider &p = (*iterator).second;
    D((*iterator).first << ":" << p.accessPointName << p.username << p.password);
  }
}
