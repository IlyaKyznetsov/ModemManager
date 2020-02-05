#include "SimManager.h"
#include "ofono_simmanager_interface.h"

static const QMap<QString, State::Type> StringToType{
    {"CardIdentifier", State::OfonoSimManagerCardIdentifier},
    {"ServiceProviderName", State::OfonoSimManagerServiceProviderName}};

SimManager::SimManager(const int &dbusTimeout, QObject *parent)
    : QObject(parent), _dbusTimeout(dbusTimeout), _interface(nullptr)
{
}

bool SimManager::isValid() const
{
  return _interface && _interface->isValid();
}

void SimManager::reset(const QString &path)
{
  Q_EMIT StateChanged(State(State::Reset, State::CallStarted));

  if (path.isEmpty())
  {
    delete _interface;
    _interface = nullptr;
    Q_EMIT StateChanged(State(State::Reset, State::CallFinished));
    return;
  }

  OfonoSimManagerInterface *interface =
      new OfonoSimManagerInterface(Ofono::SERVICE, path, QDBusConnection::systemBus(), this);
  if (!interface->isValid())
  {
    delete interface;
    Q_EMIT StateChanged(State(State::Reset, State::CallFinished));
    return;
  }

  _interface = interface;
  _interface->setTimeout(_dbusTimeout);
  connect(_interface, &OfonoSimManagerInterface::PropertyChanged, [this](const QString &in0, const QDBusVariant &in1) {
    State::Type type = StringToType.value(in0, State::_EMPTYTYPE_);
    if (State::_EMPTYTYPE_ != type)
      Q_EMIT StateChanged(State(type, State::Signal, in1.variant()));
  });

  Q_EMIT StateChanged(State(State::OfonoSimManagerGetProperties, State::CallStarted));
  connect(new QDBusPendingCallWatcher(_interface->GetProperties(), _interface), &QDBusPendingCallWatcher::finished,
          [this](QDBusPendingCallWatcher *watcher) {
            QDBusPendingReply<QVariantMap> reply(*watcher);
            watcher->deleteLater();
            if (reply.isError())
            {
              Q_EMIT StateChanged(State(State::OfonoSimManagerGetProperties, State::CallError, reply.error()));
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
              Q_EMIT StateChanged(State(State::OfonoSimManagerGetProperties, State::CallFinished));
            }
          });
  Q_EMIT StateChanged(State(State::Reset, State::CallFinished));
}
