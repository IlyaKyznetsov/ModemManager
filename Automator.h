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

Q_SIGNALS:
  void Call(const State::Type callType, const QVariant &callValue);

public:
  void processing(const State &state);
  void reset();

private:
  struct Data
  {
    bool needPowerOff = false;
    bool modemLockdown = false;
    bool modemPowered = false;
    bool modemOnline = false;

    bool simManagerInitialized = false;
    bool networkRegistrationInitialized = false;
    bool connectionManagerInitialized = false;
    bool connectionContextInitialized = false;

    bool connectionManagerAttached = false;
    bool connectionManagerPowered = false;
    bool connectionContextActive = false;
    QString simManagerCardIdentifier;
    QString simManagerProviderName;
    QString networkRegistrationStatus;
    QString connectionContextAccessPointName;
    QString connectionContextUsername;
    QString connectionContextPassword;
    //
    State::Status modemLockdownStatus = State::_EMPTYSTATUS_;
    State::Status modemPoweredStatus = State::_EMPTYSTATUS_;
    State::Status modemOnlineStatus = State::_EMPTYSTATUS_;
    State::Status connectionContextAccessPointNameStatus = State::_EMPTYSTATUS_;
    State::Status connectionContextUsernameStatus = State::_EMPTYSTATUS_;
    State::Status connectionContextPasswordStatus = State::_EMPTYSTATUS_;
    State::Status connectionContextActiveStatus = State::_EMPTYSTATUS_;
    //
    Data() = default;
    inline void resetConnectionContext();
    inline void resetConnectionManager();
    inline void resetNetworkRegistration();
    inline void resetSimManager();
    inline void resetModem();
    void debug();
  };

  const ModemManagerData::Settings &_settings;
  Data _data;
};

#endif // AUTOMATOR_H
