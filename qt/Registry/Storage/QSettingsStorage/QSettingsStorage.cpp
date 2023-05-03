#include "QSettingsStorage.h"

namespace cyphalpp::registry::v2_0::storage::qsettings {
QSettingsStorage::QSettingsStorage(std::shared_ptr<cyphalpp::registry::QSettingsRegistry> storage)
    :ICyphalRegistryProxy{ "qsettings-storage" }, m_storage{ std::move(storage) } { m_storage->load(); }
QSettingsStorage::~QSettingsStorage() {
    save();
}
Access::Service_1_0::Response QSettingsStorage::access(const Access::Service_1_0::Request &request) {
    if(request.value.is_empty()) {
        return m_storage->get(request.name);
    } else {
        return m_storage->set(request.name, request.value);
    }
}
List::Service_1_0::Response QSettingsStorage::list(const List::Service_1_0::Request &request) {
    return m_storage->getName(request.index);
}
std::size_t QSettingsStorage::registersCount() const {
    return m_storage->size();
}

void QSettingsStorage::save() {
    m_storage->save();
}
}
