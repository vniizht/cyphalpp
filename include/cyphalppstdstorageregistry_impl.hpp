//
// Copyright © 2022 JSC "VNIIZHT" or its affiliates. All Rights Reserved.
// This software is distributed under the terms of the MIT License.
//
#ifndef CYPHALPP_STDSTORAGEREGISTRY_IMPL_HPP
#define CYPHALPP_STDSTORAGEREGISTRY_IMPL_HPP
#ifndef CYPHALPP_STDSTORAGEREGISTRY_HPP
#include "cyphalppstdstorageregistry.hpp"
#endif // ndef CYPHALPP_STDSTORAGEREGISTRY_HPP

namespace cyphalpp{
namespace registry{

namespace types{
inline Name N(const std::string& s){ 
    Name ret{};
    ret.name.resize(s.size());
    std::copy_n(s.begin(), s.size(), ret.name.begin());
    return ret;
}
inline String S(const std::string& s){ 
    String ret{};
    ret.value.resize(s.size());
    std::copy_n(s.begin(), s.size(), ret.value.begin());
    return ret;
}
inline Bytes B(const std::string& s){ 
    Bytes ret{};
    ret.value.resize(s.size());
    std::copy_n(s.begin(), s.size(), ret.value.begin());
    return ret;
}
} // namespace types

inline std::string name_view(const Name& n){
    std::string ret{};
    ret.reserve(n.name.size());
    ret.append(
        reinterpret_cast<const char*>(n.name.data()),
        n.name.size()
    );
    // ret.push_back('\0');
    return ret;
}
StdStorageRegistry::StdStorageRegistry(std::string fn): file_name_(std::move(fn)){
}

/**
 * @brief getName implements uavcan.register.List.1.0 commands
 * @param i index
 * @return register name as in uavcan.register.List.1.0
 */
inline List::Response StdStorageRegistry::getName(uint16_t i)  {
    List::Response r{
        .name = Name{}
    };
    if(i < values_.size()){
        r.name = values_.at(i).name;
    }
    return r;
}
/**
 * @brief get gets a value by name with metainformation
 * @param n - name to get
 * @return
 */
inline Access::Response StdStorageRegistry::get(const Name& n) const {
    auto nv = name_view(n);
    auto id = names_.find(nv);
    if(names_.end() == id){
        return Access::Response{};
    }
    return values_.at(id->second).value;
}
/**
 * @brief set sets a new value for a field
 * @param n name of a value to set
 * @param v value to set
 * @return
 */
inline Access::Response StdStorageRegistry::set(const Name& n, const Value& v) {
    struct Changes{
        StdStorageRegistry* self;
        bool autosave;
        bool changed;
        Changes(StdStorageRegistry* self_, bool autosave_)
            :self(self_), autosave(autosave_), changed(false){}
        ~Changes(){
            if(changed and autosave){
                self->save();
            }
        }
    } changes{this, autosave_};
    auto nv = name_view(n);
    auto id = names_.find(nv);
    if(names_.end() == id){
        if(v.is_empty()){ 
            return Access::Response{}; 
        }
        values_.emplace_back(Registry::RegisterType{
                .name = n,
                .value = {
                    .timestamp = {.microsecond=0},
                    ._mutable = true,
                    .persistent = true,
                    .value = v
                }});
        const auto& last = values_.back();
        names_.insert({name_view(last.name), values_.size() - 1 });
        changes.changed = true;
        return last.value;
    }
    auto& ret = values_.at(id->second).value;
    if(not v.is_empty() and ret._mutable){
        ret.value = v;
        changes.changed = true;
    }
    return ret;
}

inline uint16_t StdStorageRegistry::size() const {
    return values_.size();
}

inline void StdStorageRegistry::setAutosave(bool autosave) {
    autosave_ = autosave;
}

inline StdStorageRegistry::~StdStorageRegistry(){
}

} // namespace registry
} // namespace cyphalpp

#endif // ndef CYPHALPP_STDSTORAGEREGISTRY_IMPL_HPP