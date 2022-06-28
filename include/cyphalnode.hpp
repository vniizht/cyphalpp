//
// Copyright © 2022 JSC "VNIIZHT" or its affiliates. All Rights Reserved.
// This software is distributed under the terms of the MIT License.
//

#include "cyphalpp.hpp"
#include <chrono>
#include <uavcan/node/Heartbeat_1_0.hpp>

namespace cyphalpp {

template<typename clock_t = std::chrono::steady_clock>
class HeartbeatPublish{
private:
    const typename clock_t::time_point startTime;
    cyphalpp::CyphalUdp& uc_;
    std::unique_ptr<cyphalpp::TimerImpl> tim_;
    uavcan::node::Heartbeat_1_0 hb_;
public:
    HeartbeatPublish(cyphalpp::CyphalUdp& uc, std::unique_ptr<cyphalpp::TimerImpl> tim)
            : startTime(clock_t::now()),
            uc_(uc), tim_(std::move(tim)),
            hb_{
                .uptime=static_cast<uint32_t>(
                    std::chrono::duration_cast<std::chrono::seconds>(clock_t::now() - startTime).count()),
                .health={
                    .value=uavcan::node::Health_1_0::NOMINAL
                },
                .mode={
                    .value=uavcan::node::Mode_1_0::OPERATIONAL
                },
                .vendor_specific_status_code=0
            }
    {
        restart();
    }
    uavcan::node::Heartbeat_1_0& heart(){
        return hb_;
    }
private:
    void restart(){
        tim_->startSingleShot(1000, [this](){
            auto uptime = std::chrono::duration_cast<std::chrono::seconds>(clock_t::now() - startTime);
            hb_.uptime = uptime.count();
            uc_.sendMessage(hb_);
            restart();
        });
    }
};
} // namespace cyphalpp
