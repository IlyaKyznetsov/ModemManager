#ifndef CONNECTIONMANAGER_H
#define CONNECTIONMANAGER_H

#include "types.h"
#include <QObject>

class OfonoConnectionManagerInterface;

class ConnectionManager : public QObject
{
  Q_OBJECT
public:
  explicit ConnectionManager(const int &dbusTimeout, QObject *parent = nullptr);
  bool isValid() const;
  QString contextPath() const;
  bool isContext(const QString &path) const;
  int contextsCount() const;
  void reset(const QString &path = QString());
  void call(const State::Type type, const QVariant &value);

Q_SIGNALS:
  void StateChanged(const State &state);

private:
  const int _dbusTimeout;
  OfonoConnectionManagerInterface *_interface;
  State::Type _currentCallType;
  QStringList _contextsPath;
  inline void _getProperties();
  inline void _getContexts();
};

#endif // CONNECTIONMANAGER_H
