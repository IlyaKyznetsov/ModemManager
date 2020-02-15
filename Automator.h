#ifndef AUTOMATOR_H
#define AUTOMATOR_H

#include "AutomatorScript.h"
#include "ModemManagerData.h"
#include <QObject>
#include <QTimer>

class Automator : public QObject
{
  Q_OBJECT
public:
  explicit Automator(const ModemManagerData::Settings &settings, QObject *parent = nullptr);
  ~Automator();

public:
  void processing(QObject *sender, const State &state);

Q_SIGNALS:
  void Call(const State::Type callType, const QVariant &value);

private Q_SLOTS:
  void automatorScriptStatusChanged(const State::Status status, const State &state);

private:
  void autoConnection(const State::Status status, const AutomatorScript::Type type, const State &state);
  const ModemManagerData::Settings &_settings;
  QScopedPointer<QTimer> _timer;
  QMap<AutomatorScript::Type, AutomatorScript*> _scripts;
  AutomatorScript::Data _data;
  void debugScriptsRunning();
};

#endif // AUTOMATOR_H
