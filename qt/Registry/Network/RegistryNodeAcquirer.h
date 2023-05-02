#pragma once
#include "NetworkRegistryServer.h"

namespace cyphalpp::registry::v2_0::network {
/*!
 * \brief The RegistryNodeAcquirer class
 *
 * \details RAII-обёртка, в конструкторе делающая regisry->setNode,
 * а в деструктор -- registry->removeNode().
 *
 * Закреплять cyphal-ноду за registry необходимо, чтобы можно
 * было обновлять registry извне по сети. При этом работа
 * будет вестись как раз через эту ноду.
 *
 * Автоматическая отписка необходима для корректного удаления
 * обоих классов.
 */
class RegistryNodeAcquirer final {
private:
    std::shared_ptr<NetworkRegistryServer> m_registry;
public:
    RegistryNodeAcquirer(std::shared_ptr<NetworkRegistryServer> registry, std::shared_ptr<cyphalpp::CyphalUdp> node);
    ~RegistryNodeAcquirer();
};
}

