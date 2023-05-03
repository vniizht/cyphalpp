#pragma once
#include "../Base/Types.h"

namespace cyphalpp::registry::v2_0 {
struct NameConverter {
    template<typename ResultType>
    ResultType from_name(const Name& value) const;
    template<typename SourceType>
    Name to_name(const SourceType& value) const;
};
}
