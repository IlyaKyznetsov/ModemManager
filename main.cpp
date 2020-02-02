#include <QApplication>
//#include "ModemManager.h"
#include "test/TestGui.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    TestGui gui;
    gui.show();

    return a.exec();
}
