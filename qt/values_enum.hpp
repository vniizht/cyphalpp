//
// Copyright © 2022 JSC "VNIIZHT" or its affiliates. All Rights Reserved.
// This software is distributed under the terms of the MIT License.
//

#ifndef VALUES_ENUM_HPP
#define VALUES_ENUM_HPP
#include <QString>
#include <QVariant>

namespace cyphalpp {

namespace qt{

template<typename E, typename T, T E::* field>
struct Enum{
    std::vector<T> values;
    std::vector<QString> names;
    std::vector<QString> descriptions;

    struct Value{
        T value;
        QString name;
        QString description;
    };

    Enum(std::initializer_list<Value> vals){
        for(auto& v: vals){
            values.emplace_back(v.value);
            names.emplace_back(v.name);
            descriptions.emplace_back(v.description);
        }
    }

    QVariant getName(const E& e) const {
        auto pos = std::find(values.begin(), values.end(), e.*field);
        if(pos == values.end()){
            return QVariant{QStringLiteral("Unknown value %1").arg(e.*field)};
        }
        auto i = pos - values.begin();
        return names[i];
    }
    QVariant getDescription(const E& e) const {
        auto pos = std::find(values.begin(), values.end(), e.*field);
        if(pos == values.end()){
            return QVariant{QStringLiteral("Unknown value %1").arg(e.*field)};
        }
        auto i = pos - values.begin();
        return descriptions[i];
    }
    E getValue(int i) const {
        E ret{};
        if(i>=0 and static_cast<size_t>(i)< values.size()){
            ret.*field = values.at(i);
        }
        return ret;
    }
};

template<typename E>
using EnumValue = Enum<E, decltype (E::value), &E::value>;

template<typename E>
using EnumStatus = Enum<E, decltype (E::status), &E::status>;

} // namespace qt

} // namespace cyphalpp
#endif // VALUES_ENUM_HPP
