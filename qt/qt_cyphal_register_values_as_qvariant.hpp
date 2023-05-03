#ifndef CYPHALPP_REGISTER_VALUES_AS_QVARIANT_HPP
#define CYPHALPP_REGISTER_VALUES_AS_QVARIANT_HPP
#include "cyphalppregistry.hpp"

class QVariant;

namespace cyphalpp {
namespace registry {

QVariant toVariant(const Value& v);
Value fromVariant(const QVariant& v);


} // namespace registry
} // namespace cyphalpp

#endif // ndef CYPHALPP_REGISTER_VALUES_AS_QVARIANT_HPP
