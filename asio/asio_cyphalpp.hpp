//
// Copyright © 2022 JSC "VNIIZHT" or its affiliates. All Rights Reserved.
// This software is distributed under the terms of the MIT License.
//

#include "cyphalpp.hpp"
#include <tl/optional.hpp>
#include <boost/array.hpp>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/asio.hpp>
#include <iostream>

namespace cyphalpp{
namespace asio{
using namespace boost::asio;
using boost::asio::ip::address_v4;
using boost::asio::ip::udp;

std::ostream& operator<<(std::ostream&  ret, const Error &e){
    ret << "E{";
    mpark::visit([&ret](auto v){ret<< v;}, e);
    ret << "}";
    return ret;
}

std::ostream& operator<<(std::ostream&  ret, const errors::DataSpecifierConversion &c) {
    ret << "DSC::";
    switch (c)
    {
    case cyphalpp::errors::DataSpecifierConversion::WrongSubnet: ret <<"WrongSubnet"; break;
    case cyphalpp::errors::DataSpecifierConversion::WrongPort: ret <<"WrongPort"; break;
    case cyphalpp::errors::DataSpecifierConversion::PortTooSmall: ret <<"PortTooSmall"; break;
    default:
        break;
    }
    return ret;
}

std::ostream& operator<<(std::ostream&  ret, const nunavut::support::Error &c) {
    ret << "nunavut::";
    switch (c)
    {
    case nunavut::support::Error::SERIALIZATION_INVALID_ARGUMENT: ret <<"SERIALIZATION_INVALID_ARGUMENT"; break;
    case nunavut::support::Error::SERIALIZATION_BUFFER_TOO_SMALL: ret <<"SERIALIZATION_BUFFER_TOO_SMALL"; break;
    case nunavut::support::Error::REPRESENTATION_BAD_ARRAY_LENGTH: ret <<"REPRESENTATION_BAD_ARRAY_LENGTH"; break;
    case nunavut::support::Error::REPRESENTATION_BAD_UNION_TAG: ret <<"REPRESENTATION_BAD_UNION_TAG"; break;
    case nunavut::support::Error::REPRESENTATION_BAD_DELIMITER_HEADER: ret <<"REPRESENTATION_BAD_DELIMITER_HEADER"; break;
    default:
        break;
    }
    return ret;
}

std::ostream& operator<<(std::ostream&  ret, const errors::ParsePacket &c) {
    ret << "ParsePacket::";
    switch (c)
    {
    case cyphalpp::errors::ParsePacket::MessageTooSmall: ret <<"MessageTooSmall"; break;
    case cyphalpp::errors::ParsePacket::RolesMismatch: ret <<"RolesMismatch"; break;
    case cyphalpp::errors::ParsePacket::SubjectIdMismatch: ret <<"SubjectIdMismatch"; break;
    case cyphalpp::errors::ParsePacket::HeaderWrongProtocolVersion: ret <<"HeaderWrongProtocolVersion"; break;
    case cyphalpp::errors::ParsePacket::MultiframeTransfersNotSupported: ret <<"MultiframeTransfersNotSupported"; break;
    default:
        break;
    }
    return ret;
}

std::ostream& operator<<(std::ostream&  ret, const errors::ServiceCall &c) {
    ret << "ServiceCall::";
    switch (c) {
    case errors::ServiceCall::TransferIdOutOfSync: ret << "TransferIdOutOfSync"; break;
    default: break;
    }
    return ret;
}


class AsioUdpSocket final: public UdpSocketImpl{
    cyphalpp::packetRecievedHandler handler_{};
    udp::socket socket_;
    udp::endpoint sender_endpoint_{};
    MessageAddr base_{0};
    uint8_t data_[UDP_MAX_MTU]{};
    bool write_bound_{false};
public:
    AsioUdpSocket(io_service& service):UdpSocketImpl(), socket_(service, udp::v4()){
    }
    ~AsioUdpSocket() override;
    virtual void prepare(const MessageAddr&  base, packetRecievedHandler handler) override{
        handler_ = handler;
        base_ = base;
    }

    // this function MUST copy data into its own buffers
    virtual size_t writeTo(const NetworkAddress& addr, const uint8_t*const data, size_t size)override{
        if(not write_bound_){
            auto base_v4 =ip::make_address_v4(base_);
            socket_.bind(udp::endpoint(base_v4, 0));
            socket_.set_option(ip::multicast::outbound_interface(base_v4));
            socket_.set_option(ip::multicast::enable_loopback(false));
            write_bound_ = true;
        }
        return socket_.send_to(buffer(data, size), udp::endpoint(ip::make_address_v4(addr.addr), addr.port));
    }
    virtual void listenUnicast(const NetworkAddress& addr)override{
        udp::endpoint listen_endpoint(ip::make_address_v4(addr.addr), addr.port);
        socket_.set_option(ip::multicast::enable_loopback(false));
        socket_.set_option(udp::socket::reuse_address(true));
        socket_.bind(listen_endpoint);
        write_bound_ = true;
        restartReceiving();
    }
    virtual void listenMulticast(const NetworkAddress& addr) override{
        auto base_addr_v4 = ip::make_address_v4(base_);
        auto addr_v4 = ip::make_address_v4(addr.addr);
        socket_.set_option(udp::socket::reuse_address(true));
        udp::endpoint listen_endpoint(addr_v4, addr.port);
        socket_.bind(listen_endpoint);
        socket_.set_option(ip::multicast::enable_loopback(false));
        write_bound_ = true;
        // Join the multicast group.
        socket_.set_option(ip::multicast::join_group(addr_v4, base_addr_v4));
        restartReceiving();
    }
private:
    void restartReceiving(){
        socket_.async_receive_from(
            boost::asio::buffer(data_, UDP_MAX_MTU), sender_endpoint_,
            boost::bind(
                &AsioUdpSocket::handle_receive_from, this,
                boost::asio::placeholders::error,
                boost::asio::placeholders::bytes_transferred));
    }
    void handle_receive_from(const boost::system::error_code& error, size_t bytes_recvd)
    {
        if(error){
            std::clog << "Error receive" << error;
            return;
        }
        if(handler_ == nullptr) return;

        cyphalpp::NetworkAddress senderAddr{
            cyphalpp::MessageAddr(sender_endpoint_.address().to_v4().to_uint()),
            static_cast<uint16_t>(sender_endpoint_.port())
        };
        auto local = socket_.local_endpoint();
        cyphalpp::NetworkAddress recieverAddr{
            cyphalpp::MessageAddr(local.address().to_v4().to_uint()),
            static_cast<uint16_t>(local.port())
        };
        handler_(senderAddr, recieverAddr, data_, bytes_recvd);
        restartReceiving();
    }
};
AsioUdpSocket::~AsioUdpSocket(){}

SocketFactory asioUdpSocket(io_service& service){
    return [&service]()-> std::unique_ptr<UdpSocketImpl>{
        return std::make_unique<AsioUdpSocket>(service);
    };
}

class AsioTimer final: public TimerImpl{
    io_service& service_;
    tl::optional<boost::asio::steady_timer> timer_;
public:
    AsioTimer(io_service& service): TimerImpl(), service_(service){
    }
    virtual void startSingleShot(uint32_t millis, callback_t callback) override{
        boost::asio::steady_timer::duration interval(std::chrono::milliseconds{millis});
        timer_.emplace(service_, interval);
        timer_->async_wait([callback= std::move(callback)](const boost::system::error_code& error){
            if(error) return;
            callback();
        });
    }
    virtual void stop() override{
        timer_.reset();
    }
    ~AsioTimer() override;
};

AsioTimer::~AsioTimer(){}

TimerFactory asioTimer(io_service& service){
    return [&service]()-> std::unique_ptr<TimerImpl>{
        return std::make_unique<AsioTimer>(service);
    };
}
} // namespace asio
} // namespace cyphalpp
