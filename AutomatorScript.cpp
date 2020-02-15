#include "AutomatorScript.h"

const State AutomatorScript::emptyState = State(State::_EMPTYTYPE_, State::_EMPTYSTATUS_);

AutomatorScript::AutomatorScript(const QVector<AutomatorScript::Item> &script, QObject *parent)
: QObject(parent), _status(State::_EMPTYSTATUS_), _script(script), _iterator(script.cbegin())
{
}

State::Status AutomatorScript::status() const
{
  return _status;
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
    D("--- PROCESSING ERROR ---:"
      << "state:" << state << "iterator:" << iterator->state << state.error().message());
    _status = State::CallError;
    Q_EMIT StatusChanged(_status, state);
  }
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
  D("ModemLockdown: " << modemLockdown);
  D("ModemPowered : " << modemPowered);
  D("ModemOnline  : " << modemOnline);
  D("SimManagerCardIdentifier     : " << simManagerCardIdentifier);
  D("SimManagerServiceProviderName:" << simManagerServiceProviderName);
  D("NetworkRegistrationStatus" << networkRegistrationStatus);
  D("ConnectionContextAccessPointName:" << connectionContextAccessPointName);
  D("ConnectionContextUsername       :" << connectionContextUsername);
  D("ConnectionContextPassword       :" << connectionContextPassword);
  D("ConnectionManagerAttached       :" << connectionManagerAttached);
  D("ConnectionManagerPowered        :" << connectionManagerPowered);
  D("ConnectionContextActive:" << connectionContextActive);
}
