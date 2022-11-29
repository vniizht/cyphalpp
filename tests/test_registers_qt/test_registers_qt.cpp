//
// Copyright © 2022 JSC "VNIIZHT" or its affiliates. All Rights Reserved.
// This software is distributed under the terms of the MIT License.
//

#include <QtTest>
#include <qt_cyphal_udp.hpp>
#include <noderegisters.h>
#include <cyphalnode.hpp>
#include <qt_cyphal_registry.hpp>


class test_registers_qt : public QObject
{
    Q_OBJECT
    uint32_t lhbase;
public:
    test_registers_qt();
    ~test_registers_qt();

private slots:
    void test_registers_qt_contructs();

};

test_registers_qt::test_registers_qt()
{
    QHostAddress lha(QHostAddress::LocalHost);
    lhbase = (lha.toIPv4Address() & static_cast<uint32_t>(~0xFFFFFFU)) | (0x300 << 16);
}

test_registers_qt::~test_registers_qt()
{

}

void test_registers_qt::test_registers_qt_contructs()
{
    
    cyphalpp::CyphalUdp cy(cyphalpp::qt::qCyphalUdpSocket, cyphalpp::qt::qCyphalTimer);
    cy.setAddr(lhbase | 0x1U);
    
    using namespace cyphalpp::registry::types;
    cyphalpp::registry::Registry registry(cy, nullptr);

    registry["as"] = U32(120);
    
    
    cyphalpp::CyphalUdp cy2(cyphalpp::qt::qCyphalUdpSocket, cyphalpp::qt::qCyphalTimer);
    cy2.setAddr(lhbase | 0x2U);
    cyphalpp::qt::utils::NodeRegisters reader(cy2, 0x1U);

}

QTEST_MAIN(test_registers_qt)

#include "test_registers_qt.moc"
