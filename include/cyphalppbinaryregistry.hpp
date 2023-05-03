//
// Copyright © 2022 JSC "VNIIZHT" or its affiliates. All Rights Reserved.
// This software is distributed under the terms of the MIT License.
//
#ifndef CYPHALPP_BINARYREGISTRY_HPP
#define CYPHALPP_BINARYREGISTRY_HPP
#include "cyphalppstdstorageregistry.hpp"
#include <fstream>
namespace cyphalpp{
namespace registry{

class BinaryRegistry : public StdStorageRegistry {
public:
    using StdStorageRegistry::StdStorageRegistry;
    /**
     * @brief load should load values from persistent store
     */
    virtual void load() override;
    /**
     * @brief save should save values to persistent store
     */
    virtual void save() override;
    virtual ~BinaryRegistry() override;
};

template<>
inline std::string Registry::Register::as_value(std::string defaultValue) const;


} // namespace registry
} // namespace cyphalpp


#endif // ndef CYPHALPP_BINARYREGISTRY_HPP
