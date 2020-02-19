#include "utils.h"
#include "Global.h"
#include <QtCore/QFile>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QJsonParseError>

ModemManagerData::Settings loadSettings(const QString &settingsPath)
{
  QFile file(settingsPath);
  if (!file.exists())
    astr_global::Exception("Отсутствует файл настроек для AutoConnectionManager: " + file.fileName());

  if (!file.open(QIODevice::ReadOnly))
    astr_global::Exception("Не усдалось открыть файл настроек для AutoConnectionManager: " + file.fileName());

  QJsonParseError error;
  const QByteArray data(file.readAll());
  file.close();
  QJsonDocument document = QJsonDocument::fromJson(data, &error);

  if (QJsonParseError::NoError != error.error)
    astr_global::Exception("Не удалось прочитать json структуру из файла настроек для AutoConnectionManager: " +
                           file.fileName());

  QJsonObject root = document.object();
  QJsonObject item = root.value("DBusTimeouts").toObject();
  ModemManagerData::Settings::DBusTimeouts dBusTimeouts(
      item.value("Manager").toInt(), item.value("Modem").toInt(), item.value("SimManager").toInt(),
      item.value("NetworkRegistration").toInt(), item.value("ConnectionManager").toInt(),
      item.value("ConnectionContext").toInt());

  item = root.value("AutoConnectionTimeouts").toObject();
  ModemManagerData::Settings::AutomatorTimeouts automatorTimeouts(item.value("ConnectionManagerAttached").toInt(),
                                                                  item.value("ErrorRepeat").toInt());
  ModemManagerData::Settings settings(dBusTimeouts, automatorTimeouts);
  QJsonObject providers = root.value("Providers").toObject();
  for (const QString &name : providers.keys())
  {
    item = providers.value(name).toObject();
    ModemManagerData::Settings::Provider provider(item.value("AccessPointName").toString(),
                                                  item.value("Username").toString(), item.value("Password").toString());
    settings.addProvider(name, provider);
  }
  return settings;
}
