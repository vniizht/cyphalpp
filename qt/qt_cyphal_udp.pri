#
# Copyright © 2022 JSC "VNIIZHT" or its affiliates. All Rights Reserved.
# This software is distributed under the terms of the MIT License.
#

INCLUDEPATH += $$PWD

QT *= core network

isEmpty(CYPHALPP_LINK_COMPILED_PWD){
HEADERS += \
    $$PWD/qt_cyphal_udp.hpp
SOURCES += \
    $$PWD/qt_cyphal_udp.cpp


!equals(CYPHALPP_DISABLE_UTILITY, 1){




HEADERS += \
    $$PWD/qt_cyphal_register_values_as_qvariant.hpp \
    $$PWD/qt_cyphal_registry.hpp \
    $$PWD/fileserver.hpp \
    $$PWD/noderegisters.h \
    $$PWD/nodeshealthmodel.h \
    $$PWD/values_enum.hpp

SOURCES += \
    $$PWD/qt_cyphal_register_values_as_qvariant.cpp \
    $$PWD/qt_cyphal_registry.cpp \
    $$PWD/fileserver.cpp \
    $$PWD/noderegisters.cpp \
    $$PWD/nodeshealthmodel.cpp

HEADERS += \
    $$PWD/Registry/Base/ICyphalRegistryProxy.h \
    $$PWD/Registry/Base/Types.h \
    $$PWD/Registry/Convert/NameConverter.h \
    $$PWD/Registry/Convert/ValueConverter.h \
    $$PWD/Registry/Network/NetworkRegistryServer.h \
    $$PWD/Registry/Network/RegistryNodeAcquirer.h \
    $$PWD/Registry/Network/RegistryRouter.h \
    $$PWD/Registry/Register/Register.h \
    $$PWD/Registry/Register/RegisterCasts.h \
    $$PWD/Registry/Storage/QSettingsStorage/QSettingsStorage.h

SOURCES += \
    $$PWD/Registry/Convert/NameConverter.cpp \
    $$PWD/Registry/Convert/ValueConverter.cpp \
    $$PWD/Registry/Network/NetworkRegistryServer.cpp \
    $$PWD/Registry/Network/RegistryNodeAcquirer.cpp \
    $$PWD/Registry/Network/RegistryRouter.cpp \
    $$PWD/Registry/Storage/QSettingsStorage/QSettingsStorage.cpp
}
}

