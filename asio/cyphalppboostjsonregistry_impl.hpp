//
// Copyright © 2022 JSC "VNIIZHT" or its affiliates. All Rights Reserved.
// This software is distributed under the terms of the MIT License.
//
#ifndef CYPHALPP_BOOSTJSONREGISTRY_IMPL_HPP
#define CYPHALPP_BOOSTJSONREGISTRY_IMPL_HPP
#include "cyphalppboostjsonregistry.hpp"

#include <boost/json.hpp>


namespace cyphalpp {
namespace registry {

using namespace boost::json;

namespace types {

template<typename U, typename T>
static value implToJValue(size_t v,T val){
    object r;
    r.emplace("type", static_cast<uint64_t>(v));
    {
        array rl;
        for (typename decltype(T::value)::value_type i: val.value){
            rl.emplace_back(static_cast<U>(i));
        }
        r.emplace("value", std::move(rl));
    }
    return r;
}

static value toJValue(const Empty& ){ return value{};}
static value toJValue(const String& v){
    value r{
        {"type", static_cast<uint64_t>(Value::VariantType::IndexOf::string)},
        {"value", std::string(
            reinterpret_cast<const char*>(v.value.data()),
            v.value.size()
        )}
    };
    return r;
}
static value toJValue(const Bytes& v){ 
    return implToJValue<uint64_t>(Value::VariantType::IndexOf::unstructured, v); 
}
static value toJValue(const Bits& v){ 
    return implToJValue<bool>(Value::VariantType::IndexOf::bit, v); 
}
static value toJValue(const Integer64& v){ 
    return implToJValue<int64_t>(Value::VariantType::IndexOf::integer64, v);
}
static value toJValue(const Integer32& v){ 
    return implToJValue<int64_t>(Value::VariantType::IndexOf::integer32, v); 
}
static value toJValue(const Integer16& v){ 
    return implToJValue<int64_t>(Value::VariantType::IndexOf::integer16, v); 
}
static value toJValue(const Integer8& v){ 
    return implToJValue<int64_t>(Value::VariantType::IndexOf::integer8, v); 
}
static value toJValue(const Natural64& v){ 
    return implToJValue<uint64_t>(Value::VariantType::IndexOf::natural64, v); 
}
static value toJValue(const Natural32& v){ 
    return implToJValue<uint64_t>(Value::VariantType::IndexOf::natural32, v);
}
static value toJValue(const Natural16& v){ 
    return implToJValue<uint64_t>(Value::VariantType::IndexOf::natural16, v);
}
static value toJValue(const Natural8& v){ 
    return implToJValue<uint64_t>(Value::VariantType::IndexOf::natural8, v);
}
static value toJValue(const Real64& v){ 
    return implToJValue<double>(Value::VariantType::IndexOf::real64, v);
}
static value toJValue(const Real32& v){ 
    return implToJValue<double>(Value::VariantType::IndexOf::real32, v);
}
static value toJValue(const Real16& v){ 
    return implToJValue<double>(Value::VariantType::IndexOf::real16, v);
}



template< typename T, typename F>
static T implFromJValue(F&& f, const value& val){
    using V = typename decltype(T::value)::value_type;
    T ret;
    auto& rl = val.get_array();
    ret.value.reserve(rl.size());
    for (const auto& i: rl){
        ret.value.emplace_back(static_cast<V>(f(i)));
    }
    return ret;
}

static String stringFromJValue(const value& v){ 
    String r;
    auto baV = v.as_string();
    r.value.resize(baV.size());
    std::copy_n(baV.begin(), baV.size(), r.value.begin());
    return r;
}

} // namespace types
 

value toJValue(const Value &v)
{
    if(v.is_empty()){
        return types::toJValue(v.get_empty());
    }
    if(v.is_string()){
        return types::toJValue(v.get_string());
    }
    if(v.is_unstructured()){
        return types::toJValue(v.get_unstructured());
    }
    if(v.is_bit()){
        return types::toJValue(v.get_bit());
    }
    if(v.is_integer64()){
        return types::toJValue(v.get_integer64());
    }
    if(v.is_integer32()){
        return types::toJValue(v.get_integer32());
    }
    if(v.is_integer16()){
        return types::toJValue(v.get_integer16());
    }
    if(v.is_integer8()){
        return types::toJValue(v.get_integer8());
    }
    if(v.is_natural64()){
        return types::toJValue(v.get_natural64());
    }
    if(v.is_natural32()){
        return types::toJValue(v.get_natural32());
    }
    if(v.is_natural16()){
        return types::toJValue(v.get_natural16());
    }
    if(v.is_natural8()){
        return types::toJValue(v.get_natural8());
    }
    if(v.is_real64()){
        return types::toJValue(v.get_real64());
    }
    if(v.is_real32()){
        return types::toJValue(v.get_real32());
    }
    if(v.is_real16()){
        return types::toJValue(v.get_real16());
    }
    return value();
}


template<typename T>T fromPtr(T* a){if(a)return *a; else return T{};}

template<typename T>T to_number(const value& a){
    error_code ec;
    auto ret = a.to_number<T>(ec);
    if(ec){
        std::clog << "Failed getting number: " << ec.message() << "\n";
        return T{};
    }
    return ret;
}

const std::unordered_map<std::string_view, size_t>& stringToType(){
    static std::unordered_map<std::string_view, size_t>* vals = []{ 
        return new std::unordered_map<std::string_view, size_t>({
            {"empty", Value::VariantType::IndexOf::empty},
            {"string", Value::VariantType::IndexOf::string},
            {"unstructured", Value::VariantType::IndexOf::unstructured},
            {"bit", Value::VariantType::IndexOf::bit},
            {"integer64", Value::VariantType::IndexOf::integer64},
            {"integer32", Value::VariantType::IndexOf::integer32},
            {"integer16", Value::VariantType::IndexOf::integer16},
            {"integer8", Value::VariantType::IndexOf::integer8},
            {"natural64", Value::VariantType::IndexOf::natural64},
            {"natural32", Value::VariantType::IndexOf::natural32},
            {"natural16", Value::VariantType::IndexOf::natural16},
            {"natural8", Value::VariantType::IndexOf::natural8},
            {"real64", Value::VariantType::IndexOf::real64},
            {"real32", Value::VariantType::IndexOf::real32},
            {"real16", Value::VariantType::IndexOf::real16},
        });
    }();
    return *vals;
}

static size_t typeFromString(const string& s){
    const auto& vals = stringToType();
    auto r = vals.find(std::string_view(s));
    if(vals.end() == r){
        std::clog << "Failed to convert type from string: " << s << '\n';
        return Value::VariantType::IndexOf::empty;
    }
    return r->second;
}

Value fromJValue(const value &v)
{
    Value r;
    r.set_empty();
    if(v.kind() != kind::object){
        std::clog << "Json value should be an object, not " << v.kind() << '\n';
        return r;
    }
    auto& m = v.get_object();
    
    auto t = ({
        auto valIt = m.find("type");
        if(valIt == m.end()){
            std::clog << "Type not found!\n";
            return r;
        };
        size_t t;
        auto tVal = valIt->value();
        if(tVal.is_string()){
            t = typeFromString(tVal.get_string());
        }else if(tVal.is_number()){
            error_code ec;
            t = tVal.to_number<uint64_t>(ec);
            if(ec){
                std::clog << "Failed getting number: " << ec.message() << "\n";
                return r;
            }
        }else{
            std::clog << "Wrong value for type:" << tVal << '\n';
            return r;
        }
        t;
    });
    auto val = ({
        auto valIt = m.find("value"); 
        if(valIt == m.end()){
            std::clog << "Value not found!\n";
            return r;
        }; 
        valIt->value(); 
    });
    if( t == Value::VariantType::IndexOf::string){
        if(val.kind() != kind::string){
            std::clog << val.kind() << "is not a string!\ns";
            return r;
        }
        r.set_string(types::stringFromJValue(val));
        return r;
    }
    if(val.kind() != kind::array){
        return r;
    }
    switch(t){
    case Value::VariantType::IndexOf::unstructured:
        r.set_unstructured(types::implFromJValue<types::Bytes>(
            [](const value&  v){return to_number<uint8_t>(v);}, val)); 
        break;
    case Value::VariantType::IndexOf::bit:
        r.set_bit(types::implFromJValue<types::Bits>(
            [](const value&  v){return fromPtr(v.if_bool());}, val));
        break;
    case Value::VariantType::IndexOf::integer64:
        r.set_integer64(types::implFromJValue<types::Integer64>(
            [](const value&  v){return to_number<int64_t>(v);}, val));
        break;
    case Value::VariantType::IndexOf::integer32:
        r.set_integer32(types::implFromJValue<types::Integer32>(
            [](const value&  v){return to_number<int32_t>(v);}, val));
        break;
    case Value::VariantType::IndexOf::integer16:
        r.set_integer16(types::implFromJValue<types::Integer16>(
            [](const value&  v){return to_number<int16_t>(v);}, val));
        break;
    case Value::VariantType::IndexOf::integer8:
        r.set_integer8(types::implFromJValue<types::Integer8>(
            [](const value&  v){return to_number<int8_t>(v);}, val));
        break;
    case Value::VariantType::IndexOf::natural64:
        r.set_natural64(types::implFromJValue<types::Natural64>(
            [](const value&  v){return to_number<uint64_t>(v);}, val));
        break;
    case Value::VariantType::IndexOf::natural32:
        r.set_natural32(types::implFromJValue<types::Natural32>(
            [](const value&  v){return to_number<uint32_t>(v);}, val));
        break;
    case Value::VariantType::IndexOf::natural16:
        r.set_natural16(types::implFromJValue<types::Natural16>(
            [](const value&  v){return to_number<uint16_t>(v);}, val));
        break;
    case Value::VariantType::IndexOf::natural8:
        r.set_natural8(types::implFromJValue<types::Natural8>(
            [](const value&  v){return to_number<uint8_t>(v);}, val));
        break;
    case Value::VariantType::IndexOf::real64:
        r.set_real64(types::implFromJValue<types::Real64>(
            [](const value&  v){return to_number<double>(v);}, val));
        break;
    case Value::VariantType::IndexOf::real32:
        r.set_real32(types::implFromJValue<types::Real32>(
            [](const value&  v){return to_number<float>(v);}, val));
        break;
    case Value::VariantType::IndexOf::real16:
        r.set_real16(types::implFromJValue<types::Real16>(
            [](const value&  v){return to_number<float>(v);}, val));
        break;
    default:
        std::clog << "Wrong type id: " <<  t  << '\n';
        break;
    }
    return r;
}


void BoostJsonRegistry::load(){
    auto js = ({
        std::ifstream f(file_name_, std::ios::binary | std::ios::in);
        if(not f.is_open()){
            std::clog << strerror(errno) << '\n';
            return;
        }
        error_code ec;
        stream_parser p;
        std::string line;
        while( std::getline( f, line ) )
        {
            p.write( line, ec );
            if( ec )
                return;
        }
        p.finish( ec );
        if( ec ){
            std::clog << "Error loading registry" << ec.message() << '\n';
            return;
        }
        p.release();
    });
    if(js.kind() != kind::object){
        std::clog << "Wrong types of object" << js.kind() << js << '\n';
        return;
    }

    for(auto kv: js.get_object()){
        set(types::N(kv.key()), fromJValue(kv.value()));
    }
}

void BoostJsonRegistry::save(){
    std::ofstream f(file_name_, std::ios::binary | std::ios::out);
    object info;
    for(auto& reg: values_){
        info.emplace(name_view(reg.name), toJValue(reg.value.value));
    }

    serializer sr;
    sr.reset( &info );
    while( ! sr.done() )
    {
        char buf[ BOOST_JSON_STACK_BUFFER_SIZE ];
        f << sr.read( buf );
    }
}

BoostJsonRegistry::~BoostJsonRegistry(){
    
}

} // namespace registry
} // namespace cyphalpp

#include <boost/json/src.hpp>
#endif // ndef CYPHALPP_BOOSTJSONREGISTRY_IMPL_HPP