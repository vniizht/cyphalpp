#
# Copyright © 2022 JSC "VNIIZHT" or its affiliates. All Rights Reserved.
# This software is distributed under the terms of the MIT License.
#

TEMPLATE = app

include($$PWD/../asio_test.pri)

LIBS += -lboost_filesystem

SOURCES += \
    main.cpp


