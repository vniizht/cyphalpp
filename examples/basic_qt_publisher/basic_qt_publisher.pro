#
# Copyright © 2022 JSC "VNIIZHT" or its affiliates. All Rights Reserved.
# This software is distributed under the terms of the MIT License.
#

QT -= gui

CONFIG += c++17 console
CONFIG -= app_bundle

isEmpty(PRDT_PATH){
    PRDT_PATH=$$OUT_PWD/prdt
}
exists($$PRDT_PATH/README.md){
}else{
    isEmpty(PRDT_URL){
        PRDT_URL="https://github.com/OpenCyphal/public_regulated_data_types.git"
    }
    system(git clone $$PRDT_URL $$PRDT_PATH){
    }else{
        error("Failed fetching public regulated data types from $$PRDT_URL to $$PRDT_PATH")
    }
}


DSDL_DIRS += $$PRDT_PATH/uavcan

include($$PWD/../../qt/cyphalpp.pri){
}else{
  error("Failed to include cyphalpp.pri !")
}


SOURCES += \
        main.cpp

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
