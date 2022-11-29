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
DSDL_INPUTS = $$system($$NNVG --generate-support only --list-inputs -O $${OUT_PWD} | sed 's/\;/\\\\n/g')
DSDL_OUTPUTS = $$system($$NNVG --generate-support only --list-outputs -O $${OUT_PWD} | sed 's/\;/\\\\n/g')
I=0
for(DSDL_DIR, DSDL_DIRS){
    DSDL_DIR_INPUTS = $$system($$NNVG --generate-support never --list-inputs -O $${OUT_PWD} $${DSDL_DIR} | sed 's/\;/\\\\n/g')
    DSDL_DIR_OUTPUTS = $$system($$NNVG --generate-support never --list-outputs -O $${OUT_PWD} $${DSDL_DIR} | sed 's/\;/\\\\n/g')
    qt{
    DSDL_DIR_OUTPUTS += $$system($$NNVG --generate-support never --list-outputs -O $${OUT_PWD} -e .qt.hpp --templates $$PWD/qt_dsdl_interface --omit-serialization-support $${DSDL_DIR} | sed 's/\;/\\\\n/g')
    }
    DSDL_INPUTS += $${DSDL_DIR_INPUTS}
    DSDL_OUTPUTS += $$DSDL_DIR_OUTPUTS
}

nnvg_phony.name = nnvg_phony ${QMAKE_FILE_IN}
nnvg_phony.input = DSDL_OUTPUTS
nnvg_phony.output = ${QMAKE_FILE_IN}
nnvg_phony.dependency_type = TYPE_UI
nnvg_phony.depends = nnvg.done.h
nnvg_phony.commands = @true
nnvg_phony.clean_commands = @true
nnvg_phony.variable_out = HEADERS

nnvg.name = nnvg ${QMAKE_FILE_IN}
nnvg.input = DSDL_DIRS
nnvg.output = nnvg.done.h
nnvg.dependency_type = TYPE_UI
nnvg.depends = $$DSDL_INPUTS
nnvg.commands = $$NNVG -O ${QMAKE_FILE_OUT_PATH} --generate-support only &&
nnvg.commands = $${nnvg.commands} for NNVG__FILE__IN__ in ${QMAKE_FILE_IN}; do
nnvg.commands = $${nnvg.commands} $$NNVG --generate-support never -O ${QMAKE_FILE_OUT_PATH} \$\$NNVG__FILE__IN__
qt{
nnvg.commands = $${nnvg.commands} && $$NNVG -e .qt.hpp --templates $$PWD/qt_dsdl_interface --omit-serialization-support -O ${QMAKE_FILE_OUT_PATH} \$\$NNVG__FILE__IN__
}
nnvg.commands = $${nnvg.commands} && touch nnvg.done.h; done
nnvg.CONFIG += target_predeps no_link combine
nnvg.variable_out = HEADERS
nnvg.clean_commands = rm -rf $$DSDL_OUTPUTS
silent:nnvg.commands = @echo nnvg ${QMAKE_FILE_IN} && $$nnvg.commands


INCLUDEPATH += \
    $$OUT_PWD 

QMAKE_EXTRA_COMPILERS += nnvg nnvg_phony

