#ifndef AUTOMATORSCRIPT_H
#define AUTOMATORSCRIPT_H

#include "types.h"
#include <QtCore/QVariant>

class AutomatorScript : public QObject
{
  Q_OBJECT
public:
  static const State emptyState;
  enum Type
  {
    _EMPTYTYPE_ = 0,
    ManagerModemAdded,
    ModemLockdownDisable,
    ModemLockdownEnable,
    ModemPoweredDisable,
    ModemPoweredEnable,
    ModemOnlineDisable,
    ModemOnlineEnable,
    SimManagerAdded,
    NetworkRegistrationAdded,
    ConnectionManagerAdded,
    ConnectionContextAdded,
    ConnectionContextAccessPointName,
    ConnectionContextUsername,
    ConnectionContextPassword,
    ConnectionContextActiveDisable,
    ConnectionContextActiveEnable
  };

  struct Data
  {
    QVariant modemLockdown;
    QVariant modemPowered;
    QVariant modemOnline;
    QVariant simManagerCardIdentifier;
    QVariant simManagerServiceProviderName;
    QVariant networkRegistrationStatus;
    QVariant connectionManagerAttached;
    QVariant connectionManagerPowered;
    QVariant connectionContextAccessPointName;
    QVariant connectionContextUsername;
    QVariant connectionContextPassword;
    QVariant connectionContextActive;
    Data() = default;
    void debug();
  };

  struct Item
  {
    typedef QVector<Item>::const_iterator Iterator;
    typedef void (*StateItemCommand)(Iterator &iterator, QObject *sender, const Data &data);
    State state;
    StateItemCommand command;
    Item(const State &_state, StateItemCommand _command = nullptr);
    bool operator==(const State &state) const;
    bool operator!=(const State &state) const;
  };

public:
  AutomatorScript(const AutomatorScript::Type type, const QVector<Item> &script, QObject *parent = nullptr);
  ~AutomatorScript() = default;
  State::Status status() const;
  Type type() const;
  void reset();
  void processing(QObject *sender, const State &state, const Data &data);
  operator QString() const;

Q_SIGNALS:
  void StatusChanged(const State::Status status, const State &state);

private:
  const Type _type;
  State::Status _status;
  const QVector<Item> _script;
  Item::Iterator _iterator;
};

#endif // AUTOMATORSCRIPT_H
