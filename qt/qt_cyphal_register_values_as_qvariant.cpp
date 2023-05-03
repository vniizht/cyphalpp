#include "qt_cyphal_register_values_as_qvariant.hpp"
#include <QVariant>

namespace cyphalpp {
namespace registry {


namespace types {

template<typename U, typename T>
static QVariant implToVariant(size_t v,T val){
    QVariantMap r;
    r.insert("type", static_cast<double>(v));
    {
        QVariantList rl;
        for (typename decltype(T::value)::value_type i: val.value){
            rl.append(QVariant::fromValue<U>(i));
        }
        r.insert("value", std::move(rl));
    }
    return r;
}

static QVariant toVariant(const Empty& ){ return QVariant{};}
static QVariant toVariant(const String& v){
    QVariantMap r;
    r.insert("type", static_cast<double>(Value::VariantType::IndexOf::string));
    r.insert("value", QString::fromUtf8(
        reinterpret_cast<const char*>(v.value.data()),
        v.value.size()));
    return r;
}
static QVariant toVariant(const Bytes& v){ 
    return implToVariant<qulonglong>(Value::VariantType::IndexOf::unstructured, v); 
}
static QVariant toVariant(const Bits& v){ 
    return implToVariant<bool>(Value::VariantType::IndexOf::bit, v); 
}
static QVariant toVariant(const Integer64& v){ 
    return implToVariant<qlonglong>(Value::VariantType::IndexOf::integer64, v);
}
static QVariant toVariant(const Integer32& v){ 
    return implToVariant<qlonglong>(Value::VariantType::IndexOf::integer32, v); 
}
static QVariant toVariant(const Integer16& v){ 
    return implToVariant<qlonglong>(Value::VariantType::IndexOf::integer16, v); 
}
static QVariant toVariant(const Integer8& v){ 
    return implToVariant<qlonglong>(Value::VariantType::IndexOf::integer8, v); 
}
static QVariant toVariant(const Natural64& v){ 
    return implToVariant<qulonglong>(Value::VariantType::IndexOf::natural64, v); 
}
static QVariant toVariant(const Natural32& v){ 
    return implToVariant<qulonglong>(Value::VariantType::IndexOf::natural32, v);
}
static QVariant toVariant(const Natural16& v){ 
    return implToVariant<qulonglong>(Value::VariantType::IndexOf::natural16, v);
}
static QVariant toVariant(const Natural8& v){ 
    return implToVariant<qulonglong>(Value::VariantType::IndexOf::natural8, v);
}
static QVariant toVariant(const Real64& v){ 
    return implToVariant<double>(Value::VariantType::IndexOf::real64, v);
}
static QVariant toVariant(const Real32& v){ 
    return implToVariant<double>(Value::VariantType::IndexOf::real32, v);
}
static QVariant toVariant(const Real16& v){ 
    return implToVariant<double>(Value::VariantType::IndexOf::real16, v);
}



template< typename T, typename F>
static T implFromVariant(F&& f, const QVariant& val){
    using V = typename decltype(T::value)::value_type;
    T ret;
    QVariantList rl = val.toList();
    ret.value.reserve(rl.size());
    for (const auto& i: rl){
        ret.value.emplace_back(static_cast<V>(f(i)));
    }
    return ret;
}

static String stringFromVariant(const QVariant& v){ 
    String r;
    auto baV = v.toString().toUtf8();
    r.value.resize(baV.size());
    std::copy_n(baV.begin(), baV.size(), r.value.begin());
    return r;
}

} // namespace types
 

QVariant toVariant(const Value &v)
{
    if(v.is_empty()){
        return types::toVariant(v.get_empty());
    }
    if(v.is_string()){
        return types::toVariant(v.get_string());
    }
    if(v.is_unstructured()){
        return types::toVariant(v.get_unstructured());
    }
    if(v.is_bit()){
        return types::toVariant(v.get_bit());
    }
    if(v.is_integer64()){
        return types::toVariant(v.get_integer64());
    }
    if(v.is_integer32()){
        return types::toVariant(v.get_integer32());
    }
    if(v.is_integer16()){
        return types::toVariant(v.get_integer16());
    }
    if(v.is_integer8()){
        return types::toVariant(v.get_integer8());
    }
    if(v.is_natural64()){
        return types::toVariant(v.get_natural64());
    }
    if(v.is_natural32()){
        return types::toVariant(v.get_natural32());
    }
    if(v.is_natural16()){
        return types::toVariant(v.get_natural16());
    }
    if(v.is_natural8()){
        return types::toVariant(v.get_natural8());
    }
    if(v.is_real64()){
        return types::toVariant(v.get_real64());
    }
    if(v.is_real32()){
        return types::toVariant(v.get_real32());
    }
    if(v.is_real16()){
        return types::toVariant(v.get_real16());
    }
    return QVariant();
}

Value fromVariant(const QVariant &v)
{
    Value r;
    r.set_empty();
    if(v.type() != QVariant::Map){
        return r;
    }
    auto m = v.toMap();
    bool ok;
    auto t = static_cast<size_t>(m["type"].toDouble(&ok));
    if(not ok){
        return r;
    }
    auto val = m["value"];
    if(t == Value::VariantType::IndexOf::string){
        r.set_string(types::stringFromVariant(val));
        return r;
    }
    if(val.type() != QVariant::List){
        return r;
    }
    switch(t){
    case Value::VariantType::IndexOf::unstructured:
        r.set_unstructured(types::implFromVariant<types::Bytes>(
            [](const QVariant&  v){return v.toULongLong();}, val)); 
        break;
    case Value::VariantType::IndexOf::bit:
        r.set_bit(types::implFromVariant<types::Bits>(
            [](const QVariant&  v){return v.toBool();}, val));
        break;
    case Value::VariantType::IndexOf::integer64:
        r.set_integer64(types::implFromVariant<types::Integer64>(
            [](const QVariant&  v){return v.toLongLong();}, val));
        break;
    case Value::VariantType::IndexOf::integer32:
        r.set_integer32(types::implFromVariant<types::Integer32>(
            [](const QVariant&  v){return v.toLongLong();}, val));
        break;
    case Value::VariantType::IndexOf::integer16:
        r.set_integer16(types::implFromVariant<types::Integer16>(
            [](const QVariant&  v){return v.toLongLong();}, val));
        break;
    case Value::VariantType::IndexOf::integer8:
        r.set_integer8(types::implFromVariant<types::Integer8>(
            [](const QVariant&  v){return v.toLongLong();}, val));
        break;
    case Value::VariantType::IndexOf::natural64:
        r.set_natural64(types::implFromVariant<types::Natural64>(
            [](const QVariant&  v){return v.toULongLong();}, val));
        break;
    case Value::VariantType::IndexOf::natural32:
        r.set_natural32(types::implFromVariant<types::Natural32>(
            [](const QVariant&  v){return v.toULongLong();}, val));
        break;
    case Value::VariantType::IndexOf::natural16:
        r.set_natural16(types::implFromVariant<types::Natural16>(
            [](const QVariant&  v){return v.toULongLong();}, val));
        break;
    case Value::VariantType::IndexOf::natural8:
        r.set_natural8(types::implFromVariant<types::Natural8>(
            [](const QVariant&  v){return v.toULongLong();}, val));
        break;
    case Value::VariantType::IndexOf::real64:
        r.set_real64(types::implFromVariant<types::Real64>(
            [](const QVariant&  v){return v.toDouble();}, val));
        break;
    case Value::VariantType::IndexOf::real32:
        r.set_real32(types::implFromVariant<types::Real32>(
            [](const QVariant&  v){return v.toDouble();}, val));
        break;
    case Value::VariantType::IndexOf::real16:
        r.set_real16(types::implFromVariant<types::Real16>(
            [](const QVariant&  v){return v.toDouble();}, val));
        break;
    default:
        break;
    }
    return r;
}


} // namespace registry
} // namespace cyphalpp
