// Загрузка настроек (файл settings.json должен лежать в каталоге с исполняемым бинарником)
#ifndef UTILS_H
#define UTILS_H

#include "ModemManagerData.h"

ModemManagerData::Settings loadSettings(const QString &settingsPath);

#endif // UTILS_H
