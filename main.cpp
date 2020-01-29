#include <QCoreApplication>
#include "ModemManager.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    ModemManager mm;

    return a.exec();
}
