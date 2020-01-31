#include "ConnectionContext.h"
#include "ofono_connection_context_interface.h"

static const QMap<QString, State::Type> StringToType{
    {"Active", State::OfonoConnectionContextActive},
    {"AccessPointName", State::OfonoConnectionContextAccessPointName},
    {"Username", State::OfonoConnectionContextUsername},
    {"Password", State::OfonoConnectionContextPassword},
    {"Type", State::OfonoConnectionContextType},
    {"AuthenticationMethod", State::OfonoConnectionContextAuthenticationMethod},
    {"Protocol", State::OfonoConnectionContextProtocol},
    {"Name", State::OfonoConnectionContextName},
    {"Settings", State::OfonoConnectionContextSettings},
    {"Interface", State::OfonoConnectionContextInterface},
    {"Method", State::OfonoConnectionContextMethod},
    {"Address", State::OfonoConnectionContextAddress},
    {"Netmask", State::OfonoConnectionContextNetmask}};

static const QMap<State::Type, QString> TypeToString{
    {State::OfonoConnectionContextActive, "Active"},
    {State::OfonoConnectionContextAccessPointName, "AccessPointName"},
    {State::OfonoConnectionContextUsername, "Username"},
    {State::OfonoConnectionContextPassword, "Password"},
    {State::OfonoConnectionContextType, "Type"},
    {State::OfonoConnectionContextAuthenticationMethod, "AuthenticationMethod"},
    {State::OfonoConnectionContextProtocol, "Protocol"},
    {State::OfonoConnectionContextName, "Name"}};

ConnectionContext::ConnectionContext(QObject *parent)
    : QObject(parent), _interface(nullptr), _currentCallType(State::_EMPTYTYPE_)
{
}

bool ConnectionContext::isValid() const
{
  return _interface && _interface->isValid();
}

void ConnectionContext::reset(const QString &path)
{
  DF() << path;
  Q_EMIT StateChanged(State(State::Reset, State::CallStarted));

  _currentCallType = State::_EMPTYTYPE_;

  if (path.isEmpty())
  {
    delete _interface;
    _interface = nullptr;
    Q_EMIT StateChanged(State(State::Reset, State::CallFinished));
    return;
  }

  OfonoConnectionContextInterface *interface =
      new OfonoConnectionContextInterface(Ofono::SERVICE, path, QDBusConnection::systemBus(), this);
  if (!interface->isValid())
  {
    delete interface;
    Q_EMIT StateChanged(State(State::Reset, State::CallFinished));
    return;
  }

  _interface = interface;
  connect(_interface, &OfonoConnectionContextInterface::PropertyChanged,
          [this](const QString &in0, const QDBusVariant &in1) {
            State::Type type = StringToType.value(in0, State::_EMPTYTYPE_);
            switch (type)
            {
              case State::_EMPTYTYPE_: break;
              default: Q_EMIT StateChanged(State(type, State::Signal, in1.variant())); break;
            }
          });

  Q_EMIT StateChanged(State(State::OfonoConnectionContextGetProperties, State::CallStarted));
  connect(new QDBusPendingCallWatcher(_interface->GetProperties(), _interface), &QDBusPendingCallWatcher::finished,
          [this](QDBusPendingCallWatcher *watcher) {
            QDBusPendingReply<QVariantMap> reply(*watcher);
            watcher->deleteLater();
            if (reply.isError())
            {
              Q_EMIT StateChanged(State(State::OfonoConnectionContextGetProperties, State::CallError, reply.error()));
            }
            else
            {
              const QVariantMap &properties = reply.value();
              for (auto iterator = properties.keyValueBegin(); iterator != properties.keyValueEnd(); ++iterator)
              {
                State::Type type = StringToType.value((*iterator).first, State::_EMPTYTYPE_);
                //                D("TTTTTTTTTTT" << type);
                switch (type)
                {
                  case State::_EMPTYTYPE_: break;
                  case State::OfonoConnectionContextSettings:
                  {
                    const QVariantMap settings = qdbus_cast<QVariantMap>((*iterator).second.value<QDBusArgument>());
                    //                    D(settings);
                    for (auto iterator = settings.keyValueBegin(); iterator != settings.keyValueEnd(); ++iterator)
                    {
                      //                      D("XXX" << (*iterator).first);
                      State::Type type = StringToType.value((*iterator).first, State::_EMPTYTYPE_);
                      if (State::_EMPTYTYPE_ != type)
                        Q_EMIT StateChanged(State(type, State::Signal, (*iterator).second));
                    }
                  }
                  break;
                  default: Q_EMIT StateChanged(State(type, State::Signal, (*iterator).second)); break;
                }
              }
              Q_EMIT StateChanged(State(State::OfonoConnectionContextGetProperties, State::CallFinished));
            }
          });
  Q_EMIT StateChanged(State(State::State::Reset, State::CallFinished));
}

void ConnectionContext::call(const State::Type type, const QVariant &value)
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
