CONFIG -= qt
CONFIG -= app_bundle
CONFIG += console


CYPHALPP_BACKEND = asio
INCLUDEPATH += \
    $$PWD/common
HEADERS += \
    $$PWD/common/asio_fixture.hpp

isEmpty(BOOST_INCLUDE_DIR): BOOST_INCLUDE_DIR=$$(BOOST_INCLUDE_DIR)
!isEmpty(BOOST_INCLUDE_DIR): INCLUDEPATH *= $${BOOST_INCLUDE_DIR}

isEmpty(BOOST_INCLUDE_DIR): {
    message("BOOST_INCLUDE_DIR is not set, assuming Boost can be found automatically in your system")
}
include($$PWD/tests.pri)
