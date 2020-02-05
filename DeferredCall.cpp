#include "DeferredCall.h"

DeferredCall::DeferredCall(const int &msCallTimeout, QObject *parent)
    : QObject(parent), _type(State::_EMPTYTYPE_), _timer(new QTimer())
{
  _timer->setSingleShot(true);
  _timer->setInterval(msCallTimeout);
  connect(_timer.data(), &QTimer::timeout, [this]() {
    if (State::_EMPTYTYPE_ != _type)
      Q_EMIT SendCallData(_type, _value);
  });
}

DeferredCall::~DeferredCall()
{
  if (!_timer.isNull())
    _timer->stop();
  _timer.reset();
  _type = State::Reset;
  _value.clear();
}

bool DeferredCall::deferredCall(const State::Type type, const QVariant &value)
{
  if (State::_EMPTYTYPE_ != _type)
    return false;

  _type = type;
  _value = value;
  _timer->start();
  return true;
}

bool DeferredCall::reset(const State::Type type)
{
  if (type == State::_EMPTYTYPE_)
    return false;

  _type = State::_EMPTYTYPE_;
  _value.clear();
  _timer->stop();
  return true;
}
