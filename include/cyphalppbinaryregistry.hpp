//
// Copyright © 2022 JSC "VNIIZHT" or its affiliates. All Rights Reserved.
// This software is distributed under the terms of the MIT License.
//
#ifndef CYPHALPP_REGISTRYIMPL_HPP
#define CYPHALPP_REGISTRYIMPL_HPP
#include "cyphalppregistry.hpp"
#include <vector>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <string_view>

namespace cyphalpp{
namespace registry{

static std::string name_view(const Name& n){
    return std::string(
        reinterpret_cast<const char*>(n.name.data()),
        n.name.size()
    );
}

class BinaryRegistry : public cyphalpp::registry::RegistryImpl{
    std::vector<Registry::RegisterType> values_;
    std::unordered_map<std::string, size_t> values_;
};

} // namespace registry
} // namespace cyphalpp


#endif // ndef CYPHALPP_REGISTRYIMPL_HPP
