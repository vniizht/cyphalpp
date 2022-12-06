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

template<std::size_t i>
using type_at = typename Value::VariantType::alternative<i>::type;

template<typename T, typename U, typename... Args>
inline T any_scalar(Args&&... args){
    return {std::initializer_list<U>{std::forward<Args>(args)...}};
}

} // namespace detail


using Empty = detail::type_at<Value::VariantType::IndexOf::empty>;
using String = detail::type_at<Value::VariantType::IndexOf::string>;
using Bytes = detail::type_at<Value::VariantType::IndexOf::unstructured>;
using Bits = detail::type_at<Value::VariantType::IndexOf::bit>;
using Integer64 = detail::type_at<Value::VariantType::IndexOf::integer64>;
using Integer32 = detail::type_at<Value::VariantType::IndexOf::integer32>;
using Integer16 = detail::type_at<Value::VariantType::IndexOf::integer16>;
using Integer8 = detail::type_at<Value::VariantType::IndexOf::integer8>;
using Natural64 = detail::type_at<Value::VariantType::IndexOf::natural64>;
using Natural32 = detail::type_at<Value::VariantType::IndexOf::natural32>;
using Natural16 = detail::type_at<Value::VariantType::IndexOf::natural16>;
using Natural8 = detail::type_at<Value::VariantType::IndexOf::natural8>;
using Real64 = detail::type_at<Value::VariantType::IndexOf::real64>;
using Real32 = detail::type_at<Value::VariantType::IndexOf::real32>;
using Real16 = detail::type_at<Value::VariantType::IndexOf::real16>;

template<size_t Nc>
inline Name N(const char (& name)[Nc]){ 
    return {{static_cast<const char*>(name), static_cast<const char*>(name) + Nc -1}}; }
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

class Registry {
    cyphalpp::CyphalUdp& uc_;
    std::unique_ptr<RegistryImpl> impl_;
    cyphalpp::CyphalUdp::SubscriptionHandle access_;
    cyphalpp::CyphalUdp::SubscriptionHandle list_;
public:
    struct RegisterType {
        Name name;
        Access::Response value;
    };

    Registry(cyphalpp::CyphalUdp& uc, std::unique_ptr<RegistryImpl> impl)
            :uc_(uc), impl_(std::move(impl))
    {
        if(impl_){
            impl_->load();
        }
        access_ = uc_.subscribeServiceRequest<Access>(
        [this](const cyphalpp::TransferMetadata&, const Access::Request& req) -> Access::Response{
            auto r = impl_->get(req.name);
            if(r._mutable and not req.value.is_empty()){
                return impl_->set(req.name, req.value);
            }
            return r;
        }
        );
        list_ = uc_.subscribeServiceRequest<List>(
        [this](const cyphalpp::TransferMetadata&, const List::Request& req) -> List::Response{
            return impl_->getName(req.index);
        }
        );
    }
    ~Registry(){
        uc_.unsubscribe(access_);
        uc_.unsubscribe(list_);
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
        
        template<typename T> T* as();
        template<typename T> const T* as() const;
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
    Register operator[](const char* const v){
        Name n{{v, v + strnlen(v, 255)}};
        return (*this)[n];
    }
    
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

} // namespace registry
} // namespace cyphalpp

#endif // ndef CYPHALPP_REGISTRYL_HPP
