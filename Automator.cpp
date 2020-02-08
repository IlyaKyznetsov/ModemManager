#include "Automator.h"
#include "Global.h"


Automator::Automator(QObject *parent)
: QObject(parent),
  _statesTodo{
    State(State::OfonoManagerModemRemoved, State::Signal),       // 0
    State(State::OfonoModemGetProperties, State::CallStarted),   // 1
    State(State::OfonoModemGetProperties, State::CallFinished),  // 2
    State(State::OfonoModemLockdown, State::CallStarted, true),  // 3
    State(State::OfonoModemLockdown, State::CallFinished),       // 4
    State(State::OfonoModemLockdown, State::Signal, true),       // 5
    State(State::OfonoModemLockdown, State::CallStarted, false), // 6
    State(State::OfonoModemLockdown, State::CallFinished),       // 7
    State(State::OfonoModemLockdown, State::Signal, false),      // 8
    State(State::OfonoModemPowered, State::CallStarted, true),   // 9
    State(State::OfonoModemPowered, State::CallFinished),        // 10
    State(State::OfonoModemPowered, State::Signal, true),        // 11
    //
    State(State::OfonoConnectionManagerGetProperties, State::CallStarted),  // 12
    State(State::OfonoConnectionManagerGetProperties, State::CallFinished), // 13
    State(State::OfonoConnectionManagerGetContexts, State::CallStarted),    // 14
    State(State::OfonoConnectionManagerGetContexts, State::CallFinished),   // 15
    //
    State(State::OfonoConnectionManagerRemoveContext, State::Status::CallStarted),  // 16
    State(State::OfonoConnectionManagerRemoveContext, State::Status::CallFinished), // 17
    State(State::OfonoConnectionManagerContextRemoved, State::Status::Signal),      // 18
    //
    State(State::OfonoConnectionManagerAddContext, State::Status::CallStarted),  // 19
    State(State::OfonoConnectionManagerAddContext, State::Status::CallFinished), // 20

    State(State::OfonoConnectionContextGetProperties, State::CallStarted),  // 21
    State(State::OfonoConnectionContextGetProperties, State::CallFinished), // 22
    //
    State(State::OfonoModemOnline, State::CallStarted, true), // 23
    State(State::OfonoModemOnline, State::CallFinished),      // 24
    State(State::OfonoModemOnline, State::Signal, true)       // 25
  },
  _stateCurrent(_statesTodo.begin())
{
    DF();
}

void Automator::stateChangedHandler(QObject *sender_ptr, const State &nextState)
{
    DF()<<sender_ptr<<nextState;
}
