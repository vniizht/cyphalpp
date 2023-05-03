#pragma once
#include "../../Base/ICyphalRegistryProxy.h"
#include <qt_cyphal_registry.hpp>
#include <QHostAddress>


namespace cyphalpp::registry::v2_0::storage::qsettings {
/*!
 * \brief The QSettingsStorage class
 *
 * \details A class that provides backward compatibility with a previous version of registers.
 */
class QSettingsStorage : public ICyphalRegistryProxy {
private:
    std::shared_ptr<cyphalpp::registry::QSettingsRegistry> m_storage;
public:
    QSettingsStorage(std::shared_ptr<cyphalpp::registry::QSettingsRegistry> storage);
    virtual ~QSettingsStorage() override;
public:
    virtual Access::Response access(const Access::Request &request) override;
    virtual List::Response list(const List::Request &request) override;
    virtual std::size_t registersCount() const override;
public:
    void save();
};
}
