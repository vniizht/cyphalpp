#
# Copyright © 2022 JSC "VNIIZHT" or its affiliates. All Rights Reserved.
# This software is distributed under the terms of the MIT License.
#

TEMPLATE = subdirs
SUBDIRS = \
    prdt 

TESTS = \
    test_asio \
    test_file_server \
    test_node_health \
    test_registers_qt

for(TEST, TESTS){
    eval($${TEST}.depends = prdt)
    SUBDIRS += $${TEST}
}
