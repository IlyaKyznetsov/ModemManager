#ifndef TESTGUI_H
#define TESTGUI_H

#include "ui_TestGui.h"
#include <ModemManager.h>
#include <QWidget>

class TestGui : public QWidget, public Ui::TestGui
{
  Q_OBJECT

public:
  explicit TestGui(QWidget *parent = nullptr);
  ~TestGui();

public Q_SLOTS:
  void OfonoStateChanged(const ModemManagerData::OfonoState &state);
};

#endif // TESTGUI_H
