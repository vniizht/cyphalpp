//
// Copyright © 2022 JSC "VNIIZHT" or its affiliates. All Rights Reserved.
// This software is distributed under the terms of the MIT License.
//

#ifndef UAVCAN_ASYNC_UDP_HPP
#define UAVCAN_ASYNC_UDP_HPP
#include <vector>
#include <memory>
#include <functional>
#include "tl/expected.hpp"
#include "mpark/variant.hpp"
#include "cyphal_udp/Header_1_0.hpp"
#include "cyphal_udp/UnicastBridgeHeader_1_0.hpp"

namespace cyphalpp{

namespace errors{

enum class DataSpecifierConversion: uint8_t{
    WrongSubnet=0,
    WrongPort,
    PortTooSmall
};

enum class ParsePacket: uint8_t{
    MessageTooSmall=0,
    RolesMismatch,
    SubjectIdMismatch,
    HeaderWrongProtocolVersion,
    MultiframeTransfersNotSupported
};

enum class ServiceCall: uint8_t{
    TransferIdOutOfSync=0
};

} // namespace errors

using Error = mpark::variant<
    errors::DataSpecifierConversion,
    errors::ParsePacket,
    errors::ServiceCall,
    nunavut::support::Error,
    int8_t
>;

static inline tl::unexpected<Error> operator-(errors::DataSpecifierConversion d){
    return tl::unexpected<Error>{d};
}
static inline tl::unexpected<Error> operator-(errors::ParsePacket d){
    return tl::unexpected<Error>{d};
}
static inline tl::unexpected<Error> operator-(errors::ServiceCall d){
    return tl::unexpected<Error>{d};
}

/**
 * MessageAddr - a class representing IPv4 address for UAVCAN/UDP
 */
struct MessageAddr{
public:
    constexpr MessageAddr(uint8_t subnet_id, uint16_t subject_id)
        :value_(0b111011110 << 23){
        value_ |= ( (static_cast<uint32_t>(subnet_id) & 0x7FU ) << 16 );
        value_ |=   (static_cast<uint32_t>(subject_id) & 0x1FFFU );
    }
    constexpr MessageAddr(uint16_t prefix_9bits, uint8_t subnet_id, uint16_t subject_id)
        :value_(0){
        value_ |= (prefix_9bits & 0x1FFU) << 23;
        value_ |= ( (static_cast<uint32_t>(subnet_id) & 0x7FU ) << 16 );
        value_ |=   (static_cast<uint32_t>(subject_id) & 0x1FFFU );
    }
    /// constructs MessageAddr with 32-bit IPv4 address in __HOST__ byte order!
    MessageAddr(uint32_t addr) : value_(addr) { }
    constexpr uint32_t prefix_9bit() const{ return (value_ >> 23) & 0x1FF; }
    constexpr uint8_t subnet_id() const {   return static_cast<uint8_t>((value_ >> 16) & 0x7FU); }
    /// for multicast addresses returns subject_id, for unicast adresses returns node_id
    constexpr uint16_t subject_or_node_id() const { return static_cast<uint16_t>(value_ & 0x1FFFU); }
    /// returns if this address is a valid multicast address for UAVCAN/UDP
    constexpr bool isValidMessage() const{  return prefix_9bit() ==  0b111011110; }
    /// return 32-bit IPv4 address in __HOST__ byte order!
    operator uint32_t()const { return value_;  }
private:
    uint32_t value_;
};

constexpr static size_t UDP_MAX_MTU=1200;
constexpr static uint16_t MULTICAST_PORT = 16383;
constexpr static uint16_t UNICAST_BASE_PORT = MULTICAST_PORT + 1;
constexpr static uint16_t MAX_SERVICE_SUBJECT_ID = 8191;



/**
 * Describes data transfer - to/from which node it goes
 */
struct NetworkAddress;
struct DataSpecifier{
    enum Role: uint32_t{
        Message=cyphal_udp::UnicastBridgeHeader_1_0::ROLE_MESSAGE,
        ServiceRequest=cyphal_udp::UnicastBridgeHeader_1_0::ROLE_SERVICE_REQUEST,
        ServiceResponce=cyphal_udp::UnicastBridgeHeader_1_0::ROLE_SERVICE_RESPONSE,
    } role;
    uint16_t node_id;       // for service call describes destination node_id, for messages should be 0
    uint16_t subject_id;    // service_id or subject_id for services and messages respectfully

    static tl::expected<DataSpecifier, Error> from_packet(
        const MessageAddr& currentAddr, const NetworkAddress& source, const NetworkAddress& dest);

    bool operator==(const DataSpecifier& other) const{
        return role == other.role && node_id == other.node_id && subject_id == other.node_id;
    }
};


/**
 * IPv4 address with port
 */
struct NetworkAddress{
    MessageAddr addr;
    uint16_t port;
    static NetworkAddress from_data_specifier(const MessageAddr& currentdAddr, const DataSpecifier& ds);
};


inline NetworkAddress NetworkAddress::from_data_specifier(const MessageAddr& currentdAddr, const DataSpecifier& ds){
    if(ds.role == DataSpecifier::Message){
        return NetworkAddress{ MessageAddr(currentdAddr.subnet_id(), ds.subject_id), MULTICAST_PORT };
    }
    return NetworkAddress{
            MessageAddr(currentdAddr.prefix_9bit(), currentdAddr.subnet_id(), ds.node_id),
            static_cast<uint16_t>(UNICAST_BASE_PORT + ds.subject_id * 2 + ((ds.role == DataSpecifier::ServiceRequest)?0U:1U))};
}


inline tl::expected<DataSpecifier, Error> DataSpecifier::from_packet(
    const MessageAddr& currentAddr, const NetworkAddress& source, const NetworkAddress& dest){
    if(not (\
            source.addr.subnet_id() == currentAddr.subnet_id() \
            and dest.addr.subnet_id() == currentAddr.subnet_id()
    )){
        return -errors::DataSpecifierConversion::WrongSubnet;
    }
    if(dest.addr.isValidMessage()){
        if(dest.port == MULTICAST_PORT){
            return DataSpecifier{
                DataSpecifier::Message,
                source.addr.subject_or_node_id(),
                dest.addr.subject_or_node_id(),
            };
        }
        return -errors::DataSpecifierConversion::WrongPort;
    }
    if (dest.port >= UNICAST_BASE_PORT){
        return DataSpecifier{
            (dest.port % 2 == 0)?(DataSpecifier::ServiceRequest):(DataSpecifier::ServiceResponce),
            source.addr.subject_or_node_id(),
            static_cast<uint16_t>((dest.port - UNICAST_BASE_PORT) / 2U),
        };
    }
    return -errors::DataSpecifierConversion::PortTooSmall;
}

struct TransferMetadata{
    DataSpecifier data;
    uint64_t transfer_id;
    uint8_t priority;
};


using packetRecievedHandler = std::function<tl::expected<void, Error>(const NetworkAddress& source, const NetworkAddress& dest, const uint8_t*const data, size_t size)>;

/**
 * This is a generic udp socket interface for this library. You need to it for your environment.
 */
struct UdpSocketImpl{
    virtual void prepare(const MessageAddr& base_addr, packetRecievedHandler handler)=0;
    // this function MUST copy data into its own buffers
    virtual size_t writeTo(const NetworkAddress& addr, const uint8_t*const data, size_t size)=0;
    virtual void listenUnicast(const NetworkAddress& addr)=0;
    virtual void listenMulticast(const NetworkAddress& subscription)=0;
    virtual ~UdpSocketImpl()=default;
};
using SocketFactory = std::function<std::unique_ptr<UdpSocketImpl>()>;

struct TimerImpl{
    using callback_t = std::function<void()>;
    virtual void startSingleShot(uint32_t millis, callback_t callback) = 0;
    virtual void stop() = 0;
    virtual ~TimerImpl()=default;
};
using TimerFactory = std::function<std::unique_ptr<TimerImpl>()>;


template<uint16_t ni>
constexpr uint16_t fixed_port_id(){ return ni;}

/**
 * @brief The CyphalUdp struct is a network node. It is used to send and recieve Cyphal messages over Cyphal/UDP protocol
 *
 * To use it, you should instantiate it and set base network address to it:
 *
 * ```cpp
 * cyphalpp::CyphalUdp node(socketsImpl, timersImpl);
 * node.setAddr(someAddr);
 * ```
 *
 *
 */
struct CyphalUdp{
private:
    struct Subscription{
        using transferRecieved = std::function<tl::expected<void, Error>(Subscription*const sub, const TransferMetadata& ds, const uint8_t*const data, size_t size)>;
        DataSpecifier ds;
        transferRecieved handler;
        std::unique_ptr<UdpSocketImpl> udp;

        Subscription(DataSpecifier ds_, transferRecieved handler_, std::unique_ptr<UdpSocketImpl> udp_)
            :ds(ds_), handler(std::move(handler_)),  udp(std::move(udp_)){}
    };
    uint8_t buffer[UDP_MAX_MTU]{0};
    SocketFactory socketFactory;
    TimerFactory timerFactory;
    MessageAddr addr_{0,0};
    std::unique_ptr<UdpSocketImpl> sender_udp_{};
    std::vector<std::unique_ptr<Subscription>> subs;
    std::uint64_t transfer_id_{0};
    bool loopback_enabled{false};
public:
    /**
     * @brief CyphalUdp constructor. It creates a node instance
     * @param sFactory - should be a function, creating a new UDP socket instance
     * @param tFactory - should be a function, creating a new timer
     * @param loopback - specifies, whether this node should receive messages from itself
     */
    CyphalUdp(SocketFactory sFactory, TimerFactory tFactory, bool loopback = false):
        socketFactory(std::move(sFactory)), timerFactory(std::move(tFactory)), loopback_enabled(loopback){
        sender_udp_ = socketFactory();
    }
    /**
     * @brief setAddr set base address `addr` for this node
     * @param addr - address of node
     */
    void setAddr(const MessageAddr& addr){
        addr_ = addr;
        sender_udp_->prepare(addr_, [](
                const NetworkAddress&, const NetworkAddress&, const uint8_t*const, size_t
        ) -> tl::expected<void, Error>{
            return -errors::ParsePacket::HeaderWrongProtocolVersion;
        });
    }
    /**
     * @brief subscribeMessage - subscribes to a broadcast message Message **with** fixed port id
     * @tparam Message - one of Nunavut-generated message classes `uavcan::node::Heartbeat_1_0`, for example
     * @tparam F - functor class to call when Message is recieved.
     * @param f - functor instance to call when Message is recieved.
     */
    template<typename Message, typename F>
    void subscribeMessage(F&& f){
        static_assert(Message::HasFixedPortID, "Message should have fixed port id!");
        subscribeMessage<Message>(
            std::forward<F>(f),
            []()->uint16_t {return Message::FixedPortId; }
        );
    }
    /**
     * @brief subscribeMessage - subscribes to a broadcast message Message **without** a fixed port id
     * @tparam Message - one of Nunavut-generated message classes `uavcan::si::unit::length::Scalar_1_0`, for example
     * @tparam PortIdF - functor class to call to determine port id
     * @tparam F - functor class to call when Message is recieved.
     * @param portIdF - functor instance to call to determine port id
     * @param f - functor instance to call when Message is recieved.
     */
    template<typename Message, typename PortIdF, typename F>
    void subscribeMessage(F&& f, PortIdF&& portIdF){
        static_assert(
            std::is_convertible<decltype(portIdF()), uint16_t>::value,
            "Port ID callback should return uint16_t");
        DataSpecifier ds{
            DataSpecifier::Message, 0, portIdF()
        };
        subscribe<Message>(
            ds, [f = std::forward<F>(f)](
                    Subscription*const, const TransferMetadata& ds, const Message& msg
            ) -> tl::expected<void, Error>{
                f(ds, msg); return {};
            });
    }

    /**
     * @brief subscribeServiceRequest - creates a server for a service Service with fixed port id
     * @param f
     */
    template<typename Service, typename F>
    void subscribeServiceRequest(F&& f){
        using Request = typename Service::Request;
        using Response = typename Service::Response;
        static_assert(Request::HasFixedPortID, "Message should have fixed port id!");
        DataSpecifier ds{
            DataSpecifier::ServiceRequest, addr_.subject_or_node_id(), Request::FixedPortId
        };
        subscribe<Request>(
            ds, [this, f = std::forward<F>(f)](
                    Subscription*const sub, const TransferMetadata& tm, const Request& msg
            ) -> tl::expected<void, Error> {
                Response resp = f(tm, msg);
                sendMessage(
                    sub->udp.get(), TransferMetadata{
                        DataSpecifier{DataSpecifier::ServiceResponce, tm.data.node_id, tm.data.subject_id},
                        tm.transfer_id, tm.priority
                    }, resp);
                return {};
        });

    }

    template<typename Message, typename F>
    Subscription* subscribe(DataSpecifier ds, F&& f){
        auto na = NetworkAddress::from_data_specifier(addr_, ds);
        auto sub = std::make_unique<Subscription>(ds, 
            [f = std::forward<F>(f)](Subscription*const sub, const TransferMetadata& ds, const uint8_t*const data, size_t size) mutable -> tl::expected<void, Error> {
                // if(size < Message::EXTENT_BYTES){
                //     return -errors::ParsePacket::MessageTooSmall;
                // }
                Message msg;
                auto res = msg.deserialize({data, size});
                if(not res){
                    return tl::unexpected<Error>{res.error()};
                }
                return f(sub, ds, msg);
            }, nullptr
        );
        sub->udp = createSocket(*sub.get());
        if(na.addr.isValidMessage()){
            sub->udp->listenMulticast(na);
        }else{
            sub->udp->listenUnicast(na);
        }
        subs.emplace_back(std::move(sub));
        return subs.back().get();
    }
    template<typename F>
    Subscription* subscribeRaw(DataSpecifier ds, F&& f){
        auto na = NetworkAddress::from_data_specifier(addr_, ds);
        auto sub = std::make_unique<Subscription>(ds,
            [f = std::forward<F>(f)](Subscription*const sub, const TransferMetadata& ds, const uint8_t*const data, size_t size) mutable -> tl::expected<void, Error> {
                return f(ds, data, size);
            }, nullptr
        );
        sub->udp = createSocket(*sub.get());
        if(na.addr.isValidMessage()){
            sub->udp->listenMulticast(na);
        }else{
            sub->udp->listenUnicast(na);
        }
        subs.emplace_back(std::move(sub));
        return subs.back().get();
    }

    /**
     * @brief sendMessage - publishes message Message with fixed port id
     * @tparam Message - one of Nunavut-generated message classes `uavcan::node::Heartbeat_1_0`, for example
     * @param msg - message to send
     * @param priority - priority of this message
     * @return void if e
     */
    template<typename Message>
    tl::expected<void, Error> sendMessage(const Message& msg, uint8_t priority = cyphal_udp::Header_1_0::PriorityNominal){
        static_assert(Message::HasFixedPortID, "Message should have fixed port id!");
        return sendMessage<Message>(msg, fixed_port_id<Message::FixedPortId>, priority);
    }

    /**
     * @brief sendMessage - publishes message Message with random port id
     * @tparam Message - one of Nunavut-generated message classes `uavcan::si::unit::length::Scalar_1_0`, for example
     * @tparam PortIdF - functor class to call to determine port id
     * @tparam F - functor class to call when Message is recieved.
     * @param msg - message to send
     * @param portIdf - functor instance to call to determine port id
     * @param priority - priority of this message
     * @return
     */
    template<typename Message, typename PortIdF>
    tl::expected<void, Error> sendMessage(const Message& msg, PortIdF&& portIdf, uint8_t priority = cyphal_udp::Header_1_0::PriorityNominal){
        static_assert(
            std::is_convertible<decltype(portIdf()), uint16_t>::value,
            "Port ID callback should return uint16_t");
        return sendMessage<Message>(sender_udp_.get(), TransferMetadata{
            DataSpecifier{DataSpecifier::Message, 0, portIdf()
        },transfer_id_++, priority}, msg);
    }

    template<typename Message>
    tl::expected<void, Error> sendMessage(UdpSocketImpl* sock, const TransferMetadata& ds, const Message& msg){
        auto prepared = prepareFrame<Message>(ds, msg);
        if(not prepared){
            return tl::make_unexpected(prepared.error());
        }
        auto na = NetworkAddress::from_data_specifier(addr_, ds.data);
        auto sentBytes = sock->writeTo(na, buffer, prepared.value());
        if(sentBytes != prepared.value()){
            // Serial.println("Failed to write message");
            return tl::make_unexpected<int8_t>(-1);
        }
        return {};
    }
    tl::expected<void, Error> sendMessage(UdpSocketImpl* sock, const TransferMetadata& ds, const uint8_t*const data, size_t size){
        auto prepared = prepareRawFrame(ds, [this, data, size]()->tl::expected<size_t, Error>{return prepareBuffer(data, size);});
        if(not prepared){
            return tl::make_unexpected(prepared.error());
        }
        auto na = NetworkAddress::from_data_specifier(addr_, ds.data);
        auto sentBytes = sock->writeTo(na, buffer, prepared.value());
        if(sentBytes != prepared.value()){
            // Serial.println("Failed to write message");
            return tl::make_unexpected<int8_t>(-1);
        }
        return {};
    }

    template<typename Service>
    class ServiceCaller{
        using Request = typename Service::Request;
        using Response = typename Service::Response;
        struct Call{
            uint64_t tid;
            std::unique_ptr<TimerImpl> timeout;
        };

        CyphalUdp& self;
        std::unique_ptr<UdpSocketImpl> senderUdp;
        TimerFactory timerFactory;
        TimerImpl::callback_t errorCallback;
        std::vector<Call > calls;

        friend class CyphalUdp;
        ServiceCaller(
            CyphalUdp& _self, 
            std::unique_ptr<UdpSocketImpl> _sub, 
            TimerFactory _timerFactory,
            TimerImpl::callback_t _errorCallback
        )
        :self(_self),senderUdp(std::move(_sub)),timerFactory(std::move(_timerFactory)), errorCallback(std::move(_errorCallback)){
        }
        bool findAndRemove(uint64_t tid){
            auto removed = std::remove_if(calls.begin(), calls.end(), [&tid](const Call& c){ return c.tid == tid; });
            bool ret = removed != calls.end();
            calls.erase(removed, calls.end());
            return ret;
        }
    public:
        /**
         * @brief call - same as operator()
         */
        auto call(uint32_t node_id, const Request& msg, uint32_t timeout_ms=1000, uint8_t priority = cyphal_udp::Header_1_0::PriorityNominal){
            return operator()(node_id, msg, timeout_ms, priority);
        }
        /**
         * @brief operator () - calls service on node_id with request msg
         * @param node_id - id of the target node
         * @param msg - message object to send
         * @param timeout_ms - how much time to wait before request is considered timed out
         * @param priority - priority of the sent message
         * @return result of the message sending process. either void - ok or with cyphalpp::Error on error
         */
        tl::expected<void, Error> operator()(uint32_t node_id, const Request& msg, uint32_t timeout_ms=1000, uint8_t priority = cyphal_udp::Header_1_0::PriorityNominal){
            auto tid = self.transfer_id_++;
            auto ret = self.sendMessage<Request>(senderUdp.get(), TransferMetadata{
                        DataSpecifier{DataSpecifier::ServiceRequest, static_cast<uint16_t>(node_id), Request::FixedPortId
                    },tid, priority}, msg);
            if(ret){
                calls.emplace_back(Call{tid, timerFactory()});
                calls.back().timeout->startSingleShot(timeout_ms, [this, tid](){
                    findAndRemove(tid);
                    errorCallback();
                });
                return {};
            }
            return tl::make_unexpected(ret.error());
        }

        tl::expected<void, Error> serviceResponce(const TransferMetadata& ds){
            if(findAndRemove(ds.transfer_id)){
                return {};
            }else{
                return -errors::ServiceCall::TransferIdOutOfSync;
            }
        }
    };

    /**
     * @brief prepareServiceCalls
     * @param f
     * @param ef
     * @return
     */
    template<typename Service, typename F, typename Ef>
    std::shared_ptr<ServiceCaller<Service>> prepareServiceCalls(F&& f, Ef&& ef){
        using Response = typename Service::Response;
        static_assert(Service::Request::HasFixedPortID, "Request should have fixed port id!");
        static_assert(Service::Response::HasFixedPortID, "Response should have fixed port id!");
        auto senderUdp = socketFactory();
        senderUdp->prepare(addr_, [](
                const NetworkAddress&, const NetworkAddress&, const uint8_t*const, size_t
        ) -> tl::expected<void, Error>{
            return -errors::ParsePacket::HeaderWrongProtocolVersion;
        });

        std::shared_ptr<ServiceCaller<Service>> ret{new ServiceCaller<Service>(
            *this, std::move(senderUdp), timerFactory, TimerImpl::callback_t(std::forward<Ef>(ef)))};
        subscribe<Response>(
            DataSpecifier{DataSpecifier::ServiceResponce, addr_.subject_or_node_id(), Response::FixedPortId},
            [caller = ret, f = std::forward<F>(f)](
                    Subscription* const, const TransferMetadata& ds, const Response& msg
            ) -> tl::expected<void, Error>{
                auto err = caller->serviceResponce(ds);
                if(err){
                    f(ds, msg);
                    return {};
                }else{
                    return tl::make_unexpected(err.error());
                }
            });
        
        return ret;
    }
private:
    std::unique_ptr<UdpSocketImpl> createSocket(Subscription& sub){
        auto udp = socketFactory();
        udp->prepare(addr_, [this, &sub](
                const NetworkAddress& source, const NetworkAddress& dest, 
                const uint8_t*const data, size_t size
        )  -> tl::expected<void, Error> {
            if(size < cyphal_udp::Header_1_0::EXTENT_BYTES){
                return -errors::ParsePacket::MessageTooSmall;
            }
            if(source.addr == addr_ and not loopback_enabled){
                return {};
            }
            TransferMetadata tm;
            {
                auto maybe_ds = DataSpecifier::from_packet(addr_, source, dest);
                if(not maybe_ds){
                    return tl::unexpected<Error>(maybe_ds.error());
                }
                tm.data = maybe_ds.value();
            }
            if(sub.ds.role != tm.data.role){
                return -errors::ParsePacket::RolesMismatch;
            }
            if(sub.ds.subject_id != tm.data.subject_id){
                return -errors::ParsePacket::SubjectIdMismatch;
            }
            cyphal_udp::Header_1_0 header;
            auto headerResult = header.deserialize({data, size});
            if(not headerResult){
                return tl::unexpected<Error>(headerResult.error());
            }
            if(header.version != 0 ){
                return -errors::ParsePacket::HeaderWrongProtocolVersion;
            }
            tm.transfer_id = header.transfer_id;
            tm.priority = header.priority;
            if(header.frame_index_eot != cyphal_udp::Header_1_0::EndOfTransmission){
                return -errors::ParsePacket::MultiframeTransfersNotSupported;
            }
            return sub.handler(&sub, tm, data + cyphal_udp::Header_1_0::EXTENT_BYTES, size - cyphal_udp::Header_1_0::EXTENT_BYTES);
        });
        return udp;
    }

    template<typename Message>
    tl::expected<size_t, Error> prepareMessage(const Message& msg){
        constexpr static auto msgSize = cyphal_udp::Header_1_0::EXTENT_BYTES + Message::EXTENT_BYTES;
        static_assert(msgSize < UDP_MAX_MTU, "Multiframe transfers not implemented!");
        auto res = msg.serialize({ buffer + cyphal_udp::Header_1_0::EXTENT_BYTES,  Message::EXTENT_BYTES });
        if(not res){
            return tl::unexpected<Error>(res.error());
        }
        return res.value();
    }


    tl::expected<size_t, Error> prepareBuffer(const uint8_t*const data, size_t size){
        auto msgSize = cyphal_udp::Header_1_0::EXTENT_BYTES + size;
        if(msgSize < UDP_MAX_MTU){
            return -errors::ParsePacket::MultiframeTransfersNotSupported;
        }
        std::copy_n(data, size, buffer + cyphal_udp::Header_1_0::EXTENT_BYTES);
        return size;
    }


    template<typename Message>
    tl::expected<size_t, Error> prepareFrame(const TransferMetadata& tm, const Message& f){
        return prepareRawFrame(tm, [this, f]()-> tl::expected<size_t, Error>{return prepareMessage(f);});
    }

    template<typename F>
    tl::expected<size_t, Error> prepareRawFrame(const TransferMetadata& tm, F&& f){
        cyphal_udp::Header_1_0 header;
        header.version = 0;
        header.priority = tm.priority;
        header.frame_index_eot = cyphal_udp::Header_1_0::EndOfTransmission;
        header.transfer_id = tm.transfer_id;
        auto res = header.serialize({ buffer,  cyphal_udp::Header_1_0::EXTENT_BYTES });
        if(not res){
            return tl::unexpected<Error>(res.error());
        }
        auto msg = f();
        if(not msg){
            return msg;
        }
        return cyphal_udp::Header_1_0::EXTENT_BYTES + msg.value();
    }
};


} // namespace cyphalpp
#endif // ndef UAVCAN_ASYNC_UDP_HPP
