#
# Copyright © 2022 JSC "VNIIZHT" or its affiliates. All Rights Reserved.
# This software is distributed under the terms of the MIT License.
#

REPO_ROOT = $$PWD/..

INCLUDEPATH += $$REPO_ROOT/include
INCLUDEPATH += $$REPO_ROOT/vendored

isEmpty(CYPHALPP_LINK_COMPILED_PWD){
include($$REPO_ROOT/qt/dsdl.pri){
}else{
    error("Error finding DSDL includes! Did you forget to run 'git submodule update --init' ?")
}
}
isEmpty(CYPHALPP_BACKEND){
    CYPHALPP_BACKEND = qt
}

equals(CYPHALPP_BACKEND, qt){
!qt{
    error("You selected a qt backend, but qt is disabled in CONFIG!")
}
HEADERS += \
    $$REPO_ROOT/qt/qt_dsdl_interface/qt_dsdl_support.hpp
INCLUDEPATH += \
    $$REPO_ROOT/qt/qt_dsdl_interface

}

cyphalpp_backend_qt.name = Qt
cyphalpp_backend_qt.include = $$REPO_ROOT/qt/qt_cyphal_udp.pri

cyphalpp_backend_asio.name = Boost::Asio
cyphalpp_backend_asio.include = $$REPO_ROOT/asio/asio_cyphalpp.pri

CYPHALPP_BACKEND_NAME = $$eval(cyphalpp_backend_$${CYPHALPP_BACKEND}.name)
CYPHALPP_BACKEND_INCLUDE = $$eval(cyphalpp_backend_$${CYPHALPP_BACKEND}.include)

include($$CYPHALPP_BACKEND_INCLUDE){
}else{
    error("Error finding $${CYPHALPP_BACKEND_NAME} backend ($$CYPHALPP_BACKEND)!")
}

!isEmpty(CYPHALPP_LINK_COMPILED_PWD){

CYPHALPP_LINK_COMPILED_OUT_PWD = \
    $$clean_path($$OUT_PWD/$$relative_path($$CYPHALPP_LINK_COMPILED_PWD, $$_PRO_FILE_PWD_))

INCLUDEPATH += $$CYPHALPP_LINK_COMPILED_OUT_PWD
DEPENDPATH += $$CYPHALPP_LINK_COMPILED_OUT_PWD

!isEmpty(CYPHALPP_LINK_COMPILED_LIB){
win32:CONFIG(release, debug|release): \
    LIBS += \
        -L$$CYPHALPP_LINK_COMPILED_OUT_PWD/release/ \
        -l$$CYPHALPP_LINK_COMPILED_LIB
else:win32:CONFIG(debug, debug|release): \
    LIBS += \
        -L$$CYPHALPP_LINK_COMPILED_OUT_PWD/debug/ \
        -l$$CYPHALPP_LINK_COMPILED_LIB
else:unix: \
    LIBS += \
        -L$$CYPHALPP_LINK_COMPILED_OUT_PWD/ \
        -l$$CYPHALPP_LINK_COMPILED_LIB
}

}
