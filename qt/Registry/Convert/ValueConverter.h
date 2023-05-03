#pragma once
#include "../Base/Types.h"
#include <optional>

namespace cyphalpp::registry::v2_0 {
/*!
 * \brief The ValueConverter class
 *
 * \details Default coverter.
 * Support only arrays of single size.
 */
struct ValueConverter {
    template<typename ResultType>
    std::optional<ResultType> from_value(const Value& value) const;
    template<typename SourceType>
    Value to_value(const SourceType& value) const;
};
}
