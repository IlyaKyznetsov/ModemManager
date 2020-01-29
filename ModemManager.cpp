#include "ModemManager.h"
#include "Global.h"

ModemManager::ModemManager(QObject *parent)
: QObject(parent), _ofonoManager(new OfonoManager(this)), _manager(new Manager(this))
{
  connect(_ofonoManager, &OfonoManager::StateChanged, this, &ModemManager::onStateChanged);
  connect(_manager, &Manager::StateChanged, this, &ModemManager::onStateChanged);
  _ofonoManager->reset();
}

void ModemManager::onStateChanged(State state)
{
  DF() << state._type << state._state;
  switch (state._type)
  {
  case State::OfonoServiceRegistered:
  {
    D("OfonoServiceRegistered");
    if (state._state == State::Signal)
    {
      _manager->reset("org.ofono");
    }
  }
  break;
  case State::OfonoServiceUnregistered:
  {
    D("OfonoServiceUnregistered");
    _manager->reset();
  }
  }
}
