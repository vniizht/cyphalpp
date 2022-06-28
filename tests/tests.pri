#
# Copyright © 2022 JSC "VNIIZHT" or its affiliates. All Rights Reserved.
# This software is distributed under the terms of the MIT License.
#

equals(TEMPLATE,subdirs){
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
}else{
    isEmpty(PRDT_PATH){
        PRDT_PATH=$$OUT_PWD/../prdt
    }
    DSDL_DIRS += $$PRDT_PATH/uavcan
    qt{
        include($$PWD/../qt/cyphalpp.pri){
        }else{
            error("Failed to include qt/cyphalpp.pri !")
        }
    }
}
