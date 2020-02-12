#ifndef SCRIPTSTYPES_H
#define SCRIPTSTYPES_H

#include "types.h"
#include <QtCore/QVariant>

namespace Scripts
{
struct Data
{
  QVariant modemLockdown;
  QVariant modemPowered;
  QVariant modemOnline;
  QVariant simManagerCardIdentifier;
  QVariant simManagerServiceProviderName;
  QVariant networkRegistrationStatus;
  QVariant connectionContextAccessPointName;
  QVariant connectionContextUsername;
  QVariant connectionContextPassword;
  QVariant connectionManagerAttached;
  QVariant connectionManagerPowered;
  QVariant connectionContextActive;
  Data() = default;
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

class Basic : public QObject
{
  Q_OBJECT
public:
  enum Status
  {
    _EMPTYSTATUS_ = 0,
    Started,
    Finished,
    Error
  };

public:
  Basic(const QVector<Item> &script, QObject *parent = nullptr);
  ~Basic() = default;
  void reset();
  bool isEnd() const;
  bool isBegin() const;
  void processing(QObject *sender, const State &state, const Scripts::Data &data);

Q_SIGNALS:
  void StatusChanged(const Scripts::Basic::Status status);

private:
  const QVector<Item> _script;
  Item::Iterator _iterator;
};
} // namespace Scripts

#endif // SCRIPTSTYPES_H
