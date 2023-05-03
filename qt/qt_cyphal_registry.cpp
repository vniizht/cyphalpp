#include "qt_cyphal_registry.hpp"
#include <QJsonValue>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <uavcan/node/ExecuteCommand_1_1.hpp>
#include "qt_cyphal_register_values_as_qvariant.hpp"

namespace cyphalpp {
namespace registry {



static Value fromJson(const QString& s, bool* ok = nullptr){
    QJsonParseError pe;
    auto var = QJsonDocument::fromJson(s.toUtf8(), &pe).toVariant();
    if(pe.error != QJsonParseError::NoError){
        if(ok){*ok = false;}
        return Value{};
    }
    if(ok){*ok = true;}
    return fromVariant(var);
}

static QString toJson(const Value& s){
    auto v = toVariant(s);
    if(v.type() == QVariant::Map){
        return QString::fromUtf8(QJsonDocument::fromVariant(v).toJson());
    }
    return v.toString();
}

static std::string name_view(const Name& n){
    return std::string(
        reinterpret_cast<const char*>(n.name.data()),
        n.name.size()
    );
}

class QSettingsRegistryPrivate{
    Q_DECLARE_PUBLIC(QSettingsRegistry)
    
    QSettingsRegistry* const q_ptr;
    std::vector<Registry::RegisterType> values_;
    std::unordered_map<std::string, size_t> names_;
public:
    QSettingsRegistryPrivate(QSettingsRegistry*const q):q_ptr(q), values_(), names_(){
        
    }
};

QSettingsRegistry::QSettingsRegistry(const QString &fileName, Format format)
    :QSettings(fileName, format), RegistryImpl(), d_ptr(new QSettingsRegistryPrivate(this))
{
}

std::unique_ptr<QSettingsRegistry> QSettingsRegistry::open(
        const QString &fileName, Format format)
{
    return std::unique_ptr<QSettingsRegistry>{
        new QSettingsRegistry(fileName, format)
    };
}

List::Response QSettingsRegistry::getName(uint16_t i){
    Q_D(const QSettingsRegistry);
    List::Response r{
        .name = Name{}
    };
    if(i < d->values_.size()){
        r.name = d->values_.at(i).name;
    }
    return r;
}

Access::Response QSettingsRegistry::get(const Name &n) const
{
    Q_D(const QSettingsRegistry);
    auto id = d->names_.find(name_view(n));
    if(d->names_.end() == id){
        return Access::Response{};
    }
    return d->values_.at(id->second).value;
}

Access::Response QSettingsRegistry::set(const Name &n, const Value &v)
{
    Q_D(QSettingsRegistry);
    auto id = d->names_.find(name_view(n));
    if(d->names_.end() == id){
        if(v.is_empty()){ return Access::Response{}; }
        d->values_.emplace_back(Registry::RegisterType{
                .name = n,
                .value = {
                    .timestamp = {.microsecond=0},
                    ._mutable = true,
                    .persistent = true,
                    .value = v
                }});
        const auto& last = d->values_.back();
        d->names_.insert({name_view(last.name), d->values_.size() - 1 });
        return last.value;
    }
    auto& ret = d->values_.at(id->second).value;
    if(not v.is_empty() and ret._mutable){
        ret.value = v;
    }
    return ret;
}

uint16_t QSettingsRegistry::size() const
{
    Q_D(const QSettingsRegistry);
    return d->names_.size();
}

void QSettingsRegistry::setAutosave(bool)
{
    
}

void QSettingsRegistry::load()
{
    auto allks = allKeys();
    for(const auto& k: allks){
        auto kba = k.toUtf8();
        set(
            Name{{kba.constBegin(), kba.constEnd()}}, 
            fromJson(value(k).toString())
        );
    }
}

void QSettingsRegistry::save()
{
    Q_D(QSettingsRegistry);
    for(const auto& r: d->values_){
        auto ksv = name_view(r.name);
        auto k = QString::fromUtf8(&ksv[0], ksv.size());
        auto v = toJson(r.value.value);
        setValue(k, v);
    }
    
    QSettings::sync();
}

QSettingsRegistry::~QSettingsRegistry(){
    delete d_ptr;
}

} // namespace registry
} // namespace cyphalpp
