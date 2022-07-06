#
# Copyright © 2022 JSC "VNIIZHT" or its affiliates. All Rights Reserved.
# This software is distributed under the terms of the MIT License.
#

DSDL_DIRS += \
    $$clean_path($$PWD/../cyphal_udp)

DISTFILES += \
    $$PWD/../cyphal_udp/Header.1.0.dsdl \
    $$PWD/../cyphal_udp/UnicastBridgeHeader.1.0.dsdl


isEmpty(NNVG_EXE){
    NNVG_EXE = nnvg
}
isEmpty(CXX_LANGUAGE_STANDARD){
    CXX_LANGUAGE_STANDARD = c++14
}
NNVG_FLAGS = -Xlang -l cpp --language-standard=$$CXX_LANGUAGE_STANDARD $$DSDL_EXTRA_FLAGS
for(LD, DSDL_LOOKUP_DIRS){
    NNVG_FLAGS += -I $$LD
}
NNVG = $$NNVG_EXE $$NNVG_FLAGS
nnvg.name = nnvg ${QMAKE_FILE_IN}
nnvg.input = DSDL_DIRS
nnvg.output = ${QMAKE_FILE_IN_BASE}.nnvg.done.a
nnvg.dependency_type = TYPE_UI
nnvg.depend_command = $$NNVG --list-inputs -O ${QMAKE_FILE_OUT_PATH} ${QMAKE_FILE_IN} | sed 's/\;/\\\\n/g'
nnvg.commands = $$NNVG -O ${QMAKE_FILE_OUT_PATH} ${QMAKE_FILE_IN}
qt{
nnvg.commands = $${nnvg.commands} && $$NNVG -e .qt.hpp --templates $$PWD/qt_dsdl_interface --omit-serialization-support -O ${QMAKE_FILE_OUT_PATH} ${QMAKE_FILE_IN}
}
nnvg.commands = $${nnvg.commands} && touch ${QMAKE_FILE_OUT}
nnvg.CONFIG += target_predeps no_link
nnvg.clean = ${QMAKE_FILE_OUT}
nnvg.clean_commands = rm -rf ${QMAKE_FILE_OUT_PATH}/${QMAKE_FILE_IN_BASE}
silent:nnvg.commands = @echo nnvg ${QMAKE_FILE_IN} && $$nnvg.commands

I=0
for(DSDL_DIR, DSDL_DIRS){
    DSDL_DIR_OUTPUTS = $$system($$NNVG --list-outputs -O $${OUT_PWD} $${DSDL_DIR} | sed 's/\;/\\\\n/g')
    for(DSDL_DIR_OUTPUT, DSDL_DIR_OUTPUTS){
        eval(dsdl_$${I}.target = $$DSDL_DIR_OUTPUT);
        eval(dsdl_$${I}.depends = $$basename(DSDL_DIR).nnvg.done.a)
        HEADERS += $$DSDL_DIR_OUTPUT
        QMAKE_EXTRA_TARGETS += dsdl_$${I}
        I=$$num_add($$I, 1)
    }
}

HEADERS += \
    $$PWD/qt_dsdl_interface/qt_dsdl_support.hpp
INCLUDEPATH += \
    $$OUT_PWD \
    $$PWD/qt_dsdl_interface
QMAKE_EXTRA_COMPILERS += nnvg

