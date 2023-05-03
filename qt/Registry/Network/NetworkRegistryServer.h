#pragma once
#include "../Base/ICyphalRegistryProxy.h"
#include <qt_cyphal_udp.hpp>

namespace cyphalpp::registry::v2_0::network {
class NetworkRegistryServer : public ICyphalRegistryProxy {
private:
    class CyphalNodeState {
    private:
        cyphalpp::CyphalUdp& m_node;
        cyphalpp::CyphalUdp::SubscriptionHandle m_access;
        cyphalpp::CyphalUdp::SubscriptionHandle m_list;
    public:
        CyphalNodeState(NetworkRegistryServer& self, cyphalpp::CyphalUdp& node);
        ~CyphalNodeState();
    };
private:
    std::shared_ptr<ICyphalRegistryProxy> m_storage;
    std::shared_ptr<CyphalNodeState> m_state;
public:
    NetworkRegistryServer(const QString& name, std::shared_ptr<ICyphalRegistryProxy> storage);
    void setImplementation(std::shared_ptr<ICyphalRegistryProxy> storage);
    void setNode(cyphalpp::CyphalUdp& node);
    void removeNode();
public:
    virtual Access::Response access(const Access::Request &request) override;
    virtual List::Response list(const List::Request &request) override;
    virtual std::size_t registersCount() const override;
};
}
