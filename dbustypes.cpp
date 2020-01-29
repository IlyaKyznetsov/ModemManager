#include "dbustypes.h"
#include <qmetatype.h>
#include <qdbusmetatype.h>

void Ofono::registerObjectPathProperties()
{
    qDBusRegisterMetaType<ObjectPathProperties>();
    qDBusRegisterMetaType<ObjectPathPropertiesList>();
    qRegisterMetaType<ObjectPathProperties>("ObjectPathProperties");
    qRegisterMetaType<ObjectPathPropertiesList>("ObjectPathPropertiesList");
}

QDBusArgument &operator<<(QDBusArgument &arg, const ObjectPathProperties &props)
{
    arg.beginStructure();
    arg << props.path << props.properties;
    arg.endStructure();
    return arg;
}

const QDBusArgument &operator>>(const QDBusArgument &arg, ObjectPathProperties &props)
{
    arg.beginStructure();
    arg >> props.path >> props.properties;
    arg.endStructure();
    return arg;
}
