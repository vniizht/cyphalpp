//
// Copyright © 2022 JSC "VNIIZHT" or its affiliates. All Rights Reserved.
// This software is distributed under the terms of the MIT License.
//
#ifndef CYPHALPP_QT_CYPHAL_REGISTRY_HPP
#define CYPHALPP_QT_CYPHAL_REGISTRY_HPP
#include <QSettings>
#include "cyphalppregistry.hpp"


namespace cyphalpp {

namespace registry {

class QSettingsRegistryPrivate;
class QSettingsRegistry: QSettings, public RegistryImpl {
    Q_OBJECT
    Q_DECLARE_PRIVATE(QSettingsRegistry)
    QSettingsRegistry(const QString &fileName, Format format);
public:
    static std::unique_ptr<QSettingsRegistry> open(const QString &fileName, Format format);
    
    virtual List::Response getName(uint16_t i) override;
    virtual Access::Response get(const Name& n) const override;
    virtual Access::Response set(const Name& n, const Value& v) override;
    virtual uint16_t size()  const override;
    virtual void setAutosave(bool autosave) override;
    virtual void load() override;
    virtual void save() override;
    virtual ~QSettingsRegistry();
private:
    QSettingsRegistryPrivate* const d_ptr;
};


} // namespace registry

} // namespace cyphalpp


#endif // ndef CYPHALPP_QT_CYPHAL_REGISTRY_HPP
