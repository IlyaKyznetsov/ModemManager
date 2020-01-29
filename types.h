#ifndef TYPES_H
#define TYPES_H

#include <QVariant>
#include <QtDBus/QDBusError>

#include "Global.h"

class State
{
public:
  enum Type
  {
    NONETYPE = 0, // for default constructor only
    // Local,
    Reset,
    // Ofono Service
    OfonoServiceRegistered,
    OfonoServiceUnregistered,
    // Ofono Manager
    OfonoModemAdded,
    OfonoModemRemoved,
    OfonoGetModems
  } _type;

  enum StateType
  {
    NONESTATE = 0,
    Signal,
    Started,
    Finished,
    Error
  } _state;

  QVariant _value;
  QDBusError _error;

  State() : _type(NONETYPE), _state(NONESTATE)
  {
  }

  State(Type type, StateType state) : _type(type), _state(state)
  {
    // _error.type() = NoError
    // _value = QVariant(Invalid);
  }

  State(Type type, StateType state, const QVariant &value)
  : _type(type), _state(state), _value(value)
  {
  }

  State(Type type, StateType state, const QDBusError &error)
  : _type(type), _state(state), _error(error)
  {
  }

  State(Type type, StateType state, const QVariant &value, const QDBusError &error)
  : _type(type), _state(state), _value(value), _error(error)
  {
  }
};

#include <qmetatype.h>
Q_DECLARE_METATYPE(State)

#endif // TYPES_H
