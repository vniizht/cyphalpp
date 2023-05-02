#pragma once
#include "../Convert/NameConverter.h"
#include "../Base/ICyphalRegistryProxy.h"
#include <functional>

namespace cyphalpp::registry::v2_0::network {
class RegistryRouter : public ICyphalRegistryProxy {
private:
    inline static const NameConverter s_nameConverter{};
    std::map<QString, std::shared_ptr<ICyphalRegistryProxy>> m_handlers;
public:
    RegistryRouter(const QString& name);
    void resetHandler(std::shared_ptr<ICyphalRegistryProxy>&& handler = {});
    void removeHandler(const QString& path);
public:
    virtual Access::Response access(const Access::Request& request) override;
    virtual List::Response list(const List::Request& request) override;
    virtual std::size_t registersCount() const override;
};
}
