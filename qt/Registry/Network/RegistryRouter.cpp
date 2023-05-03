#include "RegistryRouter.h"

namespace cyphalpp::registry::v2_0::network {
RegistryRouter::RegistryRouter(const QString &name)
    :ICyphalRegistryProxy{ name } {}

void RegistryRouter::resetHandler(std::shared_ptr<ICyphalRegistryProxy>&& handler) {
    m_handlers[handler->name()] = std::move(handler);
}
void RegistryRouter::removeHandler(const QString &path) {
    m_handlers.erase(path);
}

Access::Response RegistryRouter::access(const Access::Request &request) {
    const auto name = s_nameConverter.from_name<QString>(request.name);
    if(not name.startsWith(this->name() + '.'))
        return {};

    const auto handler_name = name.mid(this->name().size() + 1);//+1 for '.'
    for(auto& handler: m_handlers) {
        if(handler_name.startsWith(handler.first)) {
            return handler.second->access(Access::Request {
                .name = s_nameConverter.to_name(handler_name),
                .value = request.value
            });
        }
    }
    return {};
}
List::Response RegistryRouter::list(const List::Request &request) {
    auto index = request.index;
    for(auto iter = m_handlers.begin(); iter != m_handlers.end(); ++iter) {
        const auto storage_size = iter->second->registersCount();
        if(index < storage_size) {
            const auto result = iter->second->list(List::Request{ .index = index });
            return List::Response{ .name = s_nameConverter.to_name(name() + '.' + s_nameConverter.from_name<QString>(result.name)) };
        } else {
            index -= storage_size;
        }
    }
    return {};
}

std::size_t RegistryRouter::registersCount() const {
    return std::accumulate(m_handlers.begin(), m_handlers.end(), std::size_t{ 0 },
        [](std::size_t previous, const std::pair<QString, std::shared_ptr<ICyphalRegistryProxy>>& handler) -> std::size_t {
            return static_cast<std::size_t>(previous + handler.second->registersCount());
        });
}
}
