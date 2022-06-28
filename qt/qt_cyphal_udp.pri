#
# Copyright © 2022 JSC "VNIIZHT" or its affiliates. All Rights Reserved.
# This software is distributed under the terms of the MIT License.
#

INCLUDEPATH += $$PWD

QT *= core network
HEADERS += \
    $$PWD/qt_cyphal_udp.hpp

SOURCES += \
    $$PWD/qt_cyphal_udp.cpp


!equals(CYPHALPP_DISABLE_UTILITY, 1){
HEADERS += \
    $$PWD/fileserver.hpp \
    $$PWD/noderegisters.h \
    $$PWD/nodeshealthmodel.h \
    $$PWD/values_enum.hpp

SOURCES += \
    $$PWD/fileserver.cpp \
    $$PWD/noderegisters.cpp \
    $$PWD/nodeshealthmodel.cpp
}
