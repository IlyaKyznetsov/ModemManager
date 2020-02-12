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
  if (isEnd())
    return;

  const Scripts::Item::Iterator iterator = _iterator + 1;
  if (State::CallError == state.status() &&
      State(iterator->state.type(), state.status(), iterator->state.value(), state.error()) == state)
  {
    Q_EMIT StatusChanged(Error);
    reset();
    return;
  }

  if (iterator->state != state)
    return;

  if (isBegin())
    Q_EMIT StatusChanged(Started);

  ++_iterator;
  if (_iterator->command)
    _iterator->command(_iterator, sender, data);

  DF() << _iterator->state;

  if (isEnd())
  {
    Q_EMIT StatusChanged(Finished);
    reset();
  }
}

void Scripts::Basic::reset()
{
  _iterator = _script.cbegin();
}

bool Scripts::Basic::isEnd() const
{
  return (_iterator == _script.cend() || _iterator + 1 == _script.cend());
}

bool Scripts::Basic::isBegin() const
{
  return _iterator == _script.cbegin();
}
