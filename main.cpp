#include "ModemManager.h"
#include "test/TestGui.h"
#include "utils.h"
#include <QApplication>

int main(int argc, char *argv[])
{
  QApplication a(argc, argv);

  const ModemManagerData::Settings settings = loadSettings("settings.json");
  // settings.debug();
  ModemManager *mm = new ModemManager(settings, &a);
  TestGui *gui = new TestGui();

  QObject::connect(mm, &ModemManager::OfonoStateChanged, gui, &TestGui::OfonoStateChanged);
  QObject::connect(gui->pbModemPowered, &QPushButton::clicked, mm, &ModemManager::t_modemPowered);
  QObject::connect(gui->pbModemOnline, &QPushButton::clicked, mm, &ModemManager::t_modemOnline);
  QObject::connect(gui->pbModemLockdown, &QPushButton::clicked, mm, &ModemManager::t_modemLockdown);

  QObject::connect(gui->pbNetworkRegistered, &QPushButton::clicked, mm, &ModemManager::t_networkRegistered);
  QObject::connect(gui->pbNetworkUnregistered, &QPushButton::clicked, mm, &ModemManager::t_networkUnregistered);
  QObject::connect(gui->pbNetworkScan, &QPushButton::clicked, mm, &ModemManager::t_networkScan);

  QObject::connect(gui->pbContextAPN, &QPushButton::clicked,
                   [mm, gui]() { mm->t_contextSetAPN(gui->contextAPN->text()); });
  QObject::connect(gui->pbContextUsername, &QPushButton::clicked,
                   [mm, gui]() { mm->t_contextSetUsername(gui->contextUsername->text()); });
  QObject::connect(gui->pbContextPassword, &QPushButton::clicked,
                   [mm, gui]() { mm->t_contextSetPassword(gui->contextPassword->text()); });
  QObject::connect(gui->pbContextActive, &QPushButton::clicked, mm, &ModemManager::t_contextSetActive);

  gui->show();

  return a.exec();
}
