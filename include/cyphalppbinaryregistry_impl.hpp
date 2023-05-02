//
// Copyright © 2022 JSC "VNIIZHT" or its affiliates. All Rights Reserved.
// This software is distributed under the terms of the MIT License.
//
#ifndef CYPHALPP_BINARYREGISTRY_IMPL_HPP
#define CYPHALPP_BINARYREGISTRY_IMPL_HPP
#include "cyphalppstdstorageregistry_impl.hpp"
#ifndef CYPHALPP_BINARYREGISTRY_HPP
#include "cyphalppbinaryregistry.hpp"
#endif // ndef CYPHALPP_BINARYREGISTRY_HPP

namespace cyphalpp{
namespace registry{

void BinaryRegistry::load() {
    std::ifstream f(file_name_, std::ios::binary | std::ios::in);
    if(not f.is_open()) return;
    constexpr static size_t BUFFER_SIZE = std::max(Name::EXTENT_BYTES, Value::EXTENT_BYTES);
    uint8_t buffer[BUFFER_SIZE];
    f.read(reinterpret_cast<char*>(buffer), 2);
    size_t read_size = nunavut::support::const_bitspan(buffer, BUFFER_SIZE, 0).getU16(16);
    values_.reserve(read_size);
    names_.reserve(read_size);  

    for(size_t v =0; v<read_size; v++){
        Name name; Value value;
        f.read(reinterpret_cast<char*>(buffer), Name::EXTENT_BYTES);
        name.deserialize(nunavut::support::const_bitspan(buffer, BUFFER_SIZE, 0));
        f.read(reinterpret_cast<char*>(buffer), Value::EXTENT_BYTES);
        value.deserialize(nunavut::support::const_bitspan(buffer, BUFFER_SIZE, 0));
        set(name, value);
    }
}

void BinaryRegistry::save(){
    std::ofstream f(file_name_, std::ios::binary | std::ios::out);
    constexpr static size_t BUFFER_SIZE = std::max(Name::EXTENT_BYTES, Value::EXTENT_BYTES);
    uint8_t buffer[BUFFER_SIZE];
    nunavut::support::bitspan(buffer, BUFFER_SIZE, 0).setUxx(values_.size(), 16);
    f.write(reinterpret_cast<const char*>(buffer), 2);
    for(auto& reg: values_){
        std::fill_n(buffer, Name::EXTENT_BYTES, '\0');
        reg.name.serialize(nunavut::support::bitspan(buffer, BUFFER_SIZE, 0));
        f.write(reinterpret_cast<const char*>(buffer), Name::EXTENT_BYTES);
        std::fill_n(buffer, Value::EXTENT_BYTES, '\0');
        reg.value.value.serialize(nunavut::support::bitspan(buffer, BUFFER_SIZE, 0));
        f.write(reinterpret_cast<const char*>(buffer), Value::EXTENT_BYTES);
    }
}

inline BinaryRegistry::~BinaryRegistry(){
}


template<>
inline std::string Registry::Register::as_value(std::string defaultValue) const {
    const auto ptr = as<types::String>();
    if(not ptr){ return defaultValue;}
    return std::string(
        reinterpret_cast<const char*>(ptr->value.data()),
        ptr->value.size()
    );
}

} // namespace registry
} // namespace cyphalpp

#endif // ndef CYPHALPP_BINARYREGISTRY_IMPL_HPP