TEMPLATE = lib
CONFIG += qt static

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

isEmpty(PRDT_PATH){
    PRDT_PATH=$$OUT_PWD/../prdt
}
DSDL_DIRS += $$PRDT_PATH/uavcan
include($$PWD/../../qt/cyphalpp.pri)
