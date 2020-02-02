#ifndef TESTGUI_H
#define TESTGUI_H

#include <QWidget>
#include "ui_TestGui.h"
#include "ModemManager.h"

class TestGui : public QWidget, public Ui::TestGui
{
    Q_OBJECT

public:
    explicit TestGui(QWidget *parent = nullptr);
    ~TestGui();

private Q_SLOTS:
    void OfonoStateChanged(const ModemManager::OfonoState &state);

private:
    ModemManager *mm;
};

#endif // TESTGUI_H
