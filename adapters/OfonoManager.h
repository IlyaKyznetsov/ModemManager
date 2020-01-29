#ifndef OFONOMANAGER_H
#define OFONOMANAGER_H

#include <QObject>
#include <types.h>

class QDBusServiceWatcher;

class OfonoManager : public QObject
{
    Q_OBJECT
public:
    explicit OfonoManager(QObject *parent = nullptr);
    void reset(const QString& service="org.ofono");

Q_SIGNALS:
    void StateChanged(State state);

private:
    QDBusServiceWatcher *_watcher;
};

#endif // OFONOMANAGER_H
