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
    Data() = default;
    void update(const State &state);
  };

  struct Item
  {
    typedef QVector<Automator::Item>::const_iterator Iterator;
    typedef void (*StateItemCommand)(Iterator &iterator, QObject *sender, const Automator::Data &data);
    State state;
    StateItemCommand command;
    Item(
    const State &_state,
    StateItemCommand _command = [](Automator::Item::Iterator &iterator, QObject *sender, const Automator::Data &data) {
      Q_UNUSED(sender)
      Q_UNUSED(data)
      DF() << iterator->state;
    });
    bool operator==(const State &state) const;
    bool operator!=(const State &state) const;
  };
  const QVector<Automator::Item> _scriptInitializationModem;
  const QVector<Automator::Item> _scriptInitializationSimManager;
  const QVector<Automator::Item> _scriptInitializationNetworkRegistration;
  const QVector<Automator::Item> _scriptInitializationConnectionManager;
  const QVector<Automator::Item> _scriptInitializationConnectionContext;
  Item::Iterator _scriptIterator;
  Data _data;
};

#endif // AUTOMATOR_H
