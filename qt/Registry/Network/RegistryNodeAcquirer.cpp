#include "RegistryNodeAcquirer.h"

namespace cyphalpp::registry::v2_0::network {
RegistryNodeAcquirer::RegistryNodeAcquirer(std::shared_ptr<NetworkRegistryServer> registry, std::shared_ptr<cyphalpp::CyphalUdp> node)
    :m_registry{ std::move(registry) } {
        m_registry->setNode(*node);
    }
RegistryNodeAcquirer::~RegistryNodeAcquirer() {
    m_registry->removeNode();
}
}
