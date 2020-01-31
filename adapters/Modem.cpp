#include "Modem.h"
#include "ofono_modem_interface.h"

static const QMap<QString, State::Type> StringToType{
    {"Powered", State::OfonoModemPowered},      {"Online", State::OfonoModemOnline},
    {"Lockdown", State::OfonoModemLockdown},    {"Manufacturer", State::OfonoModemManufacturer},
    {"Model", State::OfonoModemModel},          {"Serial", State::OfonoModemSerial},
    {"Interfaces", State::OfonoModemInterfaces}};

static const QMap<State::Type, QString> TypeToString{{State::OfonoModemPowered, "Powered"},
                                                     {State::OfonoModemOnline, "Online"},
                                                     {State::OfonoModemLockdown, "Lockdown"}};

Modem::Modem(QObject *parent) : QObject(parent), _interface(nullptr), _currentCallType(State::_EMPTYTYPE_)
{
}

bool Modem::isValid() const
{
  return _interface && _interface->isValid();
}

QString Modem::path() const
{
  return (isValid() ? _interface->path() : QString());
}

void Modem::reset(const QString &path)
{
  DF() << path;
  Q_EMIT StateChanged(State(State::Reset, State::CallStarted));

  _currentCallType = State::_EMPTYTYPE_;
  _modemInterfaces.clear();

  if (path.isEmpty())
  {
    delete _interface;
    _interface = nullptr;
    Q_EMIT StateChanged(State(State::Reset, State::CallFinished));
    return;
  }

  OfonoModemInterface *interface = new OfonoModemInterface(Ofono::SERVICE, path, QDBusConnection::systemBus(), this);
  if (!interface->isValid())
  {
    delete interface;
    Q_EMIT StateChanged(State(State::Reset, State::CallFinished));
    return;
  }

  _interface = interface;
  connect(_interface, &OfonoModemInterface::PropertyChanged, [this](const QString &in0, const QDBusVariant &in1) {
    State::Type type = StringToType.value(in0, State::_EMPTYTYPE_);
    switch (type)
    {
      case State::_EMPTYTYPE_: break;
      case State::OfonoModemInterfaces:
      {
        for (State::Type typeInterface : _modemInterfacesChanged(in1.variant().toStringList()))
          Q_EMIT StateChanged(State(typeInterface, State::Signal));
      }
      break;
      default: Q_EMIT StateChanged(State(type, State::Signal, in1.variant())); break;
    }
  });

  Q_EMIT StateChanged(State(State::OfonoModemGetProperties, State::CallStarted));
  connect(new QDBusPendingCallWatcher(_interface->GetProperties(), _interface), &QDBusPendingCallWatcher::finished,
          [this](QDBusPendingCallWatcher *watcher) {
            QDBusPendingReply<QVariantMap> reply(*watcher);
            watcher->deleteLater();
            if (reply.isError())
            {
              Q_EMIT StateChanged(State(State::OfonoModemGetProperties, State::CallError, reply.error()));
            }
            else
            {
              const QVariantMap &properties = reply.value();
              for (auto iterator = properties.keyValueBegin(); iterator != properties.keyValueEnd(); ++iterator)
              {
                //                D("---" << (*iterator).first << (*iterator).second);
                State::Type type = StringToType.value((*iterator).first, State::_EMPTYTYPE_);
                switch (type)
                {
                  case State::_EMPTYTYPE_: break;
                  case State::OfonoModemInterfaces:
                  {
                    for (State::Type typeInterface : _modemInterfacesChanged((*iterator).second.toStringList()))
                      Q_EMIT StateChanged(State(typeInterface, State::Signal));
                  }
                  break;
                  default: Q_EMIT StateChanged(State(type, State::Signal, (*iterator).second)); break;
                }
              }
              Q_EMIT StateChanged(State(State::OfonoModemGetProperties, State::CallFinished));
            }
          });
  Q_EMIT StateChanged(State(State::State::Reset, State::CallFinished));
}

void Modem::call(const State::Type type, const QVariant &value)
{
  DF() << type << value;

  QString name = TypeToString.value(type);
  if (name.isEmpty())
  {
    throw astr_global::Exception("Неверный тип call");
  }

  Q_EMIT StateChanged(State(type, State::CallStarted, value));
  if (State::_EMPTYTYPE_ != _currentCallType)
    Q_EMIT StateChanged(State(type, State::CallError, "Running another call"));

  _currentCallType = type;
  connect(new QDBusPendingCallWatcher(_interface->SetProperty(name, QDBusVariant(value)), _interface),
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

QList<State::Type> Modem::_modemInterfacesChanged(const QStringList &interfaces)
{
  QList<State::Type> typeListChanges;
  QString name("org.ofono.SimManager");
  bool isPrev, isCurr;
  isPrev = _modemInterfaces.contains(name);
  isCurr = interfaces.contains(name);
  if (isPrev != isCurr)
  {
    typeListChanges.append((isCurr) ? State::OfonoModemInterfaceSimManagerAdded
                                    : State::OfonoModemInterfaceSimManagerRemoved);
  }

  name = "org.ofono.NetworkRegistration";
  isPrev = _modemInterfaces.contains(name);
  isCurr = interfaces.contains(name);
  if (isPrev != isCurr)
  {
    typeListChanges.append((isCurr) ? State::OfonoModemInterfaceNetworkRegistrationAdded
                                    : State::OfonoModemInterfaceNetworkRegistrationRemoved);
  }

  name = "org.ofono.ConnectionManager";
  isPrev = _modemInterfaces.contains(name);
  isCurr = interfaces.contains(name);
  if (isPrev != isCurr)
  {
    typeListChanges.append((isCurr) ? State::OfonoModemInterfaceConnectionManagerAdded
                                    : State::OfonoModemInterfaceConnectionManagerRemoved);
  }
  _modemInterfaces = interfaces;
  DF() << _modemInterfaces << typeListChanges;
  return typeListChanges;
}
