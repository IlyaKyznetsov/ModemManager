#include "Manager.h"
#include "ofono_manager_interface.h"

Manager::Manager(QObject *parent) : QObject(parent), _interface(nullptr)
{
}

bool Manager::isValid() const
{
  return _interface && _interface->isValid();
}

void Manager::reset(const QString &service)
{
  Q_EMIT StateChanged(State(State::Reset, State::Started));
  if (service.isEmpty())
  {
    delete _interface;
    _interface = nullptr;
    Q_EMIT StateChanged(State(State::Reset, State::Finished));
    return;
  }

  OfonoManagerInterface *interface =
  new OfonoManagerInterface(service, "/", QDBusConnection::systemBus(), this);
  if (!interface->isValid())
  {
    delete interface;
    Q_EMIT StateChanged(State(State::Reset, State::Finished));
    return;
  }

  _interface = interface;
  connect(_interface, &OfonoManagerInterface::ModemAdded, [this](const QDBusObjectPath &in0) {
    const QString &path = in0.path();
    if (!_modemsPath.contains(path))
    {
      _modemsPath.append(path);
      Q_EMIT StateChanged(State(State::OfonoModemAdded, State::Signal, QVariant(path)));
    }
  });

  connect(_interface, &OfonoManagerInterface::ModemRemoved, [this](const QDBusObjectPath &in0) {
    const QString &path = in0.path();
    _modemsPath.removeAll(path);
    Q_EMIT StateChanged(State(State::OfonoModemRemoved, State::Signal, QVariant(path)));
  });

  Q_EMIT StateChanged(State(State::OfonoGetModems, State::Started));
  connect(new QDBusPendingCallWatcher(_interface->GetModems(), _interface),
          &QDBusPendingCallWatcher::finished, [this](QDBusPendingCallWatcher *watcher) {
            DF();
            QDBusPendingReply<ObjectPathPropertiesList> reply(*watcher);
            watcher->deleteLater();
            if (reply.isError())
            {
              Q_EMIT StateChanged(State(State::OfonoGetModems, State::Error, reply.error()));
            }
            else
            {

              const ObjectPathPropertiesList &properties = reply.value();
              for (const ObjectPathProperties &item : properties)
              {
                const QString &path = item.path.path();
                if (!_modemsPath.contains(path)) _modemsPath.append(path);
              }
              Q_EMIT StateChanged(State(State::OfonoGetModems, State::Finished));
              if (!_modemsPath.isEmpty())
                Q_EMIT StateChanged(
                State(State::OfonoModemAdded, State::Signal, QVariant(_modemsPath.last())));
            }
          });
  Q_EMIT StateChanged(State(State::Reset, State::Finished));
}
