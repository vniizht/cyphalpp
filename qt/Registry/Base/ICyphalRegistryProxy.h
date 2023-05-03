#pragma once
#include "Types.h"

namespace cyphalpp::registry::v2_0 {
struct ICyphalRegistryProxy {
private:
    const QString m_name;
public:
    const QString& name() const { return m_name; }
    virtual ~ICyphalRegistryProxy() = default;
    ICyphalRegistryProxy(const QString& name)
        :m_name{ name } {}
public:
    virtual Access::Response access(const Access::Request& request) = 0;
    virtual List::Response list(const List::Request& request) = 0;
    virtual std::size_t registersCount() const = 0;
};
}
