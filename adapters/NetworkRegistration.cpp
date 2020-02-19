#include "NetworkRegistration.h"
#include "ofono_network_registration_interface.h"

static const QMap<QString, State::Type> StringToType{{"Name", State::OfonoNetworkRegistrationName},
                                                     {"Status", State::OfonoNetworkRegistrationStatus},
                                                     {"Strength", State::OfonoNetworkRegistrationStrength}};

NetworkRegistration::NetworkRegistration(const int &dbusTimeout, QObject *parent)
    : QObject(parent), _dbusTimeout(dbusTimeout), _interface(nullptr), _currentCallType(State::_EMPTYTYPE_)
{
}

bool NetworkRegistration::isValid() const
{
  return _interface && _interface->isValid();
}

void NetworkRegistration::reset(const QString &path)
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

  OfonoNetworkRegistrationInterface *interface =
      new OfonoNetworkRegistrationInterface(Ofono::SERVICE, path, QDBusConnection::systemBus(), this);
  if (!interface->isValid())
  {
    delete interface;
    Q_EMIT StateChanged(State(State::Reset, State::CallFinished));
    return;
  }

  _interface = interface;
  _interface->setTimeout(_dbusTimeout);
  connect(_interface, &OfonoNetworkRegistrationInterface::PropertyChanged,
          [this](const QString &in0, const QDBusVariant &in1) {
            State::Type type = StringToType.value(in0, State::_EMPTYTYPE_);
            switch (type)
            {
              case State::_EMPTYTYPE_: break;
              case State::OfonoNetworkRegistrationStrength:
              {
                bool isOk = false;
                int percent = in1.variant().toInt(&isOk);
                Q_EMIT StateChanged(State(type, State::Signal, (isOk ? QString::number(percent) : QString())));
              }
              break;
              default: Q_EMIT StateChanged(State(type, State::Signal, in1.variant())); break;
            }
          });

  Q_EMIT StateChanged(State(State::OfonoNetworkRegistrationGetProperties, State::CallStarted));
  connect(new QDBusPendingCallWatcher(_interface->GetProperties(), _interface), &QDBusPendingCallWatcher::finished,
          [this](QDBusPendingCallWatcher *watcher) {
            QDBusPendingReply<QVariantMap> reply(*watcher);
            watcher->deleteLater();
            if (reply.isError())
            {
              Q_EMIT StateChanged(State(State::OfonoNetworkRegistrationGetProperties, State::CallError, reply.error()));
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
                  case State::OfonoNetworkRegistrationStrength:
                  {
                    bool isOk = false;
                    int percent = (*iterator).second.toInt(&isOk);
                    Q_EMIT StateChanged(State(type, State::Signal, (isOk ? QString::number(percent) : QString())));
                  }
                  break;
                  default: Q_EMIT StateChanged(State(type, State::Signal, (*iterator).second)); break;
                }
              }
              Q_EMIT StateChanged(State(State::OfonoNetworkRegistrationGetProperties, State::CallFinished));
            }
          });
  Q_EMIT StateChanged(State(State::Reset, State::CallFinished));
}

void NetworkRegistration::call(const State::Type type)
{
  const QVector<State::Type> types{State::OfonoNetworkRegistrationRegister, State::OfonoNetworkRegistrationScan,
                                   State::OfonoNetworkRegistrationDeregister,
                                   State::OfonoNetworkRegistrationGetOperators};
  if (!types.contains(type))
    throw astr_global::Exception("Неверный тип call");

  Q_EMIT StateChanged(State(type, State::CallStarted));
  if (State::_EMPTYTYPE_ != _currentCallType)
    Q_EMIT StateChanged(State(type, State::CallError, QDBusError(QDBusError::Other, "Running another call")));

  _currentCallType = type;
  switch (type)
  {
    case State::OfonoNetworkRegistrationRegister:
    {
      connect(new QDBusPendingCallWatcher(_interface->Register(), _interface), &QDBusPendingCallWatcher::finished,
              [this](QDBusPendingCallWatcher *watcher) {
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
    case State::OfonoNetworkRegistrationDeregister:
    {
      connect(new QDBusPendingCallWatcher(_interface->Deregister(), _interface), &QDBusPendingCallWatcher::finished,
              [this](QDBusPendingCallWatcher *watcher) {
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
    case State::OfonoNetworkRegistrationScan:
    {
      connect(new QDBusPendingCallWatcher(_interface->Scan(), _interface), &QDBusPendingCallWatcher::finished,
              [this](QDBusPendingCallWatcher *watcher) {
                QDBusPendingReply<ObjectPathPropertiesList> reply(*watcher);
                watcher->deleteLater();
                State::Type type = _currentCallType;
                _currentCallType = State::_EMPTYTYPE_;
                if (reply.isError())
                {
                  Q_EMIT StateChanged(State(type, State::CallError, reply.error()));
                }
                else
                {
                  for (const ObjectPathProperties &item : reply.value())
                  {
                    D("OPERATOR:" << item.path.path());
                  }

                  Q_EMIT StateChanged(State(type, State::CallFinished));
                }
              });
    }
    break;
    case State::OfonoNetworkRegistrationGetOperators:
    {
      Q_EMIT StateChanged(State(State::OfonoNetworkRegistrationGetOperators, State::CallStarted));
      throw astr_global::Exception("Не реализовано");
    }
    break;
    default: throw astr_global::Exception("Неверный тип");
  }
}
