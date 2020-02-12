#include "AutomatorScript.h"

AutomatorScript::AutomatorScript(const QVector<AutomatorScript::Item> &script, QObject *parent)
: QObject(parent), _status(NotStarted), _script(script), _iterator(script.cbegin())
{
}

AutomatorScript::ScriptStatus AutomatorScript::status() const
{
  return _status;
}

void AutomatorScript::reset()
{
  _status = NotStarted;
  _iterator = _script.cbegin();
}

void AutomatorScript::processing(QObject *sender, const State &state, const AutomatorScript::Data &data)
{
  if (_iterator == _script.cend() || _iterator + 1 == _script.cend())
  {
//    DF() << "End script iterator for:"<<sender<<state;
    return;
  }

  const Item::Iterator iterator = _iterator + 1;
  if (State::CallError == state.status() &&
      State(iterator->state.type(), state.status(), iterator->state.value(), state.error()) == state)
  {
    _status = Error;
    Q_EMIT StatusChanged(Error);
    return;
  }

  if (iterator->state != state) return;

  if (_iterator == _script.cbegin())
  {
    _status = Started;
    Q_EMIT StatusChanged(Started);
  }

  ++_iterator;
  if (_iterator->command) _iterator->command(_iterator, sender, data);

  DF() << _iterator->state;

  if (_iterator + 1 == _script.cend())
  {
    _status = Finished;
    Q_EMIT StatusChanged(Finished);
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
