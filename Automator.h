#ifndef AUTOMATOR_H
#define AUTOMATOR_H

#include "ModemManagerData.h"
#include "ScriptsTypes.h"
#include <QObject>

class Automator : public QObject
{
  Q_OBJECT
public:
  explicit Automator(const ModemManagerData::Settings &settings, QObject *parent = nullptr);

public:
  void processing(QObject *sender, const State &state);
  void run(QObject *adapter, const State::Type type, const QVariant &value);

private Q_SLOTS:
  void onStatusChanged(const Scripts::Basic::Status status);

private:
  const ModemManagerData::Settings &_settings;
  Scripts::Basic _managerModemRemoved;
  Scripts::Basic _managerModemAdded;
  Scripts::Basic _modemLockdownDisable;
  Scripts::Basic _modemLockdownEnable;
  Scripts::Basic _modemPoweredDisable;
  Scripts::Basic _modemPoweredEnable;
  Scripts::Basic _modemOnlineDisable;
  Scripts::Basic _modemOnlineEnable;
  Scripts::Data _data;
};

#endif // AUTOMATOR_H
