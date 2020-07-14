// Отладочные макросы

#ifndef GLOBAL_H
#define GLOBAL_H

#if defined(QFONO_LIBRARY)
#define QFONOSHARED_EXPORT Q_DECL_EXPORT
#else
#define QFONOSHARED_EXPORT Q_DECL_IMPORT
#endif

#define QTUDEV_EXPORT Q_DECL_IMPORT
#define QTUDEV_NO_EXPORT Q_DECL_HIDDEN

#include <QDateTime>
#include <QDebug>
#include <QException>
#include <QThread>

QString toString(const bool x);

//#define D(x) qDebug()<<"D: "<<x
#define D(x) qInfo() << "D: " << QTime::currentTime().toString("mm:ss.zzz") << QThread::currentThreadId() << x
#define I(x) qInfo() << "I: " << QTime::currentTime().toString("mm:ss.zzz") << QThread::currentThreadId() << x
#define W(x)                                                                                                           \
  qWarning() << "W: " << QTime::currentTime().toString("mm:ss.zzz") << __FILE__ << __LINE__                            \
             << QThread::currentThreadId() << x
#define C(x) qCritical() << "C: " << QTime::currentTime().toString("mm:ss.zzz") << QThread::currentThreadId() << x
#define DP(x)                                                                                                          \
  qInfo() << "DP:" << QTime::currentTime().toString("mm:ss.zzz") << QThread::currentThreadId() << #x << "=" << x
void F(const QString &msg);

#define DF()                                                                                                           \
  qInfo() << "DF:" << QTime::currentTime().toString("mm:ss.zzz") << QThread::currentThreadId() << __PRETTY_FUNCTION__

void messageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg);

namespace astr_global
{
class Exception
{
public:
  explicit Exception(const QString &msg)
  {
    qCritical() << "Exception:" << msg;
    throw std::exception();
  }
};
} // namespace astr_global

//#include "dbustypes.h"
// void PRINT(const QString &title, const QVariantMap &properties);
// void PRINT(const QString &title, const ObjectPathPropertiesList &objectPathProperties);
#endif // GLOBAL_H
