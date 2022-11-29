//
// Copyright © 2022 JSC "VNIIZHT" or its affiliates. All Rights Reserved.
// This software is distributed under the terms of the MIT License.
//

#include <QtTest>
#include <QCoreApplication>

#include <nodeshealthmodel.h>
#include <qt_cyphal_udp.hpp>
#include <cyphalnode.hpp>

#include <QAbstractItemModelTester>
#include <uavcan/node/GetInfo_1_0.qt.hpp>

class node_health : public QObject
{
    Q_OBJECT
    uint32_t lhbase;
public:
    node_health();
    ~node_health();

private slots:
    void test_model();

};

node_health::node_health()
{
    QHostAddress lha(QHostAddress::LocalHost);
    lhbase = (lha.toIPv4Address() & static_cast<uint32_t>(~0xFFFFFFU)) | (0x200 << 16);
}

node_health::~node_health()
{
}

void node_health::test_model()
{
    using GetInfo = uavcan::node::GetInfo::Service_1_0;
    
    cyphalpp::CyphalUdp cy(cyphalpp::qt::qCyphalUdpSocket, cyphalpp::qt::qCyphalTimer);
    cy.setAddr(lhbase | 0x1U);
    cyphalpp::HeartbeatPublish<> hb(cy, cyphalpp::qt::qCyphalTimer());
    auto model = new cyphalpp::qt::utils::NodesHealthModel(cy, this);
    new QAbstractItemModelTester(model, QAbstractItemModelTester::FailureReportingMode::Fatal, this);

    cyphalpp::CyphalUdp cyNode2(cyphalpp::qt::qCyphalUdpSocket, cyphalpp::qt::qCyphalTimer);
    cyNode2.setAddr(lhbase | 0x2U);
    cyphalpp::HeartbeatPublish<> hbN2(cyNode2, cyphalpp::qt::qCyphalTimer());
    cyNode2.subscribeServiceRequest<GetInfo>([](const cyphalpp::TransferMetadata&, const GetInfo::Request&) -> GetInfo::Response{
        GetInfo::Response resp;
        constexpr static uint8_t name[]="ds";
        constexpr static size_t nameSize = sizeof(name) - 1; // zero byte at end
        resp.name.reserve(nameSize);
        std::copy_n(name, nameSize, std::back_inserter(resp.name));
        resp.protocol_version.major = 1;
        resp.protocol_version.minor = 0;
        resp.software_version.major = 2;
        resp.software_version.minor = 3;
        resp.hardware_version.major = 4;
        resp.hardware_version.minor = 5;
        return resp;
    });
    {
        auto r = QTest::qWaitFor([&model]()->bool{
            return model->rowCount() == 1;
        });
        QVERIFY(r);
    }
    QCOMPARE(model->index(0,0).data(), QStringLiteral("2"));
}

QTEST_MAIN(node_health)

#include "tst_node_health.moc"
