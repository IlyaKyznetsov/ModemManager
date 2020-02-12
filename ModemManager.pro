QT += dbus

QT += widgets

CONFIG += c++11 console
CONFIG -= app_bundle

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

DBUS_INTERFACES += ofono_connection_context
ofono_connection_context.files = dbus/ofono_connection_context.xml
ofono_connection_context.header_flags = -N -c OfonoConnectionContextInterface
ofono_connection_context.source_flags = -N -c OfonoConnectionContextInterface

DBUS_INTERFACES += ofono_connection_manager
ofono_connection_manager.files = dbus/ofono_connection_manager.xml
ofono_connection_manager.header_flags = -N -c OfonoConnectionManagerInterface -i adapters/dbustypes.h
ofono_connection_manager.source_flags = -N -c OfonoConnectionManagerInterface

DBUS_INTERFACES += ofono_manager
ofono_manager.files = dbus/ofono_manager.xml
ofono_manager.header_flags = -N -c OfonoManagerInterface -i adapters/dbustypes.h
ofono_manager.source_flags = -N -c OfonoManagerInterface

DBUS_INTERFACES += ofono_modem
ofono_modem.files = dbus/ofono_modem.xml
ofono_modem.header_flags = -N -c OfonoModemInterface
ofono_modem.source_flags = -N -c OfonoModemInterface

DBUS_INTERFACES += ofono_network_operator
ofono_network_operator.files = dbus/ofono_network_operator.xml
ofono_network_operator.header_flags = -N -c OfonoNetworkOperatorInterface
ofono_network_operator.source_flags = -N -c OfonoNetworkOperatorInterface

DBUS_INTERFACES += ofono_network_registration
ofono_network_registration.files = dbus/ofono_network_registration.xml
ofono_network_registration.header_flags = -N -c OfonoNetworkRegistrationInterface -i adapters/dbustypes.h
ofono_network_registration.source_flags = -N -c OfonoNetworkRegistrationInterface

DBUS_INTERFACES += ofono_radio_settings
ofono_radio_settings.files = dbus/ofono_radio_settings.xml
ofono_radio_settings.header_flags = -N -c OfonoRadioSettingsInterface
ofono_radio_settings.source_flags = -N -c OfonoRadioSettingsInterface

DBUS_INTERFACES += ofono_simmanager
ofono_simmanager.files = dbus/ofono_simmanager.xml
ofono_simmanager.header_flags = -N -c OfonoSimManagerInterface
ofono_simmanager.source_flags = -N -c OfonoSimManagerInterface

XML_FILES += \
    dbus/ofono_connection_context.xml \
    dbus/ofono_connection_manager.xml \
    dbus/ofono_modem.xml \
    dbus/ofono_network_operator.xml \
    dbus/ofono_network_registration.xml \
    dbus/ofono_simmanager.xml \
    dbus/ofono_manager.xml

OTHER_FILES += $$XML_FILES\
    version.pri \
    qofono-qt5.prf

SOURCES += \
        Automator.cpp \
        AutomatorScript.cpp \
        ModemManagerData.cpp \
        test/TestGui.cpp \
        DeferredCall.cpp \
        Global.cpp \
        ModemManager.cpp \
        adapters/ConnectionContext.cpp \
        adapters/ConnectionManager.cpp \
        adapters/Modem.cpp \
        adapters/NetworkRegistration.cpp \
        adapters/SimManager.cpp \
        adapters/dbustypes.cpp \
        adapters/Manager.cpp \
        adapters/OfonoManager.cpp \
        main.cpp \
        types.cpp \
        utils.cpp



HEADERS += \
    Automator.h \
    AutomatorScript.h \
    ModemManagerData.h \
    test/TestGui.h \
    DeferredCall.h \
    Global.h \
    ModemManager.h \
    adapters/ConnectionContext.h \
    adapters/ConnectionManager.h \
    adapters/Modem.h \
    adapters/NetworkRegistration.h \
    adapters/SimManager.h \
    adapters/dbustypes.h \
    adapters/Manager.h \
    adapters/OfonoManager.h \
    types.h \
    utils.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

FORMS += \
    test/TestGui.ui \
    test/TestGui.ui
