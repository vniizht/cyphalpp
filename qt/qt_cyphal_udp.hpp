//
// Copyright © 2022 JSC "VNIIZHT" or its affiliates. All Rights Reserved.
// This software is distributed under the terms of the MIT License.
//

#ifndef QT_UDP_UAVCAN_HPP_INCLUDED
#define QT_UDP_UAVCAN_HPP_INCLUDED
#include "cyphalpp.hpp"
#include <QDebug>

namespace cyphalpp{
QDebug operator<<(QDebug ret, const cyphalpp::Error& e);
QDebug operator<<(QDebug ret,const cyphalpp::errors::DataSpecifierConversion& c);
QDebug operator<<(QDebug ret,const nunavut::support::Error& c);
QDebug operator<<(QDebug ret,const cyphalpp::errors::ParsePacket& c);

namespace qt{
auto qCyphalUdpSocket() -> std::unique_ptr<cyphalpp::UdpSocketImpl>;
auto qCyphalTimer() -> std::unique_ptr<cyphalpp::TimerImpl>;
} // namespace qt
} // namespace cyphalpp
#endif // ndef QT_UDP_UAVCAN_HPP_INCLUDED
