#pragma once
#include <cyphalppregistry.hpp>
#include <QHostAddress>
#include "Register.h"
#include <QString>

namespace cyphalpp::registry::v2_0::storage {

template<>
inline Register& Register::operator=(const cyphalpp::registry::types::Empty& v){
    if(canEdit()){ m_value.value.set_empty(v); set(); }
    return *this;
}
template<>
inline Register& Register::operator=(const cyphalpp::registry::types::String& v){
    if(canEdit()){ m_value.value.set_string(v); set(); }
    return *this;
}
template<>
inline Register& Register::operator=(const cyphalpp::registry::types::Bytes& v){
    if(canEdit()){ m_value.value.set_unstructured(v); set(); }
    return *this;
}
template<>
inline  Register&  Register::operator=(const cyphalpp::registry::types::Bits& v){
    if(canEdit()){ m_value.value.set_bit(v); set(); }
    return *this;
}
template<>
inline Register& Register::operator=(const cyphalpp::registry::types::Integer64& v){
    if(canEdit()){ m_value.value.set_integer64(v); set(); }
    return *this;
}
template<>
inline Register& Register::operator=(const cyphalpp::registry::types::Integer32& v){
    if(canEdit()){ m_value.value.set_integer32(v); set(); }
    return *this;
}
template<>
inline Register& Register::operator=(const cyphalpp::registry::types::Integer16& v){
    if(canEdit()){ m_value.value.set_integer16(v); set(); }
    return *this;
}
template<>
inline  Register&  Register::operator=(const cyphalpp::registry::types::Integer8& v){
    if(canEdit()){ m_value.value.set_integer8(v); set(); }
    return *this;
}
template<>
inline Register& Register::operator=(const cyphalpp::registry::types::Natural64& v){
    if(canEdit()){ m_value.value.set_natural64(v); set(); }
    return *this;
}
template<>
inline Register& Register::operator=(const cyphalpp::registry::types::Natural32& v){
    if(canEdit()){ m_value.value.set_natural32(v); set(); }
    return *this;
}
template<>
inline Register& Register::operator=(const cyphalpp::registry::types::Natural16& v){
    if(canEdit()){ m_value.value.set_natural16(v); set(); }
    return *this;
}
template<>
inline Register& Register::operator=(const cyphalpp::registry::types::Natural8& v){
    if(canEdit()){ m_value.value.set_natural8(v); set(); }
    return *this;
}
template<>
inline Register& Register::operator=(const cyphalpp::registry::types::Real64& v){
    if(canEdit()){ m_value.value.set_real64(v); set(); }
    return *this;
}
template<>
inline Register& Register::operator=(const cyphalpp::registry::types::Real32& v){
    if(canEdit()){ m_value.value.set_real32(v); set(); }
    return *this;
}
template<>
inline Register& Register::operator=(const cyphalpp::registry::types::Real16& v){
    if(canEdit()){ m_value.value.set_real16(v); set(); }
    return *this;
}


// =====================================================================================


template<>
inline cyphalpp::registry::types::String* Register::as(){
    if(not (canEdit() and m_value.value.is_string())){ return nullptr; }
    return m_value.value.get_string_if();
}

template<>
inline cyphalpp::registry::types::Bytes* Register::as(){
    if(not (canEdit() and m_value.value.is_unstructured())){ return nullptr; }
    return m_value.value.get_unstructured_if();
}

template<>
inline cyphalpp::registry::types::Bits* Register::as(){
    if(not (canEdit() and m_value.value.is_bit())){ return nullptr; }
    return m_value.value.get_bit_if();
}

template<>
inline cyphalpp::registry::types::Integer64* Register::as(){
    if(not (canEdit() and m_value.value.is_integer64())){ return nullptr; }
    return m_value.value.get_integer64_if();
}

template<>
inline cyphalpp::registry::types::Integer32* Register::as(){
    if(not (canEdit() and m_value.value.is_integer32())){ return nullptr; }
    return m_value.value.get_integer32_if();
}

template<>
inline cyphalpp::registry::types::Integer16* Register::as(){
    if(not (canEdit() and m_value.value.is_integer16())){ return nullptr; }
    return m_value.value.get_integer16_if();
}

template<>
inline cyphalpp::registry::types::Integer8* Register::as(){
    if(not (canEdit() and m_value.value.is_integer8())){ return nullptr; }
    return m_value.value.get_integer8_if();
}

template<>
inline cyphalpp::registry::types::Natural64* Register::as(){
    if(not (canEdit() and m_value.value.is_natural64())){ return nullptr; }
    return m_value.value.get_natural64_if();
}

template<>
inline cyphalpp::registry::types::Natural32* Register::as(){
    if(not (canEdit() and m_value.value.is_natural32())){ return nullptr; }
    return m_value.value.get_natural32_if();
}

template<>
inline cyphalpp::registry::types::Natural16* Register::as(){
    if(not (canEdit() and m_value.value.is_natural16())){ return nullptr; }
    return m_value.value.get_natural16_if();
}

template<>
inline cyphalpp::registry::types::Natural8* Register::as(){
    if(not (canEdit() and m_value.value.is_natural8())){ return nullptr; }
    return m_value.value.get_natural8_if();
}

template<>
inline cyphalpp::registry::types::Real64* Register::as(){
    if(not (canEdit() and m_value.value.is_real64())){ return nullptr; }
    return m_value.value.get_real64_if();
}

template<>
inline cyphalpp::registry::types::Real32* Register::as(){
    if(not (canEdit() and m_value.value.is_real32())){ return nullptr; }
    return m_value.value.get_real32_if();
}

template<>
inline cyphalpp::registry::types::Real16* Register::as(){
    if(not (canEdit() and m_value.value.is_real16())){ return nullptr; }
    return m_value.value.get_real16_if();
}


// =====================================================================================


template<>
inline const cyphalpp::registry::types::String* Register::as() const {
    if(not (canEdit() and m_value.value.is_string())){ return nullptr; }
    return m_value.value.get_string_if();
}
template<>
inline const cyphalpp::registry::types::Bytes* Register::as() const {
    if(not (canEdit() and m_value.value.is_unstructured())){ return nullptr; }
    return m_value.value.get_unstructured_if();
}
template<>
inline const cyphalpp::registry::types::Bits* Register::as() const {
    if(not (canEdit() and m_value.value.is_bit())){ return nullptr; }
    return m_value.value.get_bit_if();
}
template<>
inline const cyphalpp::registry::types::Integer64* Register::as() const {
    if(not (canEdit() and m_value.value.is_integer64())){ return nullptr; }
    return m_value.value.get_integer64_if();
}
template<>
inline const cyphalpp::registry::types::Integer32* Register::as() const {
    if(not (canEdit() and m_value.value.is_integer32())){ return nullptr; }
    return m_value.value.get_integer32_if();
}
template<>
inline const cyphalpp::registry::types::Integer16* Register::as() const {
    if(not (canEdit() and m_value.value.is_integer16())){ return nullptr; }
    return m_value.value.get_integer16_if();
}
template<>
inline const cyphalpp::registry::types::Integer8* Register::as() const {
    if(not (canEdit() and m_value.value.is_integer8())){ return nullptr; }
    return m_value.value.get_integer8_if();
}
template<>
inline const cyphalpp::registry::types::Natural64* Register::as() const {
    if(not (canEdit() and m_value.value.is_natural64())){ return nullptr; }
    return m_value.value.get_natural64_if();
}
template<>
inline const cyphalpp::registry::types::Natural32* Register::as() const {
    if(not (canEdit() and m_value.value.is_natural32())){ return nullptr; }
    return m_value.value.get_natural32_if();
}
template<>
inline const cyphalpp::registry::types::Natural16* Register::as() const {
    if(not (canEdit() and m_value.value.is_natural16())){ return nullptr; }
    return m_value.value.get_natural16_if();
}
template<>
inline const cyphalpp::registry::types::Natural8* Register::as() const {
    if(not (canEdit() and m_value.value.is_natural8())){ return nullptr; }
    return m_value.value.get_natural8_if();
}
template<>
inline const cyphalpp::registry::types::Real64* Register::as() const {
    if(not (canEdit() and m_value.value.is_real64())){ return nullptr; }
    return m_value.value.get_real64_if();
}
template<>
inline const cyphalpp::registry::types::Real32* Register::as() const {
    if(not (canEdit() and m_value.value.is_real32())){ return nullptr; }
    return m_value.value.get_real32_if();
}
template<>
inline const cyphalpp::registry::types::Real16* Register::as() const {
    if(not (canEdit() and m_value.value.is_real16())){ return nullptr; }
    return m_value.value.get_real16_if();
}


// =====================================================================================


// template<>
// inline const cyphalpp::registry::types::Bytes*  Register::as() const {
//     if(not (canEdit() and m_value.value.is_unstructured())){ return nullptr; }
//     return m_value.value.get_unstructured_if();
// }
// template<>
// inline const cyphalpp::registry::types::Bits*  Register::as() const {
//     if(not (canEdit() and m_value.value.is_bit())){ return nullptr; }
//     return m_value.value.get_bit_if();
// }
template<>
inline const int64_t* Register::as() const {
    auto ptr = as<cyphalpp::registry::types::Integer64>();
    if(not ptr) { return nullptr; }
    if(ptr->value.empty()) { return nullptr; }
    return ptr->value.data();
}
template<>
inline const int32_t* Register::as() const {
    auto ptr = as<cyphalpp::registry::types::Integer32>();
    if(not ptr) { return nullptr; }
    if(ptr->value.empty()) { return nullptr; }
    return ptr->value.data();
}
template<>
inline const int16_t* Register::as() const {
    auto ptr = as<cyphalpp::registry::types::Integer16>();
    if(not ptr) { return nullptr; }
    if(ptr->value.empty()) { return nullptr; }
    return ptr->value.data();
}
template<>
inline const int8_t* Register::as() const {
    auto ptr = as<cyphalpp::registry::types::Integer8>();
    if(not ptr) { return nullptr; }
    if(ptr->value.empty()) { return nullptr; }
    return ptr->value.data();
}
template<>
inline const uint64_t* Register::as() const {
    auto ptr = as<cyphalpp::registry::types::Natural64>();
    if(not ptr) { return nullptr; }
    if(ptr->value.empty()) { return nullptr; }
    return ptr->value.data();
}
template<>
inline const uint32_t* Register::as() const {
    auto ptr = as<cyphalpp::registry::types::Natural32>();
    if(not ptr) { return nullptr; }
    if(ptr->value.empty()) { return nullptr; }
    return ptr->value.data();
}
template<>
inline const uint16_t* Register::as() const {
    auto ptr = as<cyphalpp::registry::types::Natural16>();
    if(not ptr) { return nullptr; }
    if(ptr->value.empty()) { return nullptr; }
    return ptr->value.data();
}
template<>
inline const uint8_t* Register::as() const {
    auto ptr = as<cyphalpp::registry::types::Natural8>();
    if(not ptr) { return nullptr; }
    if(ptr->value.empty()) { return nullptr; }
    return ptr->value.data();
}
template<>
inline const double* Register::as() const {
    auto ptr = as<cyphalpp::registry::types::Real64>();
    if(not ptr) { return nullptr; }
    if(ptr->value.empty()) { return nullptr; }
    return ptr->value.data();
}
template<>
inline const float* Register::as() const {
    auto ptr = as<cyphalpp::registry::types::Real32>();
    if(not ptr) { return nullptr; }
    if(ptr->value.empty()) { return nullptr; }
    return ptr->value.data();
}
// =====================================================================================

template<>
inline bool Register::as_value(bool defaultValue) const {
    auto ptr = as<cyphalpp::registry::types::Bits>();
    if(not ptr) return defaultValue;
    if(ptr->value.empty()) return defaultValue;
    return ptr->value.front();
}

template<>
inline int64_t Register::as_value(int64_t defaultValue) const {
    auto ptr = as<int64_t>();
    if(not ptr) return defaultValue;
    return *ptr;
}
template<>
inline int32_t Register::as_value(int32_t defaultValue) const {
    auto ptr = as<int32_t>();
    if(not ptr) return defaultValue;
    return *ptr;
}
template<>
inline int16_t Register::as_value(int16_t defaultValue) const {
    auto ptr = as<int16_t>();
    if(not ptr) return defaultValue;
    return *ptr;
}
template<>
inline int8_t  Register::as_value(int8_t defaultValue) const {
    auto ptr = as<int8_t>();
    if(not ptr) return defaultValue;
    return *ptr;
}
template<>
inline uint64_t Register::as_value(uint64_t defaultValue) const {
    auto ptr = as<uint64_t>();
    if(not ptr) return defaultValue;
    return *ptr;
}
template<>
inline uint32_t Register::as_value(uint32_t defaultValue) const {
    auto ptr = as<uint32_t>();
    if(not ptr) return defaultValue;
    return *ptr;
}
template<>
inline uint16_t Register::as_value(uint16_t defaultValue) const {
    auto ptr = as<uint16_t>();
    if(not ptr) return defaultValue;
    return *ptr;
}
template<>
inline uint8_t Register::as_value(uint8_t defaultValue) const {
    auto ptr = as<uint8_t>();
    if(not ptr) return defaultValue;
    return *ptr;
}
template<>
inline double Register::as_value(double defaultValue) const {
    auto ptr = as<double>();
    if(not ptr) return defaultValue;
    return *ptr;
}
template<>
inline float Register::as_value(float defaultValue) const {
    auto ptr = as<float>();
    if(not ptr) return defaultValue;
    return *ptr;
}


template<>
inline std::string Register::as_value(std::string defaultValue) const {
    auto string_pointer = as<cyphalpp::registry::types::String>();
    if(not string_pointer) { return defaultValue; }

    return std::string{ string_pointer->value.begin(), string_pointer->value.end() };
}
template<>
inline QString Register::as_value(QString defaultValue) const {
    if(auto result = QString::fromStdString(as_value<std::string>()); not result.isNull())
        return result;

    return defaultValue;
}
template<>
inline QHostAddress Register::as_value(QHostAddress defaultValue) const {
    if(QHostAddress result{ as_value<QString>() }; not result.isNull())
        return result;

    return defaultValue;
}



template<>
inline Register& Register::operator=(const std::string& v){
    uavcan::primitive::String_1_0 buffer{};
    buffer.value.resize(v.size());
    std::copy(v.begin(), v.end(), buffer.value.begin());
    return ((*this) = buffer);
}
template<>
inline Register& Register::operator=(const QString& v){
    return ((*this) = v.toStdString());
}
template<>
inline Register& Register::operator=(const QHostAddress& v){
    return ((*this) = v.toString());
}
}
