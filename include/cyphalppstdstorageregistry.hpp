//
// Copyright © 2022 JSC "VNIIZHT" or its affiliates. All Rights Reserved.
// This software is distributed under the terms of the MIT License.
//
#ifndef CYPHALPP_STDSTORAGEREGISTRY_HPP
#define CYPHALPP_STDSTORAGEREGISTRY_HPP
#include "cyphalppregistry.hpp"
#include <vector>
#include <unordered_map>
#include <string_view>

namespace cyphalpp{
namespace registry{

namespace types{
String S(const std::string& s);
Bytes B(const std::string& s);
} // namespace types

static std::string name_view(const Name& n);

class StdStorageRegistry : public cyphalpp::registry::RegistryImpl{
protected:
    std::string file_name_;
    std::vector<Registry::RegisterType> values_{};
    std::unordered_map<std::string, size_t> names_{};
private:
    bool autosave_{false};
public:
    StdStorageRegistry(std::string fn);

    /**
     * @brief getName implements uavcan.register.List.1.0 commands
     * @param i index
     * @return register name as in uavcan.register.List.1.0
     */
    virtual List::Response getName(uint16_t i) override ;
    /**
     * @brief get gets a value by name with metainformation
     * @param n - name to get
     * @return
     */
    virtual Access::Response get(const Name& n) const override;
    /**
     * @brief set sets a new value for a field
     * @param n name of a value to set
     * @param v value to set
     * @return
     */
    virtual Access::Response set(const Name& n, const Value& v) override;
    /**
     * @brief size
     * @return number of values in registry
     */
    virtual uint16_t size() const override;
    virtual void setAutosave(bool autosave) override;
    
    virtual ~StdStorageRegistry() override;
};


} // namespace registry
} // namespace cyphalpp

#endif // ndef CYPHALPP_STDSTORAGEREGISTRY_HPP