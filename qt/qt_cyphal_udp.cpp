//
// Copyright © 2022 JSC "VNIIZHT" or its affiliates. All Rights Reserved.
// This software is distributed under the terms of the MIT License.
//

#include <array>
#include "qt_cyphal_udp.hpp"
#include "cyphalpp.hpp"
#include <QUdpSocket>
#include <QNetworkDatagram>
#include <QNetworkInterface>
#include <QTimer>


namespace cyphalpp{

namespace qt {
class QCyphalUdpSocket final: public QUdpSocket, public UdpSocketImpl{
    Q_OBJECT
    MessageAddr base_{0};
public:
    QCyphalUdpSocket(QObject* parent=nullptr):QUdpSocket(parent), UdpSocketImpl() {
    }
    ~QCyphalUdpSocket() override;
    virtual void prepare(const MessageAddr& base, cyphalpp::packetRecievedHandler handler) override{
        base_ = base;
        connect(this, &QUdpSocket::readyRead, [this, handler = std::move(handler)](){
            std::vector<QNetworkDatagram> rx;
            while(hasPendingDatagrams()){
                rx.emplace_back(receiveDatagram());
            }
            for(auto& dg: rx){
                cyphalpp::NetworkAddress senderAddr{
                    cyphalpp::MessageAddr(dg.senderAddress().toIPv4Address()),
                            static_cast<uint16_t>(dg.senderPort())
                };
                cyphalpp::NetworkAddress recieverAddr{
                    cyphalpp::MessageAddr(dg.destinationAddress().toIPv4Address()),
                            static_cast<uint16_t>(dg.destinationPort())
                };
                auto data = dg.data();
                auto error = handler(senderAddr, recieverAddr, reinterpret_cast<uint8_t*>(data.data()), data.size());
                if(not error){
                    qWarning() << "Error parsing packet" << error.error();
                }
            }
        });
    }
    static uint32_t getSubnet(const QHostAddress& a){
        return cyphalpp::MessageAddr(a.toIPv4Address()).subnet_id();
    }
    static QNetworkInterface findBindInterface(const QHostAddress& a){
        auto addrSubnet = getSubnet(a);
        const auto ifaces = QNetworkInterface::allInterfaces();
        for(const QNetworkInterface &iface: ifaces) {
            const auto entries = iface.addressEntries();
            for(const QNetworkAddressEntry &addressEntry: entries) {
                auto address = addressEntry.ip();
                if (address.protocol() == QAbstractSocket::IPv4Protocol) {
                    if(addrSubnet == getSubnet(address)){
                        return iface;
                    }
                }
            }
        }
        return {};
    }
    // this function MUST copy data into its own buffers
    virtual size_t writeTo(const cyphalpp::NetworkAddress& addr, const uint8_t*const data, size_t size)override{
        if(state() == QAbstractSocket::UnconnectedState){
            bind(QHostAddress(base_));
        }
        QNetworkDatagram dg(
            QByteArray(reinterpret_cast<const char*>(data), static_cast<int>(size)),
            QHostAddress(addr.addr), addr.port
        );
        return writeDatagram(dg);
    }
    void rebindUnicast(const cyphalpp::NetworkAddress& addr){
        close();
        QTimer::singleShot(1000, [this, addr]{
            listenUnicast(addr);
        });
    }
    void rebindMulticast(const cyphalpp::NetworkAddress& addr){
        close();
        QTimer::singleShot(1000, [this, addr]{
            listenMulticast(addr);
        });
    }
    virtual void listenUnicast(const cyphalpp::NetworkAddress& addr)override{
        QHostAddress a(static_cast<uint32_t>(addr.addr));
        if(not bind(a, addr.port, QUdpSocket::ShareAddress)){
            qWarning() << errorString();
            rebindUnicast(addr);
        }
    }
    virtual void listenMulticast(const cyphalpp::NetworkAddress& addr)override{
        QHostAddress a(static_cast<uint32_t>(addr.addr));
        auto bi = findBindInterface(a);
        if(not bi.isValid()){
            qWarning() << "Failed to listen on multicast address" << a << " - interface with subnet " << addr.addr.subnet_id() << " not found!";
            rebindMulticast(addr);
            return;
        }
        if(not bind(a, addr.port, QUdpSocket::ShareAddress)){
            qWarning() << errorString();
            rebindMulticast(addr);
            return;
        }
        joinMulticastGroup(a, bi);
    }

};


class QCyphalTimer final: QTimer, public TimerImpl{
    Q_OBJECT
    QMetaObject::Connection connection{};
public:
    QCyphalTimer(): QTimer(), TimerImpl(){
    }
    virtual void startSingleShot(uint32_t millis, callback_t callback) override{
        if(connection){
            QTimer::disconnect(connection);
        }
        connection = QTimer::connect(this, &QTimer::timeout, callback);
        QTimer::setSingleShot(true);
        QTimer::start(millis);
    }
    virtual void stop() override{
        QTimer::stop();
    }
    ~QCyphalTimer() override;
};

QCyphalTimer::~QCyphalTimer(){}
QCyphalUdpSocket::~QCyphalUdpSocket(){}

std::unique_ptr<UdpSocketImpl> qCyphalUdpSocket(){return std::make_unique<QCyphalUdpSocket>();}
std::unique_ptr<TimerImpl> qCyphalTimer(){ return std::make_unique<QCyphalTimer>(); }

} // namespace qt


QDebug operator<<(QDebug ret, const errors::DataSpecifierConversion &c) {
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

QDebug operator<<(QDebug ret, const nunavut::support::Error &c) {
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

QDebug operator<<(QDebug ret, const errors::ParsePacket &c) {
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

QDebug operator<<(QDebug ret, const errors::ServiceCall &c) {
    ret << "ServiceCall::";
    switch (c) {
    case errors::ServiceCall::TransferIdOutOfSync: ret << "TransferIdOutOfSync"; break;
    default: break;
    }
    return ret;
}

QDebug operator<<(QDebug ret, const Error &e){
    ret << "E{";
    mpark::visit([&ret](auto v)
    {
        ret<< v;
    },
    e);
    ret << "}";
    return ret;
}

} // namespace cyphalpp

#include "qt_cyphal_udp.moc"
