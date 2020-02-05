#ifndef DEFERREDCALL_H
#define DEFERREDCALL_H

#include "types.h"
#include <QObject>
#include <QTimer>

class DeferredCall : public QObject
{
  Q_OBJECT
public:
  explicit DeferredCall(const int &msCallTimeout, QObject *parent = nullptr);
  ~DeferredCall();

Q_SIGNALS:
  void SendCallData(const State::Type type, const QVariant &value);

public:
  bool deferredCall(const State::Type type, const QVariant &value);
  bool reset(const State::Type type);

private:
  State::Type _type;
  QVariant _value;
  QSharedPointer<QTimer> _timer;
};

#endif // DEFERREDCALL_H
