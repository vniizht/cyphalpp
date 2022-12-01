#ifndef ASIO_FIXTURE_HPP
#define ASIO_FIXTURE_HPP

#define BOOST_TEST_NO_LIB
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
    Node():Base(), node(Base::base_addr() | node_id, Base::udpSockets, Base::timers){
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



template<uint16_t... ids>
struct Network: public AsioNode<ids>...{
    Network(): AsioNode<ids>()...{}
    virtual ~Network() override {}
};

using TwoNodes = Network<1,2>;

#endif // ndef ASIO_FIXTURE_HPP
