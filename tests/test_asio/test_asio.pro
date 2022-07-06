#
# Copyright © 2022 JSC "VNIIZHT" or its affiliates. All Rights Reserved.
# This software is distributed under the terms of the MIT License.
#

TEMPLATE = app
CONFIG -= qt
CONFIG -= app_bundle
CONFIG += console

isEmpty(BOOST_INCLUDE_DIR): BOOST_INCLUDE_DIR=$$(BOOST_INCLUDE_DIR)
!isEmpty(BOOST_INCLUDE_DIR): INCLUDEPATH *= $${BOOST_INCLUDE_DIR}

isEmpty(BOOST_INCLUDE_DIR): {
    message("BOOST_INCLUDE_DIR is not set, assuming Boost can be found automatically in your system")
}

INCLUDEPATH *= $$PWD/../../include/
INCLUDEPATH *= $$PWD/../../vendored/
INCLUDEPATH *= $$PWD/../../asio/

HEADERS += \
    $$PWD/../../include/cyphalpp.hpp \
    $$PWD/../../asio/asio_cyphal.hpp

SOURCES += \
    main.cpp


include($$PWD/../tests.pri)
include($$PWD/../../qt/dsdl.pri)
