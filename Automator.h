#ifndef AUTOMATOR_H
#define AUTOMATOR_H

#include "types.h"
#include <QObject>

class Automator : public QObject
{
  Q_OBJECT
public:
  explicit Automator(QObject *parent = nullptr);
  void stateChangedHandler(QObject *sender_ptr, const State &state);

private:
  struct Data
  {
      QVariant modemLockdown;
      QVariant modemPowered;
      QVariant modemOnline;
      Data()=default;
      void update(const State &state);
  };

  struct Item
  {
    typedef void (*StateItemCommand)(QObject *sender, const Automator::Data& data);
    State state;
    StateItemCommand command;
    Item(const State &_state, StateItemCommand _command = nullptr);
    bool operator==(const State &state) const;
    bool operator!=(const State &state) const;
  };
  const QVector<Automator::Item> _automatorStates;
  typedef QVector<Automator::Item>::const_iterator Iterator;
  Iterator _stateIterator;
  Data _data;
};

#endif // AUTOMATOR_H
