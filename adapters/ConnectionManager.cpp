#include "ConnectionManager.h"
#include "ofono_connection_manager_interface.h"

static const QMap<QString, State::Type> StringToType{{"Attached", State::OfonoConnectionManagerAttached},
                                                     {"RoamingAllowed", State::OfonoConnectionManagerRoamingAllowed},
                                                     {"Powered", State::OfonoConnectionManagerPowered}};

ConnectionManager::ConnectionManager(const int &dbusTimeout, QObject *parent)
    : QObject(parent), _dbusTimeout(dbusTimeout), _interface(nullptr), _currentCallType(State::_EMPTYTYPE_)
{
}

bool ConnectionManager::isValid() const
{
  return _interface && _interface->isValid();
}

QString ConnectionManager::contextPath() const
{
  return (_contextsPath.isEmpty() ? QString() : _contextsPath.first());
}

bool ConnectionManager::isContext(const QString &path) const
{
  return _contextsPath.contains(path);
}

void ConnectionManager::reset(const QString &path)
{
  Q_EMIT StateChanged(State(State::Reset, State::CallStarted));

  _currentCallType = State::_EMPTYTYPE_;

  if (path.isEmpty())
  {
    delete _interface;
    _interface = nullptr;
    Q_EMIT StateChanged(State(State::Reset, State::CallFinished));
    return;
  }

  OfonoConnectionManagerInterface *interface =
      new OfonoConnectionManagerInterface(Ofono::SERVICE, path, QDBusConnection::systemBus(), this);
  if (!interface->isValid())
  {
    delete interface;
    Q_EMIT StateChanged(State(State::Reset, State::CallFinished));
    return;
  }

  _interface = interface;
  _interface->setTimeout(_dbusTimeout);
  connect(_interface, &OfonoConnectionManagerInterface::PropertyChanged,
          [this](const QString &in0, const QDBusVariant &in1) {
            State::Type type = StringToType.value(in0, State::_EMPTYTYPE_);
            switch (type)
            {
              case State::_EMPTYTYPE_: break;
              default: Q_EMIT StateChanged(State(type, State::Signal, in1.variant())); break;
            }
          });

  connect(_interface, &OfonoConnectionManagerInterface::ContextAdded, [this](const QDBusObjectPath &in0) {
    const QString &path = in0.path();
    if (!_contextsPath.contains(path))
    {
      _contextsPath.append(path);
      Q_EMIT StateChanged(State(State::OfonoConnectionManagerContextAdded, State::Signal, QVariant(path)));
    }
  });

  connect(_interface, &OfonoConnectionManagerInterface::ContextRemoved, [this](const QDBusObjectPath &in0) {
    const QString &path = in0.path();
    _contextsPath.removeAll(path);
    Q_EMIT StateChanged(State(State::OfonoConnectionManagerContextRemoved, State::Signal, QVariant(path)));
    if (!_contextsPath.isEmpty())
      Q_EMIT StateChanged(
          State(State::OfonoConnectionManagerContextAdded, State::Signal, QVariant(_contextsPath.first())));
  });

  _getProperties();

  Q_EMIT StateChanged(State(State::State::Reset, State::CallFinished));
}

void ConnectionManager::_getProperties()
{
  Q_EMIT StateChanged(State(State::OfonoConnectionManagerGetProperties, State::CallStarted));
  connect(new QDBusPendingCallWatcher(_interface->GetProperties(), _interface), &QDBusPendingCallWatcher::finished,
          [this](QDBusPendingCallWatcher *watcher) {
            QDBusPendingReply<QVariantMap> reply(*watcher);
            watcher->deleteLater();
            if (reply.isError())
            {
              Q_EMIT StateChanged(State(State::OfonoConnectionManagerGetProperties, State::CallError, reply.error()));
            }
            else
            {
              const QVariantMap &properties = reply.value();
              for (auto iterator = properties.keyValueBegin(); iterator != properties.keyValueEnd(); ++iterator)
              {
                State::Type type = StringToType.value((*iterator).first, State::_EMPTYTYPE_);
                switch (type)
                {
                  case State::_EMPTYTYPE_: break;
                  default: Q_EMIT StateChanged(State(type, State::Signal, (*iterator).second)); break;
                }
              }
              Q_EMIT StateChanged(State(State::OfonoConnectionManagerGetProperties, State::CallFinished));
              _getContexts();
            }
          });
}

void ConnectionManager::_getContexts()
{
  Q_EMIT StateChanged(State(State::OfonoConnectionManagerGetContexts, State::CallStarted));
  connect(new QDBusPendingCallWatcher(_interface->GetContexts(), _interface), &QDBusPendingCallWatcher::finished,
          [this](QDBusPendingCallWatcher *watcher) {
            QDBusPendingReply<ObjectPathPropertiesList> reply = watcher->reply();
            watcher->deleteLater();
            if (reply.isError())
            {
              Q_EMIT StateChanged(State(State::OfonoConnectionManagerGetContexts, State::CallError, reply.error()));
            }
            else
            {
              const ObjectPathPropertiesList &contexts = reply.value();
              for (const ObjectPathProperties &item : contexts)
              {
                const QString &path = item.path.path();
                if (!_contextsPath.contains(path))
                  _contextsPath.append(path);
              }
              Q_EMIT StateChanged(State(State::OfonoConnectionManagerGetContexts, State::CallFinished));
              if (!_contextsPath.isEmpty())
                Q_EMIT StateChanged(
                    State(State::OfonoConnectionManagerContextAdded, State::Signal, QVariant(_contextsPath.first())));
            }
          });
}

void ConnectionManager::call(const State::Type type, const QVariant &value)
{
  DF() << type << value;

  Q_EMIT StateChanged(State(type, State::CallStarted, value));
  if (State::_EMPTYTYPE_ != _currentCallType)
    Q_EMIT StateChanged(State(type, State::CallError, "Running another call"));

  switch (type)
  {
    case State::OfonoConnectionManagerRoamingAllowed:
    {
      _currentCallType = type;
      connect(new QDBusPendingCallWatcher(_interface->SetProperty("RoamingAllowed", QDBusVariant(value)), _interface),
              &QDBusPendingCallWatcher::finished, [this](QDBusPendingCallWatcher *watcher) {
                QDBusPendingReply<> reply(*watcher);
                watcher->deleteLater();
                State::Type type = _currentCallType;
                _currentCallType = State::_EMPTYTYPE_;
                if (reply.isError())
                {
                  Q_EMIT StateChanged(State(type, State::CallError, reply.error()));
                }
                else
                {
                  Q_EMIT StateChanged(State(type, State::CallFinished));
                }
              });
    }
    break;
    case State::OfonoConnectionManagerPowered:
    {
      _currentCallType = type;
      connect(new QDBusPendingCallWatcher(_interface->SetProperty("Powered", QDBusVariant(value)), _interface),
              &QDBusPendingCallWatcher::finished, [this](QDBusPendingCallWatcher *watcher) {
                QDBusPendingReply<> reply(*watcher);
                watcher->deleteLater();
                State::Type type = _currentCallType;
                _currentCallType = State::_EMPTYTYPE_;
                if (reply.isError())
                {
                  Q_EMIT StateChanged(State(type, State::CallError, reply.error()));
                }
                else
                {
                  Q_EMIT StateChanged(State(type, State::CallFinished));
                }
              });
    }
    break;
    case State::OfonoConnectionManagerAddContext:
    {
      _currentCallType = type;
      connect(new QDBusPendingCallWatcher(_interface->AddContext(value.toString()), _interface),
              &QDBusPendingCallWatcher::finished, [this](QDBusPendingCallWatcher *watcher) {
                QDBusPendingReply<QDBusObjectPath> reply(*watcher);
                watcher->deleteLater();
                State::Type type = _currentCallType;
                _currentCallType = State::_EMPTYTYPE_;
                if (reply.isError())
                {
                  Q_EMIT StateChanged(State(type, State::CallError, reply.error()));
                }
                else
                {
#warning проверить, что приходит сигнал о добавлении контекста
                  Q_EMIT StateChanged(State(type, State::CallFinished));
                }
              });
    }
    break;
    case State::OfonoConnectionManagerRemoveContext:
    {
      _currentCallType = type;
      connect(new QDBusPendingCallWatcher(_interface->RemoveContext(QDBusObjectPath(value.toString())), _interface),
              &QDBusPendingCallWatcher::finished, [this](QDBusPendingCallWatcher *watcher) {
                QDBusPendingReply<> reply(*watcher);
                watcher->deleteLater();
                State::Type type = _currentCallType;
                _currentCallType = State::_EMPTYTYPE_;
                if (reply.isError())
                {
                  Q_EMIT StateChanged(State(type, State::CallError, reply.error()));
                }
                else
                {
#warning проверить, что приходит сигнал о удалении контекста
                  Q_EMIT StateChanged(State(type, State::CallFinished));
                }
              });
    }
    break;
    default: throw astr_global::Exception("Неверный тип call");
  }
}
