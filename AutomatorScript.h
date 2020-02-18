#ifndef AUTOMATORSCRIPT_H
#define AUTOMATORSCRIPT_H

#include "types.h"
#include <QtCore/QVariant>

class AutomatorScript : public QObject
{
  Q_OBJECT
public:
  static const State emptyState;

  struct Item
  {
    typedef QVector<Item>::const_iterator Iterator;
    State state;
    Item(const State &_state);
    bool operator==(const State &state) const;
    bool operator!=(const State &state) const;
  };

public:
  AutomatorScript(const QVector<Item> &script);
  ~AutomatorScript() = default;
  State::Status status() const;
  QDBusError error() const;
  State::Status processing(const State &state);
  void reset();
  operator QString() const;

private:
  const QVector<Item> _script;
  Item::Iterator _iterator;
  State::Status _scriptStatus;
  QDBusError _scriptError;
};

#endif // AUTOMATORSCRIPT_H
