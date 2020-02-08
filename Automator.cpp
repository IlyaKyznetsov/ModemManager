#include "Automator.h"
#include "Global.h"
#include "adapters/Modem.h"

Automator::Automator(QObject *parent)
: QObject(parent),
  _statesTodo{
    StateItem(
    State(State::OfonoManagerModemRemoved, State::Signal),
    [](QObject *sender_ptr) {
      DF() << "--- State::OfonoManagerModemRemoved, State::Signal";
      Q_UNUSED(sender_ptr)
    },
    [](QObject *sender_ptr) {
      DF() << "+++ State::OfonoManagerModemRemoved, State::CallStarted";
      Q_UNUSED(sender_ptr)
    }), // 0
    StateItem(
    State(State::OfonoModemGetProperties, State::CallStarted),
    [](QObject *sender_ptr) {
      DF() << "--- State::OfonoModemGetProperties, State::CallStarted";
      Q_UNUSED(sender_ptr)
    },
    [](QObject *sender_ptr) {
      DF() << "+++ State::OfonoModemGetProperties, State::CallStarted";
      Q_UNUSED(sender_ptr)
    }), // 1
    StateItem(
    State(State::OfonoModemGetProperties, State::CallFinished),
    [](QObject *sender_ptr) { DF() << "--- State::OfonoModemGetProperties, State::CallFinished"; },
    [](QObject *sender_ptr) {
      DF() << "+++ State::OfonoModemGetProperties, State::CallFinished";
      qobject_cast<Modem *>(sender_ptr)->call(State::OfonoModemLockdown, true);
    }), // 2
    StateItem(
    State(State::OfonoModemLockdown, State::CallStarted, true),
    [](QObject *sender_ptr) { DF() << "--- State::OfonoModemLockdown, State::CallStarted, true"; },
    [](QObject *sender_ptr) { DF() << "+++ State::OfonoModemLockdown, State::CallStarted, true"; }), // 3
    StateItem(
    State(State::OfonoModemLockdown, State::CallFinished),
    [](QObject *sender_ptr) { DF() << "---"; },
    [](QObject *sender_ptr) { DF() << "+++"; }), // 4
    StateItem(
    State(State::OfonoModemLockdown, State::Signal, true),
    [](QObject *sender_ptr) { DF() << "---"; },
    [](QObject *sender_ptr) { DF() << "+++"; }), // 5
    StateItem(
    State(State::OfonoModemLockdown, State::CallStarted, false),
    [](QObject *sender_ptr) { DF() << "---"; },
    [](QObject *sender_ptr) { DF() << "+++"; }), // 6
    StateItem(
    State(State::OfonoModemLockdown, State::CallFinished),
    [](QObject *sender_ptr) { DF() << "---"; },
    [](QObject *sender_ptr) { DF() << "+++"; }), // 7
    StateItem(
    State(State::OfonoModemLockdown, State::Signal, false),
    [](QObject *sender_ptr) { DF() << "---"; },
    [](QObject *sender_ptr) { DF() << "+++ State(State::OfonoModemLockdown, State::Signal, false)"; }), // 8
    StateItem(
    State(State::OfonoModemPowered, State::CallStarted, true),
    [](QObject *sender_ptr) { DF() << "---"; },
    [](QObject *sender_ptr) {
      DF() << "+++ State(State::OfonoModemPowered, State::CallStarted, true)";
    }), // 9
    StateItem(
    State(State::OfonoModemPowered, State::CallFinished),
    [](QObject *sender_ptr) { DF() << "---"; },
    [](QObject *sender_ptr) {
      DF() << "+++ State(State::OfonoModemPowered, State::CallFinished)";
      qobject_cast<Modem *>(sender_ptr)->call(State::OfonoModemLockdown, true);
    }), // 10
    StateItem(
    State(State::OfonoModemPowered, State::Signal, true),
    [](QObject *sender_ptr) { DF() << "---"; },
    [](QObject *sender_ptr) { DF() << "+++"; }), // 11
    //
    StateItem(
    State(State::OfonoConnectionManagerGetProperties, State::CallStarted),
    [](QObject *sender_ptr) { DF() << "---"; },
    [](QObject *sender_ptr) { DF() << "+++"; }), // 12
    StateItem(
    State(State::OfonoConnectionManagerGetProperties, State::CallFinished),
    [](QObject *sender_ptr) { DF() << "---"; },
    [](QObject *sender_ptr) { DF() << "+++"; }), // 13
    StateItem(
    State(State::OfonoConnectionManagerGetContexts, State::CallStarted),
    [](QObject *sender_ptr) { DF(); },
    [](QObject *sender_ptr) { DF(); }), // 14
    StateItem(
    State(State::OfonoConnectionManagerGetContexts, State::CallFinished),
    [](QObject *sender_ptr) { DF(); },
    [](QObject *sender_ptr) { DF(); }), // 15
    //
    StateItem(
    State(State::OfonoConnectionManagerRemoveContext, State::Status::CallStarted),
    [](QObject *sender_ptr) { DF(); },
    [](QObject *sender_ptr) { DF(); }), // 16
    StateItem(
    State(State::OfonoConnectionManagerRemoveContext, State::Status::CallFinished),
    [](QObject *sender_ptr) { DF(); },
    [](QObject *sender_ptr) { DF(); }), // 17
    StateItem(
    State(State::OfonoConnectionManagerContextRemoved, State::Status::Signal),
    [](QObject *sender_ptr) { DF(); },
    [](QObject *sender_ptr) { DF(); }), // 18
    //
    StateItem(
    State(State::OfonoConnectionManagerAddContext, State::Status::CallStarted),
    [](QObject *sender_ptr) { DF(); },
    [](QObject *sender_ptr) { DF(); }), // 19
    StateItem(
    State(State::OfonoConnectionManagerAddContext, State::Status::CallFinished),
    [](QObject *sender_ptr) { DF(); },
    [](QObject *sender_ptr) { DF(); }), // 20

    StateItem(
    State(State::OfonoConnectionContextGetProperties, State::CallStarted),
    [](QObject *sender_ptr) { DF(); },
    [](QObject *sender_ptr) { DF(); }), // 21
    StateItem(
    State(State::OfonoConnectionContextGetProperties, State::CallFinished),
    [](QObject *sender_ptr) { DF(); },
    [](QObject *sender_ptr) { DF(); }), // 22
    //
    StateItem(
    State(State::OfonoModemOnline, State::CallStarted, true),
    [](QObject *sender_ptr) { DF(); },
    [](QObject *sender_ptr) { DF(); }), // 23
    StateItem(
    State(State::OfonoModemOnline, State::CallFinished),
    [](QObject *sender_ptr) { DF(); },
    [](QObject *sender_ptr) { DF(); }), // 24
    StateItem(
    State(State::OfonoModemOnline, State::Signal, true),
    [](QObject *sender_ptr) { DF(); },
    [](QObject *sender_ptr) { DF(); }) // 25*/
  },
  _stateCurrent(_statesTodo.begin())
{
  DF();
}

void Automator::stateChangedHandler(QObject *sender_ptr, const State &nextState)
{
  if (State::CallError == nextState.status())
  {
    throw astr_global::Exception(nextState.error().message());
  }

  if ((_stateCurrent + 1)->operator!=(nextState))
  {
    //    DF() << " --- " << nextState;
    return;
  }

  ++_stateCurrent;
  DF() << _stateCurrent->_state;

  next(_stateCurrent + 1, sender_ptr);
}

Automator::StateItem::StateItem(const State &state, Automator::Function prev, Automator::Function next)
: _state(state), _prev(prev), _next(next)
{
  //  DF() << _state;
}

bool Automator::StateItem::operator==(const State &state) const
{
  return _state == state;
}

bool Automator::StateItem::operator!=(const State &state) const
{
  return _state != state;
}
