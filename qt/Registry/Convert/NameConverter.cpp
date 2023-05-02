#include "NameConverter.h"

namespace cyphalpp::registry::v2_0 {
template<>
std::string NameConverter::from_name(const Name& value) const {
    return std::string{ value.name.begin(), value.name.end() };
}
template<>
QString NameConverter::from_name(const Name& value) const {
    return QString::fromStdString(from_name<std::string>(value));
}
template<>
Name NameConverter::to_name(const std::string& value) const {
    return Name { .name = { value.begin(), value.end() } };
}
template<>
Name NameConverter::to_name(const QString& value) const {
    return to_name(value.toStdString());
}

}
