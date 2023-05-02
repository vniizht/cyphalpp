//
// Copyright © 2022 JSC "VNIIZHT" or its affiliates. All Rights Reserved.
// This software is distributed under the terms of the MIT License.
//
#ifndef CYPHALPP_REGISTRYL_HPP
#define CYPHALPP_REGISTRYL_HPP

#include "cyphalpp.hpp"
#include <uavcan/_register/List_1_0.hpp>
#include <uavcan/_register/Access_1_0.hpp>

namespace cyphalpp {

namespace registry {

using Access    = uavcan::_register::Access::Service_1_0;
using List      = uavcan::_register::List::Service_1_0;
using Name      = uavcan::_register::Name_1_0;
using Value     = uavcan::_register::Value_1_0;

namespace types {

namespace detail {

template<typename T1, typename T2, typename... Error>
struct are_all_same : std::false_type {};

template<typename T, typename... Checking>
struct are_all_same<T, T, Checking...> : are_all_same<T, Checking...> {}; 

template<typename T>
struct are_all_same<T,T> : std::true_type {};

template<typename T, typename U, typename... Args>
inline T any_scalar(Args&&... args){
    return {std::initializer_list<U>{std::forward<Args>(args)...}};
}

} // namespace detail

using Empty = uavcan ::primitive::Empty_1_0;
using String = uavcan::primitive::String_1_0;
using Bytes = uavcan::primitive::Unstructured_1_0;
using Bits = uavcan::primitive::array::Bit_1_0;
using Integer64 = uavcan::primitive::array::Integer64_1_0;
using Integer32 = uavcan::primitive::array::Integer32_1_0;
using Integer16 = uavcan::primitive::array::Integer16_1_0;
using Integer8 = uavcan::primitive::array::Integer8_1_0;
using Natural64 = uavcan::primitive::array::Natural64_1_0;
using Natural32 = uavcan::primitive::array::Natural32_1_0;
using Natural16 = uavcan::primitive::array::Natural16_1_0;
using Natural8 = uavcan::primitive::array::Natural8_1_0;
using Real64 = uavcan::primitive::array::Real64_1_0;
using Real32 = uavcan::primitive::array::Real32_1_0;
using Real16 = uavcan::primitive::array::Real16_1_0;

template<size_t Nc>
inline Name N(const char (& name)[Nc]){
    static_assert(Nc > 0, "Name should not be empty");
    Name ret{};
    ret.name.resize(Nc - 1);
    std::copy_n(name, Nc-1, ret.name.begin());
    return ret;
}
template<size_t Nc>
inline String S(const char (& name)[Nc]){
    static_assert(Nc > 0, "String should not be empty");
    String ret{};
    ret.value.resize(Nc - 1);
    std::copy_n(name, Nc-1, ret.value.begin());
    return ret;
}
inline Bits B(bool v){ return detail::any_scalar<Bits , bool>(v); }
inline Integer64 I64(int64_t v){ return detail::any_scalar<Integer64, int64_t>(v); }
inline Integer32 I32(int32_t v){ return detail::any_scalar<Integer32, int32_t>(v); }
inline Integer16 I16(int16_t v){ return detail::any_scalar<Integer16, int16_t>(v); }
inline Integer8 I8(int8_t v){ return detail::any_scalar<Integer8, int8_t>(v); }
inline Natural64 U64(uint64_t v){ return detail::any_scalar<Natural64, uint64_t>(v); }
inline Natural32 U32(uint32_t v){ return detail::any_scalar<Natural32, uint32_t>(v); }
inline Natural16 U16(uint16_t v){ return detail::any_scalar<Natural16, uint16_t>(v); }
inline Natural8 U8(uint8_t v){ return detail::any_scalar<Natural8, uint8_t>(v); }
inline Real64 F64(double v){ return detail::any_scalar<Real64, double>(v); }
inline Real32 F32(float v){ return detail::any_scalar<Real32, float>(v); }
inline Real16 F16(float v){ return detail::any_scalar<Real16, float>(v); }

} // namespace types


/**
 * This is an abstract registry interface. You need to implement it for your environment.
 */
struct RegistryImpl{
    /**
     * @brief getName implements uavcan.register.List.1.0 commands
     * @param i index
     * @return register name as in uavcan.register.List.1.0
     */
    virtual List::Response getName(uint16_t i) = 0;
    /**
     * @brief get gets a value by name with metainformation
     * @param n - name to get
     * @return 
     */
    virtual Access::Response get(const Name& n) const = 0;
    /**
     * @brief set sets a new value for a field
     * @param n name of a value to set
     * @param v value to set
     * @return 
     */
    virtual Access::Response set(const Name& n, const Value& v) = 0;
    /**
     * @brief size
     * @return number of values in registry
     */
    virtual uint16_t size() const = 0;
    virtual void setAutosave(bool autosave) = 0;
    /**
     * @brief load should load values from persistent store
     */
    virtual void load() = 0;
    /**
     * @brief save should save values to persistent store
     */
    virtual void save() = 0;
    virtual ~RegistryImpl()=default;
};

class Registry final {
    struct CyphalState{
        cyphalpp::CyphalUdp* uc_;
        cyphalpp::CyphalUdp::SubscriptionHandle access_;
        cyphalpp::CyphalUdp::SubscriptionHandle list_;
        CyphalState(Registry* self, cyphalpp::CyphalUdp& v):uc_(&v){
            access_ = uc_->subscribeServiceRequest<Access>(
            [self](const cyphalpp::TransferMetadata&, const Access::Request& req) -> Access::Response{
                auto r = self->impl_->get(req.name);
                if(r._mutable and not req.value.is_empty()){
                    return self->impl_->set(req.name, req.value);
                }
                return r;
            }
            );
            list_ = uc_->subscribeServiceRequest<List>(
            [self](const cyphalpp::TransferMetadata&, const List::Request& req) -> List::Response{
                return self->impl_->getName(req.index);
            }
            );
        }
        ~CyphalState(){
            uc_->unsubscribe(access_);
            uc_->unsubscribe(list_);
        }
    };
    friend struct Registry::CyphalState;
    std::unique_ptr<RegistryImpl> impl_;
    std::unique_ptr<CyphalState> state_;
public:
    struct RegisterType {
        Name name;
        Access::Response value;
    };

    Registry(std::unique_ptr<RegistryImpl> impl):impl_(nullptr)
    {
        setImplementation(std::move(impl));
    }
    ~Registry(){

    }

    class NodeResetter{
        Registry& r;
    public:
        explicit NodeResetter(Registry& registry):r(registry){}
        ~NodeResetter(){ r.removeNode(); }
    };

    NodeResetter setNode(cyphalpp::CyphalUdp& v){
        state_ = nullptr;
        state_ = std::make_unique<CyphalState>(this, v);
        return NodeResetter{*this};
    }
    void removeNode(){
        state_ = nullptr;
    }
    void setImplementation(std::unique_ptr<RegistryImpl> impl){
        if(impl_){
            impl_->save();
        }
        impl_ = std::move(impl);
        if(impl_){
            impl_->load();
        }
    }
    friend class Register;
    class Register{
        friend class Registry;
        Registry* registry_;
        Name name_;
        Value val_;
        bool mutable_;
        bool persistent_;
    public:
        template<typename T>
        Register& operator=(const T& v);

        void update(){
            auto ret = registry_->impl_->get(name_);
            val_ = ret.value;
            mutable_ = ret._mutable;
            persistent_ = ret.persistent;
        }

        Value& value() & { return val_; }
        const Value& value() const &{ return val_; }
        Value value() && { return val_; }
        const Value value() const &&{ return val_; }

        template<typename T>
        T* as();
        template<typename T>
        const T* as() const;
        template<typename T>
        T as_value(T defaultValue = {}) const;
    private:
        Register(Registry* reg, const Name& n)
            :registry_(reg)
            , name_(n)
            , val_{}
            , mutable_{false}
            , persistent_{false}
        {
            update();
        }
        void set(){
            auto ret = registry_->impl_->set(name_, val_);
            val_ = ret.value;
            mutable_ = ret._mutable;
            persistent_ = ret.persistent;
        }
        bool canEdit() const{
            return mutable_ or val_.is_empty();
        }

    };
    template<typename T>
    Register setDefault(const Name& name, T&& defaultValue){
        auto r = (*this)[name];
        if(r.val_.is_empty()){
            r.mutable_ = true;
            r.persistent_ = true;
            r = defaultValue;
        }
        return r;
    }

    template<size_t N>
    Register getValue(const char (& name)[N]) { return (*this)[name]; }
    // Register getValue(const char* const v) { return (*this)[v]; }
    Register getValue(const Name& n) { return (*this)[n]; }

    // Register operator[](const char* const v){
    //     Name n{{v, v + strnlen(v, 255)}};
    //     return (*this)[n];
    // }
    Register operator[](const Name& n){
        return {this, n};
    }
    template<size_t N>
    Register operator[](const char (& name)[N]){ 
        return (*this)[types::N(name)]; 
    }
    void save(){
        impl_->save();
    }
};

template<>
inline Registry::Register& Registry::Register::operator=(const types::Empty& v){ 
    if(canEdit()){ val_.set_empty(v); set(); }
    return *this; 
}
template<>
inline Registry::Register& Registry::Register::operator=(const types::String& v){ 
    if(canEdit()){ val_.set_string(v); set(); }
    return *this; 
}
template<>
inline Registry::Register& Registry::Register::operator=(const types::Bytes& v){ 
    if(canEdit()){ val_.set_unstructured(v); set(); }
    return *this; 
}
template<>
inline Registry::Register& Registry::Register::operator=(const types::Bits& v){ 
    if(canEdit()){ val_.set_bit(v); set(); }
    return *this; 
}
template<>
inline Registry::Register& Registry::Register::operator=(const types::Integer64& v){ 
    if(canEdit()){ val_.set_integer64(v); set(); }
    return *this; 
}
template<>
inline Registry::Register& Registry::Register::operator=(const types::Integer32& v){ 
    if(canEdit()){ val_.set_integer32(v); set(); }
    return *this; 
}
template<>
inline Registry::Register& Registry::Register::operator=(const types::Integer16& v){ 
    if(canEdit()){ val_.set_integer16(v); set(); }
    return *this; 
}
template<>
inline Registry::Register& Registry::Register::operator=(const types::Integer8& v){ 
    if(canEdit()){ val_.set_integer8(v); set(); }
    return *this; 
}
template<>
inline Registry::Register& Registry::Register::operator=(const types::Natural64& v){ 
    if(canEdit()){ val_.set_natural64(v); set(); }
    return *this; 
}
template<>
inline Registry::Register& Registry::Register::operator=(const types::Natural32& v){ 
    if(canEdit()){ val_.set_natural32(v); set(); }
    return *this; 
}
template<>
inline Registry::Register& Registry::Register::operator=(const types::Natural16& v){ 
    if(canEdit()){ val_.set_natural16(v); set(); }
    return *this; 
}
template<>
inline Registry::Register& Registry::Register::operator=(const types::Natural8& v){ 
    if(canEdit()){ val_.set_natural8(v); set(); }
    return *this; 
}
template<>
inline Registry::Register& Registry::Register::operator=(const types::Real64& v){ 
    if(canEdit()){ val_.set_real64(v); set(); }
    return *this; 
}
template<>
inline Registry::Register& Registry::Register::operator=(const types::Real32& v){ 
    if(canEdit()){ val_.set_real32(v); set(); }
    return *this; 
}
template<>
inline Registry::Register& Registry::Register::operator=(const types::Real16& v){ 
    if(canEdit()){ val_.set_real16(v); set(); }
    return *this; 
}


// =====================================================================================


template<>
inline types::String* Registry::Register::as(){ 
    if(not (canEdit() and val_.is_string())){ return nullptr; }
    return val_.get_string_if();
}

template<>
inline types::Bytes* Registry::Register::as(){ 
    if(not (canEdit() and val_.is_unstructured())){ return nullptr; }
    return val_.get_unstructured_if();
}

template<>
inline types::Bits* Registry::Register::as(){ 
    if(not (canEdit() and val_.is_bit())){ return nullptr; }
    return val_.get_bit_if();
}

template<>
inline types::Integer64* Registry::Register::as(){ 
    if(not (canEdit() and val_.is_integer64())){ return nullptr; }
    return val_.get_integer64_if();
}

template<>
inline types::Integer32* Registry::Register::as(){ 
    if(not (canEdit() and val_.is_integer32())){ return nullptr; }
    return val_.get_integer32_if();
}

template<>
inline types::Integer16* Registry::Register::as(){ 
    if(not (canEdit() and val_.is_integer16())){ return nullptr; }
    return val_.get_integer16_if();
}

template<>
inline types::Integer8* Registry::Register::as(){ 
    if(not (canEdit() and val_.is_integer8())){ return nullptr; }
    return val_.get_integer8_if();
}

template<>
inline types::Natural64* Registry::Register::as(){ 
    if(not (canEdit() and val_.is_natural64())){ return nullptr; }
    return val_.get_natural64_if();
}

template<>
inline types::Natural32* Registry::Register::as(){ 
    if(not (canEdit() and val_.is_natural32())){ return nullptr; }
    return val_.get_natural32_if();
}

template<>
inline types::Natural16* Registry::Register::as(){ 
    if(not (canEdit() and val_.is_natural16())){ return nullptr; }
    return val_.get_natural16_if();
}

template<>
inline types::Natural8* Registry::Register::as(){ 
    if(not (canEdit() and val_.is_natural8())){ return nullptr; }
    return val_.get_natural8_if();
}

template<>
inline types::Real64* Registry::Register::as(){ 
    if(not (canEdit() and val_.is_real64())){ return nullptr; }
    return val_.get_real64_if();
}

template<>
inline types::Real32* Registry::Register::as(){ 
    if(not (canEdit() and val_.is_real32())){ return nullptr; }
    return val_.get_real32_if();
}

template<>
inline types::Real16* Registry::Register::as(){ 
    if(not (canEdit() and val_.is_real16())){ return nullptr; }
    return val_.get_real16_if();
}


// =====================================================================================


template<>
inline const types::String* Registry::Register::as() const { 
    if(not (canEdit() and val_.is_string())){ return nullptr; }
    return val_.get_string_if();
}
template<>
inline const types::Bytes* Registry::Register::as() const { 
    if(not (canEdit() and val_.is_unstructured())){ return nullptr; }
    return val_.get_unstructured_if();
}
template<>
inline const types::Bits* Registry::Register::as() const { 
    if(not (canEdit() and val_.is_bit())){ return nullptr; }
    return val_.get_bit_if();
}
template<>
inline const types::Integer64* Registry::Register::as() const { 
    if(not (canEdit() and val_.is_integer64())){ return nullptr; }
    return val_.get_integer64_if();
}
template<>
inline const types::Integer32* Registry::Register::as() const { 
    if(not (canEdit() and val_.is_integer32())){ return nullptr; }
    return val_.get_integer32_if();
}
template<>
inline const types::Integer16* Registry::Register::as() const { 
    if(not (canEdit() and val_.is_integer16())){ return nullptr; }
    return val_.get_integer16_if();
}
template<>
inline const types::Integer8* Registry::Register::as() const { 
    if(not (canEdit() and val_.is_integer8())){ return nullptr; }
    return val_.get_integer8_if();
}
template<>
inline const types::Natural64* Registry::Register::as() const { 
    if(not (canEdit() and val_.is_natural64())){ return nullptr; }
    return val_.get_natural64_if();
}
template<>
inline const types::Natural32* Registry::Register::as() const { 
    if(not (canEdit() and val_.is_natural32())){ return nullptr; }
    return val_.get_natural32_if();
}
template<>
inline const types::Natural16* Registry::Register::as() const { 
    if(not (canEdit() and val_.is_natural16())){ return nullptr; }
    return val_.get_natural16_if();
}
template<>
inline const types::Natural8* Registry::Register::as() const { 
    if(not (canEdit() and val_.is_natural8())){ return nullptr; }
    return val_.get_natural8_if();
}
template<>
inline const types::Real64* Registry::Register::as() const { 
    if(not (canEdit() and val_.is_real64())){ return nullptr; }
    return val_.get_real64_if();
}
template<>
inline const types::Real32* Registry::Register::as() const { 
    if(not (canEdit() and val_.is_real32())){ return nullptr; }
    return val_.get_real32_if();
}
template<>
inline const types::Real16* Registry::Register::as() const { 
    if(not (canEdit() and val_.is_real16())){ return nullptr; }
    return val_.get_real16_if();
}


// =====================================================================================


// template<>
// inline const types::Bytes* Registry::Register::as() const { 
//     if(not (canEdit() and val_.is_unstructured())){ return nullptr; }
//     return val_.get_unstructured_if();
// }
// template<>
// inline const types::Bits* Registry::Register::as() const { 
//     if(not (canEdit() and val_.is_bit())){ return nullptr; }
//     return val_.get_bit_if();
// }
template<>
inline const int64_t* Registry::Register::as() const { 
    auto ptr = as<types::Integer64>();
    if(not ptr) { return nullptr; }
    if(ptr->value.empty()) { return nullptr; }
    return ptr->value.data();
}
template<>
inline const int32_t* Registry::Register::as() const { 
    auto ptr = as<types::Integer32>();
    if(not ptr) { return nullptr; }
    if(ptr->value.empty()) { return nullptr; }
    return ptr->value.data();
}
template<>
inline const int16_t* Registry::Register::as() const { 
    auto ptr = as<types::Integer16>();
    if(not ptr) { return nullptr; }
    if(ptr->value.empty()) { return nullptr; }
    return ptr->value.data();
}
template<>
inline const int8_t* Registry::Register::as() const { 
    auto ptr = as<types::Integer8>();
    if(not ptr) { return nullptr; }
    if(ptr->value.empty()) { return nullptr; }
    return ptr->value.data();
}
template<>
inline const uint64_t* Registry::Register::as() const { 
    auto ptr = as<types::Natural64>();
    if(not ptr) { return nullptr; }
    if(ptr->value.empty()) { return nullptr; }
    return ptr->value.data();
}
template<>
inline const uint32_t* Registry::Register::as() const { 
    auto ptr = as<types::Natural32>();
    if(not ptr) { return nullptr; }
    if(ptr->value.empty()) { return nullptr; }
    return ptr->value.data();
}
template<>
inline const uint16_t* Registry::Register::as() const { 
    auto ptr = as<types::Natural16>();
    if(not ptr) { return nullptr; }
    if(ptr->value.empty()) { return nullptr; }
    return ptr->value.data();
}
template<>
inline const uint8_t* Registry::Register::as() const { 
    auto ptr = as<types::Natural8>();
    if(not ptr) { return nullptr; }
    if(ptr->value.empty()) { return nullptr; }
    return ptr->value.data();
}
template<>
inline const double* Registry::Register::as() const { 
    auto ptr = as<types::Real64>();
    if(not ptr) { return nullptr; }
    if(ptr->value.empty()) { return nullptr; }
    return ptr->value.data();
}
template<>
inline const float* Registry::Register::as() const { 
    auto ptr = as<types::Real32>();
    if(not ptr) { return nullptr; }
    if(ptr->value.empty()) { return nullptr; }
    return ptr->value.data();
}
// =====================================================================================

template<>
inline bool Registry::Register::as_value(bool defaultValue) const {
    auto ptr = as<types::Bits>();
    if(not ptr) return defaultValue;
    if(ptr->value.empty()) return defaultValue;
    return ptr->value.front();
}

template<>
inline int64_t Registry::Register::as_value(int64_t defaultValue) const {
    auto ptr = as<int64_t>();
    if(not ptr) return defaultValue;
    return *ptr;
}
template<>
inline int32_t Registry::Register::as_value(int32_t defaultValue) const {
    auto ptr = as<int32_t>();
    if(not ptr) return defaultValue;
    return *ptr;
}
template<>
inline int16_t Registry::Register::as_value(int16_t defaultValue) const {
    auto ptr = as<int16_t>();
    if(not ptr) return defaultValue;
    return *ptr;
}
template<>
inline int8_t Registry::Register::as_value(int8_t defaultValue) const {
    auto ptr = as<int8_t>();
    if(not ptr) return defaultValue;
    return *ptr;
}
template<>
inline uint64_t Registry::Register::as_value(uint64_t defaultValue) const {
    auto ptr = as<uint64_t>();
    if(not ptr) return defaultValue;
    return *ptr;
}
template<>
inline uint32_t Registry::Register::as_value(uint32_t defaultValue) const {
    auto ptr = as<uint32_t>();
    if(not ptr) return defaultValue;
    return *ptr;
}
template<>
inline uint16_t Registry::Register::as_value(uint16_t defaultValue) const {
    auto ptr = as<uint16_t>();
    if(not ptr) return defaultValue;
    return *ptr;
}
template<>
inline uint8_t Registry::Register::as_value(uint8_t defaultValue) const {
    auto ptr = as<uint8_t>();
    if(not ptr) return defaultValue;
    return *ptr;
}
template<>
inline double Registry::Register::as_value(double defaultValue) const {
    auto ptr = as<double>();
    if(not ptr) return defaultValue;
    return *ptr;
}
template<>
inline float Registry::Register::as_value(float defaultValue) const {
    auto ptr = as<float>();
    if(not ptr) return defaultValue;
    return *ptr;
}

} // namespace registry
} // namespace cyphalpp

#endif // ndef CYPHALPP_REGISTRYL_HPP
