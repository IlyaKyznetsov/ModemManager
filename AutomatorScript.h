#ifndef AUTOMATORSCRIPT_H
#define AUTOMATORSCRIPT_H

#include "types.h"
#include <QtCore/QVariant>

class AutomatorScript : public QObject
{
  Q_OBJECT
public:
  static const State emptyState;

  enum ScriptStatus
  {
    NotStarted = 0,
    Started,
    Finished,
    Error
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
    void clearModem();
    void clearSimManager();
    void clearNetworkRegistration();
    void clearConnectionManager();
    void clearConnectionContext();
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
  AutomatorScript(const QVector<Item> &script, QObject *parent = nullptr);
  ~AutomatorScript() = default;
  ScriptStatus status() const;
  void reset();
  void processing(QObject *sender, const State &state, const Data &data);

Q_SIGNALS:
  void StatusChanged(const ScriptStatus status);

private:
  ScriptStatus _status;
  const QVector<Item> _script;
  Item::Iterator _iterator;
};

#endif // AUTOMATORSCRIPT_H
