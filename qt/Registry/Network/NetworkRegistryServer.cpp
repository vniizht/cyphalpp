#include "NetworkRegistryServer.h"

namespace cyphalpp::registry::v2_0::network {
NetworkRegistryServer::NetworkRegistryServer(const QString& name, std::shared_ptr<ICyphalRegistryProxy> storage)
    :ICyphalRegistryProxy{ name }, m_storage{}, m_state{} {
    setImplementation(std::move(storage));
}

void NetworkRegistryServer::setImplementation(std::shared_ptr<ICyphalRegistryProxy> storage) {
    m_storage.swap(storage);
}
void NetworkRegistryServer::setNode(cyphalpp::CyphalUdp &node) {
    removeNode();
    m_state = std::make_unique<CyphalNodeState>(std::ref(*this), std::ref(node));
}
void NetworkRegistryServer::removeNode() {
    m_state = {};
}

Access::Service_1_0::Response NetworkRegistryServer::access(const Access::Service_1_0::Request &request) {
    return m_storage->access(request);
}
List::Service_1_0::Response NetworkRegistryServer::list(const List::Service_1_0::Request &request) {
    return m_storage->list(request);
}
std::size_t NetworkRegistryServer::registersCount() const {
    return m_storage->registersCount();
}

NetworkRegistryServer::CyphalNodeState::CyphalNodeState(NetworkRegistryServer &self, cyphalpp::CyphalUdp &node):m_node{ node } {
    m_access = m_node.subscribeServiceRequest<Access>(
    [self = &self](const cyphalpp::TransferMetadata& /*metadata*/, const Access::Request& request) -> Access::Response{
        return self->access(request);
    }
    );
    m_list = m_node.subscribeServiceRequest<List>(
    [self = &self](const cyphalpp::TransferMetadata& /*metadata*/, const List::Request& request) -> List::Response {
        return self->list(request);
    });
}
NetworkRegistryServer::CyphalNodeState::~CyphalNodeState(){
    m_node.unsubscribe(m_access);
    m_node.unsubscribe(m_list);
}
}
