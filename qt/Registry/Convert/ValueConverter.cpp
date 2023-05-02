#include "ValueConverter.h"
#include <cyphalppregistry.hpp>
#include <QHostAddress>
#include <QByteArray>
#include <QBitArray>
#include <algorithm>
#include <QString>


namespace cyphalpp::registry::v2_0 {
template<>
std::optional<bool> ValueConverter::from_value(const Value& value) const {
    auto value_pointer = value.get_bit_if();
    if(value_pointer == nullptr) return {};
    if(value_pointer->value.size() != 1) return {};
    return value_pointer->value.front();
}
template<>
std::optional<std::uint8_t> ValueConverter::from_value(const Value& value) const {
    auto value_pointer = value.get_natural8_if();
    if(value_pointer == nullptr) return {};
    if(value_pointer->value.size() != 1) return {};
    return value_pointer->value.front();
}
template<>
std::optional<std::uint16_t> ValueConverter::from_value(const Value& value) const {
    auto value_pointer = value.get_natural16_if();
    if(value_pointer == nullptr) return {};
    if(value_pointer->value.size() != 1) return {};
    return value_pointer->value.front();
}
template<>
std::optional<std::uint32_t> ValueConverter::from_value(const Value& value) const {
    auto value_pointer = value.get_natural32_if();
    if(value_pointer == nullptr) return {};
    if(value_pointer->value.size() != 1) return {};
    return value_pointer->value.front();
}
template<>
std::optional<std::uint64_t> ValueConverter::from_value(const Value& value) const {
    auto value_pointer = value.get_natural64_if();
    if(value_pointer == nullptr) return {};
    if(value_pointer->value.size() != 1) return {};
    return value_pointer->value.front();
}

template<>
std::optional<std::int8_t> ValueConverter::from_value(const Value& value) const {
    auto value_pointer = value.get_integer8_if();
    if(value_pointer == nullptr) return {};
    if(value_pointer->value.size() != 1) return {};
    return value_pointer->value.front();
}
template<>
std::optional<std::int16_t> ValueConverter::from_value(const Value& value) const {
    auto value_pointer = value.get_integer16_if();
    if(value_pointer == nullptr) return {};
    if(value_pointer->value.size() != 1) return {};
    return value_pointer->value.front();
}
template<>
std::optional<std::int32_t> ValueConverter::from_value(const Value& value) const {
    auto value_pointer = value.get_integer32_if();
    if(value_pointer == nullptr) return {};
    if(value_pointer->value.size() != 1) return {};
    return value_pointer->value.front();
}
template<>
std::optional<std::int64_t> ValueConverter::from_value(const Value& value) const {
    auto value_pointer = value.get_integer64_if();
    if(value_pointer == nullptr) return {};
    if(value_pointer->value.size() != 1) return {};
    return value_pointer->value.front();
}

template<>
std::optional<float> ValueConverter::from_value(const Value& value) const {
    if(auto value_pointer = value.get_real16_if(); value_pointer != nullptr) {
        if(value_pointer->value.size() == 1) {
            return value_pointer->value.front();
        } else {
            return {};
        }
    }
    if(auto value_pointer = value.get_real32_if(); value_pointer != nullptr) {
        if(value_pointer->value.size() == 1) {
            return value_pointer->value.front();
        } else {
            return {};
        }
    }

    return {};
}
template<>
std::optional<double> ValueConverter::from_value(const Value& value) const {
    auto value_pointer = value.get_real64_if();
    if(value_pointer == nullptr) return {};
    if(value_pointer->value.size() != 1) return {};
    return value_pointer->value.front();
}


template<>
std::optional<uavcan::primitive::Empty_1_0> ValueConverter::from_value(const Value& value) const {
    return value.is_empty() ? value.get_empty() : std::optional<uavcan::primitive::Empty_1_0>{};
}
template<>
std::optional<QByteArray> ValueConverter::from_value(const Value& value) const {
    auto value_pointer = value.get_unstructured_if();
    if(value_pointer == nullptr) return {};
    if(value_pointer->value.size() != 1) return {};

    QByteArray result; result.resize(value_pointer->value.size());
    std::copy(value_pointer->value.begin(), value_pointer->value.end(), result.begin());
    return result;
}
template<>
std::optional<QBitArray> ValueConverter::from_value(const Value& value) const {
    auto value_pointer = value.get_bit_if();
    if(value_pointer == nullptr) return {};

    QBitArray result; result.resize(value_pointer->value.size());
    for(std::size_t index = 0; index != value_pointer->value.size(); ++index)
        result.setBit(index, value_pointer->value.at(index));

    return result;
}
template<>
std::optional<std::string> ValueConverter::from_value(const Value& value) const {
    auto value_pointer = value.get_string_if();
    if(value_pointer == nullptr) return {};

    return std::string{ value_pointer->value.begin(), value_pointer->value.end() };
}
template<>
std::optional<QString> ValueConverter::from_value(const Value& value) const {
    auto std_string = from_value<std::string>(value);
    if(not std_string.has_value())
        return {};

    return QString::fromStdString(std_string.value());
}
template<>
std::optional<QHostAddress> ValueConverter::from_value(const Value& value) const {
    auto qstring = from_value<QString>(value);
    if(not qstring.has_value())
        return {};

    return QHostAddress{ qstring.value() };
}
template<>
std::optional<std::chrono::system_clock::time_point> ValueConverter::from_value(const Value& value) const {
    auto timestamp = from_value<int64_t>(value);
    if(not timestamp.has_value())
        return {};

    return std::chrono::system_clock::time_point{ std::chrono::microseconds{ timestamp.value() } };
}





template<>
Value ValueConverter::to_value(const bool& value) const {
    Value result;
    result.set_bit(cyphalpp::registry::types::B(value));
    return result;
}
template<>
Value ValueConverter::to_value(const std::uint8_t& value) const {
    Value result;
    result.set_natural8(cyphalpp::registry::types::U8(value));
    return result;
}
template<>
Value ValueConverter::to_value(const std::uint16_t& value) const {
    Value result;
    result.set_natural16(cyphalpp::registry::types::U16(value));
    return result;
}
template<>
Value ValueConverter::to_value(const std::uint32_t& value) const {
    Value result;
    result.set_natural32(cyphalpp::registry::types::U32(value));
    return result;
}
template<>
Value ValueConverter::to_value(const std::uint64_t& value) const {
    Value result;
    result.set_natural64(cyphalpp::registry::types::U64(value));
    return result;
}

template<>
Value ValueConverter::to_value(const std::int8_t& value) const {
    Value result;
    result.set_integer8(cyphalpp::registry::types::I8(value));
    return result;
}
template<>
Value ValueConverter::to_value(const std::int16_t& value) const {
    Value result;
    result.set_integer16(cyphalpp::registry::types::I16(value));
    return result;
}
template<>
Value ValueConverter::to_value(const std::int32_t& value) const {
    Value result;
    result.set_integer32(cyphalpp::registry::types::I32(value));
    return result;
}
template<>
Value ValueConverter::to_value(const std::int64_t& value) const {
    Value result;
    result.set_integer64(cyphalpp::registry::types::I64(value));
    return result;
}


template<>
Value ValueConverter::to_value(const float& value) const {
    Value result;
    result.set_real32(cyphalpp::registry::types::F32(value));
    return result;
}
template<>
Value ValueConverter::to_value(const double& value) const {
    Value result;
    result.set_real64(cyphalpp::registry::types::F64(value));
    return result;
}

template<>
Value ValueConverter::to_value(const QByteArray& value) const {
    uavcan::primitive::Unstructured_1_0 data;
    data.value.resize(value.size());
    std::copy(value.begin(), value.end(), data.value.begin());

    Value result;
    result.set_unstructured(data);
    return result;
}
template<>
Value ValueConverter::to_value(const QBitArray& value) const {
    uavcan::primitive::array::Bit_1_0 data;
    data.value.resize(value.size());
    for(int index = 0; index < value.size(); ++index)
        data.value.at(index) = value.at(index);

    Value result;
    result.set_bit(data);
    return result;
}
template<>
Value ValueConverter::to_value(const std::string& value) const {
    Value result;
    result.set_string(uavcan::primitive::String_1_0{ .value = { value.begin(), value.end() } });
    return result;
}
template<>
Value ValueConverter::to_value(const QString& value) const {
    return to_value(value.toStdString());
}
template<>
Value ValueConverter::to_value(const QHostAddress& value) const {
    return to_value(value.toString());
}
template<>
Value ValueConverter::to_value(const std::chrono::system_clock::time_point& value) const {
    return to_value(int64_t{ std::chrono::duration_cast<std::chrono::microseconds>(value.time_since_epoch()).count() });
}
}
