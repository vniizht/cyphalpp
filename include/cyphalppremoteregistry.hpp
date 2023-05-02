//
// Copyright © 2022 JSC "VNIIZHT" or its affiliates. All Rights Reserved.
// This software is distributed under the terms of the MIT License.
//
#ifndef CYPHALPP_REMOTEREGISTRY_HPP
#define CYPHALPP_REMOTEREGISTRY_HPP
#include "cyphalppstdstorageregistry.hpp"

namespace cyphalpp{
namespace registry{

class RemoteRegistry : public cyphalpp::registry::StdStorageRegistry {
    cyphalpp::CyphalUdp* cy;
    std::shared_ptr<cyphalpp::CyphalUdp::ServiceCaller< List>> list;
    std::shared_ptr<cyphalpp::CyphalUdp::ServiceCaller< Access>> access;
    uint16_t node_id;
public:
    RemoteRegistry();

    /**
     * @brief set sets a new value for a field
     * @param n name of a value to set
     * @param v value to set
     * @return
     */
    virtual Access::Response set(const Name& n, const Value& v) override;

    /**
     * @brief load should load values from persistent store
     */
    virtual void load() override;
    /**
     * @brief save should save values to persistent store
     */
    virtual void save() override;
    
    virtual ~RemoteRegistry() override;
private:
    void connect();
    void disconnect();
};


} // namespace registry
} // namespace cyphalpp

#endif // ndef CYPHALPP_REMOTEREGISTRY_HPP