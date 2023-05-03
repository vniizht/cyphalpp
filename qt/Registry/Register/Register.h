#pragma once
#include "../Base/ICyphalRegistryProxy.h"

namespace cyphalpp::registry::v2_0::storage {
class Register {
private:
    ICyphalRegistryProxy& m_storage;
    Name m_name;
    Access::Response m_value;
public:
    template<typename T>
    Register& operator=(const T& v);

    const Value& value() const &{ return m_value.value; }
    const Value value() const &&{ return m_value.value; }
    Value& value() & { return m_value.value; }
    Value value() && { return m_value.value; }

    template<typename T>
    T* as();
    template<typename T>
    const T* as() const;
    template<typename T>
    T as_value(T defaultValue = {}) const;
public:
    template<typename Default>
    Register(ICyphalRegistryProxy& storage, const Name& name, const Default& defaultValue)
        :Register{ storage, name } {
            if(m_value.value.is_empty()){
                m_value._mutable = true;
                m_value.persistent = true;
                (*this) = defaultValue;
            }
        }
    Register(ICyphalRegistryProxy& storage, const Name& name)
        :m_storage{ storage }, m_name{ name } {
            update();
        }
    ~Register() {
        set();
    }
public:
    bool canEdit() const{
        return m_value._mutable or m_value.value.is_empty();
    }
    void update() {
        m_value = m_storage.access(Access::Request{ .name = m_name, .value = {} });
    }
    void set() {
        m_value = m_storage.access(Access::Request{ .name = m_name, .value = m_value.value });
    }
};
}
