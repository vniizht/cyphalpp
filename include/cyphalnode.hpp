//
// Copyright © 2022 JSC "VNIIZHT" or its affiliates. All Rights Reserved.
// This software is distributed under the terms of the MIT License.
//

#include "cyphalpp.hpp"
#include <chrono>
#include <uavcan/node/Heartbeat_1_0.hpp>
#include <uavcan/_register/List_1_0.hpp>
#include <uavcan/_register/Access_1_0.hpp>

namespace cyphalpp {

template<typename clock_t = std::chrono::steady_clock>
class HeartbeatPublish{
private:
    const typename clock_t::time_point startTime;
    cyphalpp::CyphalUdp& uc_;
    std::unique_ptr<cyphalpp::TimerImpl> tim_;
    uavcan::node::Heartbeat_1_0 hb_;
public:
    HeartbeatPublish(cyphalpp::CyphalUdp& uc, std::unique_ptr<cyphalpp::TimerImpl> tim)
            : startTime(clock_t::now()),
            uc_(uc), tim_(std::move(tim)),
            hb_{
                .uptime=static_cast<uint32_t>(
                    std::chrono::duration_cast<std::chrono::seconds>(clock_t::now() - startTime).count()),
                .health={
                    .value=uavcan::node::Health_1_0::NOMINAL
                },
                .mode={
                    .value=uavcan::node::Mode_1_0::OPERATIONAL
                },
                .vendor_specific_status_code=0
            }
    {
        restart();
    }
    uavcan::node::Heartbeat_1_0& heart(){
        return hb_;
    }
private:
    void restart(){
        tim_->startSingleShot(1000, [this](){
            auto uptime = std::chrono::duration_cast<std::chrono::seconds>(clock_t::now() - startTime);
            hb_.uptime = uptime.count();
            uc_.sendMessage(hb_);
            restart();
        });
    }
};

namespace registry {

using Access    = uavcan::_register::Access::Service_1_0;
using List      = uavcan::_register::List::Service_1_0;
using Name      = uavcan::_register::Name_1_0;
using Value     = uavcan::_register::Value_1_0;

namespace types {

namespace detail {

template<std::size_t i>
using type_at = typename Value::VariantType::alternative<i>::type;

template<typename T, typename U>
T any_scalar(const U& v){
    return {{v}};
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


Bits B(bool v){ return detail::any_scalar<Bits , bool>(v); }
Integer64 I64(int64_t v){ return detail::any_scalar<Integer64, int64_t>(v); }
Integer32 I32(int64_t v){ return detail::any_scalar<Integer32, int32_t>(v); }
Integer16 I16(int64_t v){ return detail::any_scalar<Integer16, int16_t>(v); }
Integer8 I8(int64_t v){ return detail::any_scalar<Integer8, int8_t>(v); }
Natural64 U64(uint64_t v){ return detail::any_scalar<Natural64, uint64_t>(v); }
Natural32 U32(uint32_t v){ return detail::any_scalar<Natural32, uint32_t>(v); }
Natural16 U16(uint16_t v){ return detail::any_scalar<Natural16, uint16_t>(v); }
Natural8 U8(uint8_t v){ return detail::any_scalar<Natural8, uint8_t>(v); }
Real64 F64(double v){ return detail::any_scalar<Real64, double>(v); }
Real32 F32(float v){ return detail::any_scalar<Real32, float>(v); }
Real16 F16(float v){ return detail::any_scalar<Real16, float>(v); }

} // namespace types


/**
 * This is an abstract registry interface. You need to implement it for your environment.
 */
struct RegistryImpl{
    virtual List::Response getName(uint16_t i) = 0;
    virtual Access::Response get(const Name& n) const = 0;
    virtual Access::Response set(const Name& n, const Value& v) = 0;
    virtual uint16_t size() = 0;
    virtual void setAutosave(bool autosave) = 0;
    virtual void save() = 0;
    virtual ~RegistryImpl()=default;
};

class Registry {
    cyphalpp::CyphalUdp& uc_;
    std::unique_ptr<RegistryImpl> impl_;
    cyphalpp::CyphalUdp::SubscriptionHandle access_;
    cyphalpp::CyphalUdp::SubscriptionHandle list_;
public:
    Registry(cyphalpp::CyphalUdp& uc, std::unique_ptr<RegistryImpl> impl)
            :uc_(uc), impl_(std::move(impl))
    {
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
        Register& operator=(const types::Empty& v){ 
            if(mutable_){ val_.set_empty(v); set(); }
            return *this; 
        }
        Register& operator=(const types::String& v){ 
            if(mutable_){ val_.set_string(v); set(); }
            return *this; 
        }
        Register& operator=(const types::Bytes& v){ 
            if(mutable_){ val_.set_unstructured(v); set(); }
            return *this; 
        }
        Register& operator=(const types::Bits& v){ 
            if(mutable_){ val_.set_bit(v); set(); }
            return *this; 
        }
        Register& operator=(const types::Integer64& v){ 
            if(mutable_){ val_.set_integer64(v); set(); }
            return *this; 
        }
        Register& operator=(const types::Integer32& v){ 
            if(mutable_){ val_.set_integer32(v); set(); }
            return *this; 
        }
        Register& operator=(const types::Integer16& v){ 
            if(mutable_){ val_.set_integer16(v); set(); }
            return *this; 
        }
        Register& operator=(const types::Integer8& v){ 
            if(mutable_){ val_.set_integer8(v); set(); }
            return *this; 
        }
        Register& operator=(const types::Natural64& v){ 
            if(mutable_){ val_.set_natural64(v); set(); }
            return *this; 
        }
        Register& operator=(const types::Natural32& v){ 
            if(mutable_){ val_.set_natural32(v); set(); }
            return *this; 
        }
        Register& operator=(const types::Natural16& v){ 
            if(mutable_){ val_.set_natural16(v); set(); }
            return *this; 
        }
        Register& operator=(const types::Natural8& v){ 
            if(mutable_){ val_.set_natural8(v); set(); }
            return *this; 
        }
        Register& operator=(const types::Real64& v){ 
            if(mutable_){ val_.set_real64(v); set(); }
            return *this; 
        }
        Register& operator=(const types::Real32& v){ 
            if(mutable_){ val_.set_real32(v); set(); }
            return *this; 
        }
        Register& operator=(const types::Real16& v){ 
            if(mutable_){ val_.set_real16(v); set(); }
            return *this; 
        }
        void get(){
            auto ret = registry_->impl_->get(name_);
            val_ = ret.value;
            mutable_ = ret._mutable;
            persistent_ = ret.persistent;
        }
    private:
        Register(Registry* reg, const Name& n)
            :registry_(reg)
            , name_(n)
            , val_{}
            , mutable_{false}
            , persistent_{false}
        {
            get();
        }
        void set(){
            auto ret = registry_->impl_->set(name_, val_);
            val_ = ret.value;
            mutable_ = ret._mutable;
            persistent_ = ret.persistent;
        }
    };
    Register operator[](const char* const v){
        Name n{{v, v + strnlen(v, 255)}};
        return (*this)[n];
    }
    
    Register operator[](const Name& n){
        return {this, n};
    }
};

} // namespace registry
} // namespace cyphalpp
