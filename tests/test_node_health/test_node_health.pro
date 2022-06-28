#
# Copyright © 2022 JSC "VNIIZHT" or its affiliates. All Rights Reserved.
# This software is distributed under the terms of the MIT License.
#

QT += testlib
QT -= gui

include($$PWD/../tests.pri){
}else{
    error("Error including tests!")
}

CONFIG += qt console warn_on depend_includepath testcase
CONFIG -= app_bundle

TEMPLATE = app

SOURCES +=  tst_node_health.cpp
