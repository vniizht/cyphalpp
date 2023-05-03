#pragma once
#include <array>
#include <uavcan/_register/Access_1_0.qt.hpp>
#include <uavcan/_register/List_1_0.qt.hpp>
#include <uavcan/_register/Value_1_0.qt.hpp>
#include <uavcan/_register/Name_1_0.qt.hpp>

namespace cyphalpp::registry::v2_0 {
    using Access = uavcan::_register::Access::Service_1_0;
    using List = uavcan::_register::List::Service_1_0;
    using Value = uavcan::_register::Value_1_0;
    using Name = uavcan::_register::Name_1_0;
}

