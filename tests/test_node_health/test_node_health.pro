#
# Copyright © 2022 JSC "VNIIZHT" or its affiliates. All Rights Reserved.
# This software is distributed under the terms of the MIT License.
#
TEMPLATE = app
CONFIG += qt


include($$PWD/../tests.pri){
}else{
    error("Error including tests!")
}


SOURCES +=  tst_node_health.cpp
