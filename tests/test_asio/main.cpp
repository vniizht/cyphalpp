//
// Copyright © 2022 JSC "VNIIZHT" or its affiliates. All Rights Reserved.
// This software is distributed under the terms of the MIT License.
//

#define BOOST_TEST_MODULE AsioTests
#include <boost/test/included/unit_test.hpp>
#include <uavcan/node/Heartbeat_1_0.hpp>
#include <asio_cyphalpp.hpp>

bool operator==(const uavcan::node::Health_1_0&l,const uavcan::node::Health_1_0&r){ return l.value == r.value; }
bool operator==(const uavcan::node::Mode_1_0&l,const uavcan::node::Mode_1_0&r){ return l.value == r.value; }

bool operator==(const uavcan::node::Heartbeat_1_0& l, const uavcan::node::Heartbeat_1_0& r){
    return l.uptime == r.uptime
        && l.health == r.health
        && l.mode == r.mode
        && l.vendor_specific_status_code == r.vendor_specific_status_code;
}
using namespace cyphalpp;
using namespace boost::asio;
using boost::asio::ip::address_v4;
using boost::asio::ip::udp;
using namespace std::chrono_literals;

template<typename Base, uint16_t node_id>
struct Node: public virtual Base{
    CyphalUdp node;
    Node():Base(), node(Base::udpSockets, Base::timers){
        node.setAddr(Base::base_addr() | node_id);
    }
    virtual ~Node() override {
    }
};

struct CyphalAsioNetwork{
    io_service service;
    SocketFactory udpSockets;
    TimerFactory timers;
    CyphalAsioNetwork():
        service{},
        udpSockets{asio::asioUdpSocket(service)},
        timers{asio::asioTimer(service)}{

    }
    static uint32_t base_addr(){ return address_v4::loopback().to_uint() & (~0x1FFF);}
    virtual ~CyphalAsioNetwork(){}
};

template<uint16_t node_id>
using AsioNode = Node<CyphalAsioNetwork, node_id>;

struct TwoNodes: public AsioNode<1>, public AsioNode<2>{
    TwoNodes(): AsioNode<1>(), AsioNode<2>(){}
    virtual ~TwoNodes() override {}
};

BOOST_FIXTURE_TEST_CASE( TestMessageFixedId, TwoNodes)
{
    bool called{false};
    const uavcan::node::Heartbeat_1_0 shb{
        .uptime=12,
        .health={.value=uavcan::node::Health_1_0::WARNING},
        .mode={.value=uavcan::node::Mode_1_0::OPERATIONAL},
        .vendor_specific_status_code=128
    };
    AsioNode<1>::node.subscribeMessage<uavcan::node::Heartbeat_1_0>([this, &called, &shb](const TransferMetadata& tm, const uavcan::node::Heartbeat_1_0& hb){
        if(tm.data.node_id == 2 and hb == shb){
            called = true;
            service.stop();
        }
    });


    AsioNode<2>::node.sendMessage(shb);

    service.run_for(1000ms);

    BOOST_TEST( called );
}

#include <uavcan/si/sample/length/Scalar_1_0.hpp>

bool operator==(const uavcan::time::SynchronizedTimestamp_1_0& l, const uavcan::time::SynchronizedTimestamp_1_0& r){
    return l.microsecond == r.microsecond;
}
bool operator==(const uavcan::si::sample::length::Scalar_1_0& l, const uavcan::si::sample::length::Scalar_1_0& r){
    return l.timestamp == r.timestamp
        && l.meter == r.meter;
}
BOOST_FIXTURE_TEST_CASE( TestMessageRandomId, TwoNodes)
{
    bool called{false};
    const uavcan::si::sample::length::Scalar_1_0 shb{
        .timestamp={.microsecond=12},
        .meter =12345
    };
    AsioNode<1>::node.subscribeMessage<uavcan::si::sample::length::Scalar_1_0>([this, &called, &shb](
            const TransferMetadata& tm, const uavcan::si::sample::length::Scalar_1_0& hb
    ){
        if(tm.data.node_id == 2 and hb == shb){
            called = true;
            service.stop();
        }
    }, fixed_port_id<100>);


    AsioNode<2>::node.sendMessage(shb, fixed_port_id<100>);

    service.run_for(1000ms);

    BOOST_TEST( called );
}

#include <uavcan/node/ExecuteCommand_1_1.hpp>

using ExecuteCommand = uavcan::node::ExecuteCommand::Service_1_1;

bool operator ==(const ExecuteCommand::Request& l, const ExecuteCommand::Request& r){
    return l.command == r.command && l.parameter == r.parameter;
}

bool operator ==(const ExecuteCommand::Response& l, const ExecuteCommand::Response& r){
    return l.status == r.status;
}

BOOST_FIXTURE_TEST_CASE( TestServiceWithFixedId, TwoNodes)
{
    bool callback_called{false};
    bool answered{false};

    const ExecuteCommand::Request sreq{
        .command=ExecuteCommand::Request::COMMAND_STORE_PERSISTENT_STATES,
        .parameter={}
    };
    const ExecuteCommand::Response ans{
        .status = ExecuteCommand::Response::STATUS_INTERNAL_ERROR
    };
    AsioNode<2>::node.subscribeServiceRequest<ExecuteCommand>([this, &callback_called, &sreq, &ans](const TransferMetadata& tm, const ExecuteCommand::Request& req){
        if(tm.data.node_id == 1 and req == sreq){
            callback_called = true;
        }else{
            service.stop();
        }
        return ans;
    });


    auto service_caller = AsioNode<1>::node.prepareServiceCalls<ExecuteCommand>([this, &answered, &ans](const TransferMetadata& tm, const ExecuteCommand::Response& rsp){
        if(tm.data.node_id == 2 and ans == rsp){
            answered = true;
            service.stop();
        }
    }, [this](){
        std::clog << "Error!" << std::endl;
        service.stop();}
    );

    BOOST_TEST( bool{service_caller->call(2, sreq)} );

    service.run_for(5000ms);

    BOOST_TEST( callback_called );
    BOOST_TEST( answered );
}

BOOST_FIXTURE_TEST_CASE( TestServiceWithRandomId, TwoNodes)
{
    bool callback_called{false};
    bool answered{false};

    const ExecuteCommand::Request sreq{
        .command=ExecuteCommand::Request::COMMAND_STORE_PERSISTENT_STATES,
        .parameter={}
    };
    const ExecuteCommand::Response ans{
        .status = ExecuteCommand::Response::STATUS_INTERNAL_ERROR
    };
    AsioNode<2>::node.subscribeServiceRequest<ExecuteCommand>([this, &callback_called, &sreq, &ans](const TransferMetadata& tm, const ExecuteCommand::Request& req){
        if(tm.data.node_id == 1 and req == sreq){
            callback_called = true;
        }else{
            service.stop();
        }
        return ans;
    });


    auto service_caller = AsioNode<1>::node.prepareServiceCalls<ExecuteCommand>([this, &answered, &ans](const TransferMetadata& tm, const ExecuteCommand::Response& rsp){
        if(tm.data.node_id == 2 and ans == rsp){
            answered = true;
            service.stop();
        }
    }, [this](){
        std::clog << "Error!" << std::endl;
        service.stop();
    });

    BOOST_TEST( bool{service_caller->call(2, sreq)} );

    service.run_for(5000ms);

    BOOST_TEST( callback_called );
    BOOST_TEST( answered );
}
