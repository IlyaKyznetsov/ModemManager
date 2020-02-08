#ifndef AUTOMATOR_H
#define AUTOMATOR_H

#include <QObject>
#include "types.h"

class Automator : public QObject
{
    Q_OBJECT
public:
    explicit Automator(QObject *parent = nullptr);
    void stateChangedHandler(QObject *sender_ptr, const State &nextState);

private:
    const QVector<State>_statesTodo;
    QVector<State>::const_iterator _stateCurrent;
};

#endif // AUTOMATOR_H
