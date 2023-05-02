#
# Copyright © 2022 JSC "VNIIZHT" or its affiliates. All Rights Reserved.
# This software is distributed under the terms of the MIT License.
#

equals(TEMPLATE, lib){

}else{
    CYPHALPP_LINK_COMPILED_PWD = $$PWD/prdt
    
    qt{
        CYPHALPP_LINK_COMPILED_LIB = prdt
        QT += testlib
        QT -= gui
        CONFIG += console warn_on depend_includepath testcase
        CONFIG -= app_bundle
    }
    
    include($$PWD/../qt/cyphalpp.pri){
    }else{
        error("Failed to include qt/cyphalpp.pri !")
    }
}
