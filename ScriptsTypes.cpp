#include "ScriptsTypes.h"

Scripts::Item::Item(const State &_state, Scripts::Item::StateItemCommand _command) : state(_state), command(_command)
{
}

bool Scripts::Item::operator==(const State &state) const
{
  return state == this->state;
}

bool Scripts::Item::operator!=(const State &state) const
{
  return state != this->state;
}

Scripts::Basic::Basic(const QVector<Scripts::Item> &script, QObject *parent)
    : QObject(parent), _script(script), _iterator(script.cbegin())
{
}

void Scripts::Basic::processing(QObject *sender, const State &state, const Scripts::Data &data)
{
  if (_iterator == _script.cend() || _iterator + 1 == _script.cend())
  {
     DF("end iterator");
     throw 1;
  }

  const Scripts::Item::Iterator iterator = _iterator + 1;
  if (State::CallError == state.status() &&
      State(iterator->state.type(), state.status(), iterator->state.value(), state.error()) == state)
  {
    _status = Error;
    Q_EMIT StatusChanged(Error);
    return;
  }

  if (iterator->state != state)
    return;

  if (_iterator == _script.cbegin())
  {
    _status = Started;
    Q_EMIT StatusChanged(Started);
  }

  ++_iterator;
  if (_iterator->command)
    _iterator->command(_iterator, sender, data);

  DF() << _iterator->state;

  if (_iterator + 1 == _script.cend())
  {
    _status = Finished;
    Q_EMIT StatusChanged(Finished);
  }
}

void Scripts::Basic::reset()
{
  _status = NotStarted;
  _iterator = _script.cbegin();
}
