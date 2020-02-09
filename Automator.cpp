#include "Automator.h"
#include "Global.h"
#include "adapters/Modem.h"

Automator::Automator(QObject *parent)
: QObject(parent),
  _automatorStates{
    Item(State(State::OfonoManagerModemRemoved, State::Signal)),     // 0
    Item(State(State::OfonoModemGetProperties, State::CallStarted)), // 1
    Item(State(State::OfonoModemGetProperties, State::CallFinished),
         [](QObject *sender, const Automator::Data &data) {
           DF() << "+++ State::OfonoModemGetProperties, State::CallFinished" << sender
                << data.modemLockdown << data.modemPowered << data.modemOnline;
           static_cast<Modem *>(sender)->call(State::OfonoModemLockdown, !data.modemLockdown.toBool());
         }),                                                           // 2
    Item(State(State::OfonoModemLockdown, State::CallStarted, true)),  // 3
    Item(State(State::OfonoModemLockdown, State::CallFinished)),       // 4
    Item(State(State::OfonoModemLockdown, State::Signal, true)),       // 5
    Item(State(State::OfonoModemLockdown, State::CallStarted, false)), // 6
    Item(State(State::OfonoModemLockdown, State::CallFinished)),       // 7
    Item(State(State::OfonoModemLockdown, State::Signal, false)),      // 8
    Item(State(State::OfonoModemPowered, State::CallStarted, true)),   // 9
    Item(State(State::OfonoModemPowered, State::CallFinished)),        // 10
    Item(State(State::OfonoModemPowered, State::Signal, true)),        // 11
    //
    Item(State(State::OfonoConnectionManagerGetProperties, State::CallStarted)),  // 12
    Item(State(State::OfonoConnectionManagerGetProperties, State::CallFinished)), // 13
    Item(State(State::OfonoConnectionManagerGetContexts, State::CallStarted)),    // 14
    Item(State(State::OfonoConnectionManagerGetContexts, State::CallFinished)),   // 15
    //
    Item(State(State::OfonoConnectionManagerRemoveContext, State::Status::CallStarted)),  // 16
    Item(State(State::OfonoConnectionManagerRemoveContext, State::Status::CallFinished)), // 17
    Item(State(State::OfonoConnectionManagerContextRemoved, State::Status::Signal)),      // 18
    //
    Item(State(State::OfonoConnectionManagerAddContext, State::Status::CallStarted)),  // 19
    Item(State(State::OfonoConnectionManagerAddContext, State::Status::CallFinished)), // 20

    Item(State(State::OfonoConnectionContextGetProperties, State::CallStarted)),  // 21
    Item(State(State::OfonoConnectionContextGetProperties, State::CallFinished)), // 22
    //
    Item(State(State::OfonoModemOnline, State::CallStarted, true)), // 23
    Item(State(State::OfonoModemOnline, State::CallFinished)),      // 24
    Item(State(State::OfonoModemOnline, State::Signal, true))       // 25*/
  },
  _stateIterator(_automatorStates.begin())
{
  DF();
}

void Automator::stateChangedHandler(QObject *sender_ptr, const State &state)
{
  if (State::CallError == state.status())
  {
    throw astr_global::Exception(state.error().message());
  }

  _data.update(state);

  QVector<Item>::const_iterator stateIterator = _stateIterator + 1;
  if (stateIterator->operator!=(state)) return;

  _stateIterator = stateIterator;
  if (_stateIterator->command)
  {
    _stateIterator->command(sender_ptr, _data);
  }
}

// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=

Automator::Item::Item(const State &_state, Automator::Item::StateItemCommand _command)
: state(_state), command(_command)
{
}

bool Automator::Item::operator==(const State &state) const
{
  return state == this->state;
}

bool Automator::Item::operator!=(const State &state) const
{
  return state != this->state;
}

void Automator::Data::update(const State &state)
{
  switch (state.status())
  {
  case State::Signal:
    break;
  default:
    return;
  }

  switch (state.type())
  {
  case State::OfonoModemPowered:
  {
    modemPowered = state.value();
  }
  break;
  case State::OfonoModemOnline:
  {
    modemOnline = state.value();
  }
  break;
  case State::OfonoModemLockdown:
  {
    modemLockdown = state.value();
  }
  break;

  default:
    return;
  }
}
