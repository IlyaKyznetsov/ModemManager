#ifndef CONNECTIONCONTEXT_H
#define CONNECTIONCONTEXT_H

#include "types.h"
#include <QObject>

class OfonoConnectionContextInterface;

class ConnectionContext : public QObject
{
  Q_OBJECT
public:
  explicit ConnectionContext(QObject *parent = nullptr);
  bool isValid() const;
  void reset(const QString &path = QString());
  void call(const State::Type type, const QVariant &value);

Q_SIGNALS:
  void StateChanged(const State &state);

private:
  OfonoConnectionContextInterface *_interface;
  State::Type _currentCallType;
};

#endif // CONNECTIONCONTEXT_H
