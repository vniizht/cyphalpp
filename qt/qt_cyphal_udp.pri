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
}
}
