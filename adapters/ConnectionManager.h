#ifndef CONNECTIONMANAGER_H
#define CONNECTIONMANAGER_H

#include "types.h"
#include <QObject>

class OfonoConnectionManagerInterface;

class ConnectionManager : public QObject
{
  Q_OBJECT
public:
  explicit ConnectionManager(QObject *parent = nullptr);
  bool isValid() const;
  QString contextPath() const;
  void reset(const QString &path = QString());
  void call(const State::Type type);
  void call(const State::Type type, const QVariant &value);

Q_SIGNALS:
  void StateChanged(const State &state);

private:
  OfonoConnectionManagerInterface *_interface;
  State::Type _currentCallType;
  QStringList _contextsPath;
  inline void _getProperties();
  inline void _getContexts();
};

#endif // CONNECTIONMANAGER_H
