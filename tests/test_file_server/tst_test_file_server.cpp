//
// Copyright © 2022 JSC "VNIIZHT" or its affiliates. All Rights Reserved.
// This software is distributed under the terms of the MIT License.
//

#include <QtTest>
#include <qt_cyphal_udp.hpp>
#include <fileserver.hpp>



class test_file_server : public QObject
{
    Q_OBJECT
    uint32_t lhbase;
public:
    test_file_server();
    ~test_file_server();

private slots:
    void test_file_server_contructs();

};

test_file_server::test_file_server()
{
    QHostAddress lha(QHostAddress::LocalHost);
    lhbase = (lha.toIPv4Address() & static_cast<uint32_t>(~0xFFFFFFU)) | (0x100 << 16);
}

test_file_server::~test_file_server()
{

}

void test_file_server::test_file_server_contructs()
{
    cyphalpp::CyphalUdp cy(lhbase | 0x1U, cyphalpp::qt::qCyphalUdpSocket, cyphalpp::qt::qCyphalTimer);
    cyphalpp::qt::utils::FileServer fs(cy);


    cyphalpp::CyphalUdp cy2(lhbase | 0x2U, cyphalpp::qt::qCyphalUdpSocket, cyphalpp::qt::qCyphalTimer);

}

QTEST_MAIN(test_file_server)

#include "tst_test_file_server.moc"
