//
// Copyright © 2022 JSC "VNIIZHT" or its affiliates. All Rights Reserved.
// This software is distributed under the terms of the MIT License.
//
#ifndef CYPHALPP_BOOSTJSONREGISTRY_HPP
#define CYPHALPP_BOOSTJSONREGISTRY_HPP
#include "cyphalppstdstorageregistry.hpp"

namespace cyphalpp {
namespace registry {

class BoostJsonRegistry: public StdStorageRegistry {
public:
    using StdStorageRegistry::StdStorageRegistry;
    virtual void load() override;
    virtual void save() override;
    virtual ~BoostJsonRegistry() override;
};


} // namespace registry
} // namespace cyphalpp

#endif // ndef CYPHALPP_BOOSTJSONREGISTRY_HPP