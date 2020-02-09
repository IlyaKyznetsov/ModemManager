#include "Automator.h"
#include "Global.h"
#include "adapters/Modem.h"

Automator::Automator(QObject *parent)
: QObject(parent),
  _scriptInitializationModem{
    Item(State(State::OfonoManagerModemRemoved, State::Signal)),     // 0
    Item(State(State::OfonoModemGetProperties, State::CallStarted)), // 1
    Item(State(State::OfonoModemGetProperties, State::CallFinished),
         [](Automator::Item::Iterator &iterator, QObject *sender, const Automator::Data &data) {
           DF() << "+++ " << iterator->state << sender << data.modemLockdown << data.modemPowered
                << data.modemOnline;
           Modem *modem = static_cast<Modem *>(sender);
           if (!data.modemLockdown.toBool())
             modem->call(State::OfonoModemLockdown, true);
           else
             throw astr_global::Exception("State::OfonoModemLockdown is true");
         }),                                                          // 2
    Item(State(State::OfonoModemLockdown, State::CallStarted, true)), // 3
    Item(State(State::OfonoModemLockdown, State::CallFinished)),      // 4
    Item(State(State::OfonoModemLockdown, State::Signal, true),
         [](Automator::Item::Iterator &iterator, QObject *sender, const Automator::Data &data) {
           DF() << "+++ " << iterator->state << sender << data.modemLockdown << data.modemPowered
                << data.modemOnline;
           Modem *modem = static_cast<Modem *>(sender);
           if (data.modemLockdown.toBool())
             modem->call(State::OfonoModemLockdown, false);
           else
             throw astr_global::Exception("State::OfonoModemLockdown is false");
         }),                                                           // 5
    Item(State(State::OfonoModemLockdown, State::CallStarted, false)), // 6
    Item(State(State::OfonoModemLockdown, State::CallFinished)),       // 7
    Item(State(State::OfonoModemLockdown, State::Signal, false),
         [](Automator::Item::Iterator &iterator, QObject *sender, const Automator::Data &data) {
           DF() << "+++ " << iterator->state << sender << data.modemLockdown << data.modemPowered
                << data.modemOnline;
           Modem *modem = static_cast<Modem *>(sender);
           if (!data.modemPowered.toBool())
             modem->call(State::OfonoModemPowered, true);
           else
             throw astr_global::Exception("State::OfonoModemPowered is true");
         }),                                                         // 8
    Item(State(State::OfonoModemPowered, State::CallStarted, true)), // 9
    Item(State(State::OfonoModemPowered, State::CallFinished)),      // 10
    Item(State(State::OfonoModemPowered, State::Signal, true),
         [](Automator::Item::Iterator &iterator, QObject *sender, const Automator::Data &data) {
           DF() << "+++ " << iterator->state << sender << data.modemLockdown << data.modemPowered
                << data.modemOnline;
           Modem *modem = static_cast<Modem *>(sender);
           if (!data.modemOnline.toBool())
             modem->call(State::OfonoModemOnline, true);
           else
             throw astr_global::Exception("State::OfonoModemOnline is true");
         }), // 11

    Item(State(State::OfonoModemOnline, State::CallStarted, true)), // 23
    Item(State(State::OfonoModemOnline, State::CallFinished)),      // 24
    Item(State(State::OfonoModemOnline, State::Signal, true))       // 25
  },
  _scriptIterator(_scriptInitializationModem.begin())
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

  Item::Iterator stateIterator = (_scriptIterator + 1);
  if (stateIterator->operator!=(state)) return;
  _scriptIterator = stateIterator;
  _scriptIterator->command(_scriptIterator, sender_ptr, _data);
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
