#ifndef MODEMMANAGER_H
#define MODEMMANAGER_H

#include <QObject>
#include "adapters/OfonoManager.h"
#include "adapters/Manager.h"

#include <QSharedPointer>

class ModemManager : public QObject
{
    Q_OBJECT
public:
    explicit ModemManager(QObject *parent = nullptr);

signals:

private Q_SLOTS:
    void onStateChanged(State state);
private:
    OfonoManager* _ofonoManager;
    Manager* _manager;
};

#endif // MODEMMANAGER_H
