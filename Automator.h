#ifndef AUTOMATOR_H
#define AUTOMATOR_H

#include "types.h"
#include <QObject>

class Automator : public QObject
{
  Q_OBJECT
public:
  explicit Automator(QObject *parent = nullptr);
  void stateChangedHandler(QObject *sender_ptr, const State &nextState);

private:
  typedef void (*Function)(QObject *sender_ptr);
  class StateItem
  {
  public:
    State _state;
    Function _prev;
    Function _next;


  public:
    StateItem(const State &state, Function prev, Function next);
    bool operator==(const State &state) const;
    bool operator!=(const State &state) const;
  };
  const QVector<StateItem> _statesTodo;
  QVector<StateItem>::const_iterator _stateCurrent;
  QVector<StateItem>::const_iterator next(const QVector<StateItem>::const_iterator current, QObject *sender_ptr)
  {
    DF() << "====================== next" << current << sender_ptr;
    current->_next(sender_ptr);
    return current + 1;
  }
  QVector<StateItem>::const_iterator prev(const QVector<StateItem>::const_iterator current, QObject *sender_ptr)
  {
    DF() << "====================== prev" << current << sender_ptr;
    current->_prev(sender_ptr);
    return current - 1;
  }
};

#endif // AUTOMATOR_H
