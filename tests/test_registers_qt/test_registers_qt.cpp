//
// Copyright © 2022 JSC "VNIIZHT" or its affiliates. All Rights Reserved.
// This software is distributed under the terms of the MIT License.
//

#include <QtTest>
#include <qt_cyphal_udp.hpp>
#include <noderegisters.h>
#include <cyphalppheartbeatpublish.hpp>
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
    void test_load_store();

};

test_registers_qt::test_registers_qt()
{
    QHostAddress lha(QHostAddress::LocalHost);
    lhbase = (lha.toIPv4Address() & static_cast<uint32_t>(~0xFFFFFFU)) | (0x00000);
}

test_registers_qt::~test_registers_qt()
{

}

void test_registers_qt::test_registers_qt_contructs()
{
    QTemporaryFile f{};
    QVERIFY(f.open());
    
    cyphalpp::registry::Registry registry(cyphalpp::registry::QSettingsRegistry::open(f.fileName(), QSettings::Format::IniFormat));
    cyphalpp::CyphalUdp cy(lhbase | 0x1U, cyphalpp::qt::qCyphalUdpSocket, cyphalpp::qt::qCyphalTimer);
    registry.setNode(cy);


    using namespace cyphalpp::registry::types;
    registry["key-set-through-opeq"] = U32(120);
    
    
    cyphalpp::CyphalUdp cy2(lhbase | 0x2U, cyphalpp::qt::qCyphalUdpSocket, cyphalpp::qt::qCyphalTimer);
    cyphalpp::qt::utils::NodeRegisters reader(cy2, 0x1U);
    {
        auto r = QTest::qWaitFor([&reader]()->bool{
            return reader.rowCount() >= 1 and reader.index(0,1).data().toString() != "<<EMPTY>>";
        }, 6000);
        QVERIFY(r);
    }
    QCOMPARE(reader.rowCount(), 1);
    QCOMPARE(reader.index(0,0).data(), QStringLiteral("key-set-through-opeq"));
    QCOMPARE(reader.index(0,1).data(), QStringLiteral("[ 120 ]"));
    QCOMPARE(reader.index(0,2).data(), QStringLiteral("YES"));
    QCOMPARE(reader.index(0,3).data(), QStringLiteral("YES"));
    registry.removeNode();
}

void test_registers_qt::test_load_store()
{
    QTemporaryFile f{};
    QVERIFY(f.open());
    f.close();
    f.setAutoRemove(true);
    using namespace cyphalpp::registry::types;
    {
        cyphalpp::registry::Registry registry( cyphalpp::registry::QSettingsRegistry::open(f.fileName(), QSettings::Format::IniFormat));
        cyphalpp::CyphalUdp cy(lhbase | 0x1U, cyphalpp::qt::qCyphalUdpSocket, cyphalpp::qt::qCyphalTimer);
        registry.setNode(cy);
        registry.setDefault(N("key-set-through-opeq"), U32(0));
        registry.setDefault(N("key-with-setdefault"), U32(10));
        {
            auto reg = registry["key-set-through-opeq"];
            auto ints = reg.value().get_natural32_if();
            QVERIFY(ints);
            QCOMPARE(ints->value.at(0), 0U);
            
            QCOMPARE(registry["key-with-setdefault"].as<Natural32>()->value.at(0), 10U);
            
        }
    
        registry["key-set-through-opeq"] = U32(120);
        QCOMPARE(registry["key-set-through-opeq"].as<Natural32>()->value.at(0), 120U);
        
        registry.save();
        registry.removeNode();
    }
    {
        cyphalpp::registry::Registry registry(cyphalpp::registry::QSettingsRegistry::open(f.fileName(), QSettings::Format::IniFormat));
        cyphalpp::CyphalUdp cy(lhbase | 0x1U, cyphalpp::qt::qCyphalUdpSocket, cyphalpp::qt::qCyphalTimer);
        registry.setNode(cy);
        registry.setDefault(N("key-set-through-opeq"), U32(0));
        registry.setDefault(N("key-with-setdefault"), U32(0));
        registry.setDefault(N("another-unset-key"), U32(0));
    
        auto reg = registry["key-set-through-opeq"];
        auto ints = reg.value().get_natural32_if();
        QVERIFY(ints);
        QCOMPARE(ints->value.at(0), 120U);
        QCOMPARE(registry["key-with-setdefault"].as<Natural32>()->value.at(0), 10U);
        QCOMPARE(registry["another-unset-key"].as<Natural32>()->value.at(0), 0U);
        registry.removeNode();
    }
}

QTEST_MAIN(test_registers_qt)

#include "test_registers_qt.moc"
