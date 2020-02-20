#include "AutomatorScript.h"

const State AutomatorScript::emptyState = State(State::_EMPTYTYPE_, State::_EMPTYSTATUS_);

AutomatorScript::AutomatorScript(const QVector<AutomatorScript::Item> &script)
    : _script(script), _iterator(script.cbegin()), _scriptStatus(State::_EMPTYSTATUS_)
{
}

State::Status AutomatorScript::status() const
{
  return _scriptStatus;
}

QDBusError AutomatorScript::error() const
{
  return _scriptError;
}

bool AutomatorScript::Signal()
{
  if (_scriptStatus == State::Signal || State::_EMPTYSTATUS_ == _scriptStatus || State::CallFinished == _scriptStatus)
  {
    _scriptStatus = State::Signal;
    return true;
  }

  const QMap<State::Status, QString> statuses = {{State::_EMPTYSTATUS_, "_EMPTYSTATUS_"},
                                                 {State::Signal, "Signal"},
                                                 {State::CallStarted, "CallStarted"},
                                                 {State::CallFinished, "CallFinished"},
                                                 {State::CallError, "CallError"}};

  throw astr_global::Exception("SIGNAL: " + statuses.value(_scriptStatus));
  return false;
}

State::Status AutomatorScript::processing(const State &state)
{
  if (_iterator == _script.cend())
  {
    C("Bad iterator");
    throw astr_global::Exception("Bad iterator");
  }

  const Item::Iterator iterator = _iterator + 1;
  if (iterator == _script.cend())
    return _scriptStatus;

  if (iterator->state == state)
  {
    if (emptyState == _iterator->state)
      _scriptStatus = State::CallStarted;

    _iterator = iterator;
    if (iterator + 1 == _script.cend())
    {
      _iterator = _script.cbegin();
      _scriptStatus = State::_EMPTYSTATUS_;
      return State::CallFinished;
    }
  }
  else if (State(iterator->state.type(), State::CallError, iterator->state.value(), state.error()) == state)
  {
    _iterator = _script.cbegin();
    _scriptStatus = State::CallError;
    _scriptError = state.error();
  }
  return _scriptStatus;
}

void AutomatorScript::reset()
{
  _iterator = _script.cbegin();
  _scriptStatus = State::_EMPTYSTATUS_;
  _scriptError = QDBusError();
}

AutomatorScript::operator QString() const
{
  const QMap<State::Status, QString> statuses = {{State::_EMPTYSTATUS_, "_EMPTYSTATUS_"},
                                                 {State::Signal, "Signal"},
                                                 {State::CallStarted, "CallStarted"},
                                                 {State::CallFinished, "CallFinished"},
                                                 {State::CallError, "CallError"}};
  return statuses.value(_scriptStatus);
}

AutomatorScript::Item::Item(const State &_state) : state(_state)
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
