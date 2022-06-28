#
# Copyright © 2022 JSC "VNIIZHT" or its affiliates. All Rights Reserved.
# This software is distributed under the terms of the MIT License.
#

REPO_ROOT = $$PWD/..

INCLUDEPATH += $$REPO_ROOT/include
INCLUDEPATH += $$REPO_ROOT/vendored
include($$REPO_ROOT/qt/dsdl.pri){
}else{
    error("Error finding DSDL includes! Did you forget to run 'git submodule update --init' ?")
}
include($$REPO_ROOT/qt/qt_cyphal_udp.pri){
}else{
    error("Error finding DSDL includes! Did you forget to run 'git submodule update --init' ?")
}
