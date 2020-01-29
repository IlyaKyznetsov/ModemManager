#include "Global.h"

QString toString(const bool x)
{
  return (x ? "true" : "false");
}

void F(const QString &msg)
{
  qFatal(QByteArray("F: ").append(msg.toLocal8Bit()).data());
}

#include <QFile>
#include <QTextStream>
#include <iostream>
void messageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
  static QFile log("/tmp/out.log");
  if (!log.isOpen())
    log.open(QIODevice::WriteOnly | QIODevice::Text);
  Q_UNUSED(context);
  QByteArray localMsg = msg.toLocal8Bit();
  QTextStream out(&log);
  switch (type)
  {
    case QtDebugMsg:
    case QtInfoMsg:
    case QtWarningMsg:
    case QtCriticalMsg:
    case QtFatalMsg: out << localMsg.constData() << '\n'; std::cout << localMsg.constData() << std::endl;
  }
}
