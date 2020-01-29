#include "OfonoManager.h"
#include "dbustypes.h"
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QDBusServiceWatcher>

OfonoManager::OfonoManager(QObject *parent) : QObject(parent), _watcher(nullptr)
{
  Ofono::registerObjectPathProperties();
}

void OfonoManager::reset(const QString &service)
{
  Q_EMIT StateChanged(State(State::Reset, State::Started));
  if (service.isEmpty())
  {
    delete _watcher;
    _watcher = nullptr;
    Q_EMIT StateChanged(State(State::Reset, State::Finished));
    return;
  }

  QDBusConnection bus(QDBusConnection::systemBus());
  _watcher = new QDBusServiceWatcher(service, bus,
                                     QDBusServiceWatcher::WatchForRegistration | QDBusServiceWatcher::WatchForUnregistration,
                                     this);
  connect(_watcher, &QDBusServiceWatcher::serviceRegistered,
          [this]() { Q_EMIT StateChanged(State(State::OfonoServiceRegistered, State::Signal)); });
  connect(_watcher, &QDBusServiceWatcher::serviceUnregistered,
          [this]() { Q_EMIT StateChanged(State(State::OfonoServiceUnregistered, State::Signal)); });
  Q_EMIT StateChanged(State(State::Reset, State::Finished));
  if (bus.interface()->isServiceRegistered(service))
    Q_EMIT StateChanged(State(State::OfonoServiceRegistered, State::Signal));
}
