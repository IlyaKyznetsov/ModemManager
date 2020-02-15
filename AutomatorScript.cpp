#include "AutomatorScript.h"

const State AutomatorScript::emptyState = State(State::_EMPTYTYPE_, State::_EMPTYSTATUS_);

AutomatorScript::AutomatorScript(const Type type, const QVector<AutomatorScript::Item> &script, QObject *parent)
: QObject(parent), _type(type), _status(State::_EMPTYSTATUS_), _script(script), _iterator(script.cbegin())
{
}

State::Status AutomatorScript::status() const
{
  return _status;
}

AutomatorScript::Type AutomatorScript::type() const
{
  return _type;
}

void AutomatorScript::reset()
{
  _status = State::_EMPTYSTATUS_;
  _iterator = _script.cbegin();
}

void AutomatorScript::processing(QObject *sender, const State &state, const AutomatorScript::Data &data)
{
  const Item::Iterator iterator = _iterator + 1;
  if (iterator == _script.cend()) return;

  if (iterator->state == state)
  {
    if (emptyState == _iterator->state)
    {
      _status = State::CallStarted;
      Q_EMIT StatusChanged(_status, state);
    }

    _iterator = iterator;
    if (_iterator->command) _iterator->command(_iterator, sender, data);

    if (iterator + 1 == _script.cend())
    {
      _status = State::CallFinished;
      Q_EMIT StatusChanged(_status, state);
    }
  }
  else if (State(iterator->state.type(), State::CallError, iterator->state.value(), state.error()) == state)
  {
    _status = State::CallError;
    Q_EMIT StatusChanged(_status, state);
  }
}

AutomatorScript::operator QString() const
{
  const QMap<Type, QString> types{
    { _EMPTYTYPE_, "Script::__EMPTYTYPE__" },
    { ManagerModemAdded, "Script::ManagerModemAdded" },
    { ModemLockdownDisable, "Script::ModemLockdownDisable" },
    { ModemLockdownEnable, "Script::ModemLockdownEnable" },
    { ModemPoweredDisable, "Script::ModemPoweredDisable" },
    { ModemPoweredEnable, "Script::ModemPoweredEnable" },
    { ModemOnlineDisable, "Script::ModemOnlineDisable" },
    { ModemOnlineEnable, "Script::ModemOnlineEnable" },
    { SimManagerAdded, "Script::SimManagerAdded" },
    { NetworkRegistrationAdded, "Script::NetworkRegistrationAdded" },
    { ConnectionManagerAdded, "Script::ConnectionManagerAdded" },
    { ConnectionContextAdded, "Script::ConnectionContextAdded" },
    { ConnectionContextAccessPointName, "Script::ConnectionContextAccessPointName" },
    { ConnectionContextUsername, "Script::ConnectionContextUsername" },
    { ConnectionContextPassword, "Script::ConnectionContextPassword" },
    { ConnectionContextActiveDisable, "Script::ConnectionContextActiveDisable" },
    { ConnectionContextActiveEnable, "Script::ConnectionContextActiveEnable" }
  };

  const QMap<State::Status, QString> statuses = { { State::Status::_EMPTYSTATUS_,   "_EMPTYSTATUS_ " },
                                                  { State::Status::Signal,          "Signal      : " },
                                                  { State::Status::CallStarted,     "CallStarted : " },
                                                  { State::Status::CallFinished,    "CallFinished: " },
                                                  { State::Status::CallError,       "CallError   : " } };
  return "(" + statuses.value(status())+types.value(_type) + ")";
}

AutomatorScript::Item::Item(const State &_state, AutomatorScript::Item::StateItemCommand _command)
: state(_state), command(_command)
{
}

bool AutomatorScript::Item::operator==(const State &state) const
{
  return state == this->state;
}

bool AutomatorScript::Item::operator!=(const State &state) const
{
  return state != this->state;
}

void AutomatorScript::Data::debug()
{
  D("ModemLockdown                   : " << modemLockdown);
  D("ModemPowered                    : " << modemPowered);
  D("ModemOnline                     : " << modemOnline);
  D("SimManagerCardIdentifier        : " << simManagerCardIdentifier);
  D("SimManagerServiceProviderName   :" << simManagerServiceProviderName);
  D("NetworkRegistrationStatus       :" << networkRegistrationStatus);
  D("ConnectionContextAccessPointName:" << connectionContextAccessPointName);
  D("ConnectionContextUsername       :" << connectionContextUsername);
  D("ConnectionContextPassword       :" << connectionContextPassword);
  D("ConnectionManagerAttached       :" << connectionManagerAttached);
  D("ConnectionManagerPowered        :" << connectionManagerPowered);
  D("ConnectionContextActive         :" << connectionContextActive);
}
