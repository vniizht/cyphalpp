//
// Copyright © 2022 JSC "VNIIZHT" or its affiliates. All Rights Reserved.
// This software is distributed under the terms of the MIT License.
//
#ifndef CYPHALPP_REMOTEREGISTRY_IMPL_HPP
#define CYPHALPP_REMOTEREGISTRY_IMPL_HPP
#include "cyphalppremoteregistry.hpp"
#include "cyphalppstdstorageregistry_impl.hpp"

namespace cyphalpp{
namespace registry{

RemoteRegistry::RemoteRegistry(): cy(nullptr), list(), access(){
}

/**
 * @brief set sets a new value for a field
 * @param n name of a value to set
 * @param v value to set
 * @return
 */
Access::Response RemoteRegistry::set(const Name& n, const Value& v){
    return StdStorageRegistry::set(n, v);
}

/**
 * @brief load should load values from persistent store
 */
void RemoteRegistry::load(){
    if(not cy) return;
}
/**
 * @brief save should save values to persistent store
 */
void RemoteRegistry::save(){
    if(not cy) return;
}

RemoteRegistry::~RemoteRegistry(){

}

void RemoteRegistry::connect(){
    list = cy->prepareServiceCalls<List>([](List::Request& rq) -> List::Response{

    }, [](){

    });
    access = cy->prepareServiceCalls<Access>([](Access::Request& rq) -> Access::Response{

    }, [](){
        
    });
}

void RemoteRegistry::disconnect(){
    list = nullptr;
    access = nullptr;
    cy = nullptr;
}

} // namespace registry
} // namespace cyphalpp

#endif // ndef CYPHALPP_REMOTEREGISTRY_IMPL_HPP