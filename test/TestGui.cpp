#include "TestGui.h"
#include "ui_TestGui.h"

#include "Global.h"

TestGui::TestGui(QWidget *parent) : QWidget(parent), mm(new ModemManager)
{
  setupUi(this);
  connect(mm, &ModemManager::OfonoStateChanged, this, &TestGui::OfonoStateChanged);

  connect(pbModemPowered, &QPushButton::clicked, mm, &ModemManager::t_modemPowered);
  connect(pbModemOnline, &QPushButton::clicked, mm, &ModemManager::t_modemOnline);
  connect(pbModemLockdown, &QPushButton::clicked, mm, &ModemManager::t_modemLockdown);

  connect(pbContextAPN, &QPushButton::clicked, [this]() { mm->t_contextSetAPN(contextAPN->text()); });
  connect(pbContextUsername, &QPushButton::clicked, [this]() { mm->t_contextSetUsername(contextUsername->text()); });
  connect(pbContextPassword, &QPushButton::clicked, [this]() { mm->t_contextSetPassword(contextPassword->text()); });

  connect(pbContextActive, &QPushButton::clicked, mm, &ModemManager::t_contextSetActive);
}

TestGui::~TestGui()
{
}

void TestGui::OfonoStateChanged(const ModemManager::OfonoState &state)
{
  out->clear();
  out->append("------------ OfonoState ------------");
  out->append("Ofono connected: " + toString(state.isOfonoConnected));
  if (!state.isOfonoConnected)
    return;

  if (state.modem.isNull())
    return;

  out->append("----Modem----");
  out->append("Powered     : " + toString(state.modem->powered));
  out->append("Online      : " + toString(state.modem->online));
  out->append("Lockdown    : " + toString(state.modem->lockdown));
  out->append("Manufacturer: " + state.modem->manufacturer);
  out->append("Model       : " + state.modem->model);
  out->append("Serial      : " + state.modem->serial);

  if (!state.simManager.isNull())
  {
    out->append("----SimManager----");
    out->append("CardIdentifier     : " + state.simManager->cardIdentifier);
    out->append("ServiceProviderName: " + state.simManager->serviceProviderName);
  }

  if (!state.networkRegistration.isNull())
  {
    out->append("----NetworkRegistration----");
    out->append("Status  : " + state.networkRegistration->status);
    out->append("Name    : " + state.networkRegistration->name);
    out->append("Strength: " + state.networkRegistration->strength);
  }

  if (!state.connectionManager.isNull())
  {
    out->append("----ConnectionManager----");
    out->append("Attached      : " + toString(state.connectionManager->attached));
    out->append("RoamingAllowed: " + toString(state.connectionManager->roamingAllowed));
    out->append("Powered       : " + toString(state.connectionManager->powered));
  }

  if (!state.connectionContext.isNull())
  {
    out->append("--ConnectionContext--");
    out->append("Active              : " + toString(state.connectionContext->active));
    out->append("AccessPointName     : " + state.connectionContext->accessPointName);
    out->append("Username            : " + state.connectionContext->username);
    out->append("Password            : " + state.connectionContext->password);
    out->append("Type                : " + state.connectionContext->type);
    out->append("AuthenticationMethod: " + state.connectionContext->authenticationMethod);
    out->append("Protocol            : " + state.connectionContext->protocol);
    out->append("Name                : " + state.connectionContext->name);
    out->append("Interface           : " + state.connectionContext->interface);
    out->append("Method              : " + state.connectionContext->method);
    out->append("Address             : " + state.connectionContext->address);
    out->append("Netmask             : " + state.connectionContext->netmask);
  }
}
