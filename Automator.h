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
  void Call(const State::Type callType, const QVariant &callValue);

private Q_SLOTS:
  void automatorScriptStatusChanged(const State::Status scriptStatus, const State &state);

private:
  const ModemManagerData::Settings &_settings;
  QScopedPointer<QTimer> _timer;
  const QVector<AutomatorScript *> _scripts;
  AutomatorScript::Data _data;
  DeferredCall _call;
  void debugScriptsRunning();
  AutomatorScript *script(const AutomatorScript::Type type);
};

#endif // AUTOMATOR_H
