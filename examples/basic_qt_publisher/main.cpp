#include <QCoreApplication>
#include <cyphalpp.hpp>
#include <cyphalnode.hpp>
#include <qt_cyphal_udp.hpp>
#include <QHostAddress>
#include <QTimer>

// qt-specific transpiled dsdl with qDebug output functions
#include <uavcan/node/Heartbeat_1_0.qt.hpp>
#include <uavcan/node/ExecuteCommand_1_1.qt.hpp>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    auto args = a.arguments();
    if(args.size() != 2){
        qFatal("Wrong number of arguments, expected 2, got %d", args.size());
    }
    QHostAddress addr(args[1]);
    if(addr.isNull()){
        qFatal("Couldn't convert first argument to IP address");
    }

    // creates a node instance
    cyphalpp::CyphalUdp cy(cyphalpp::qt::qCyphalUdpSocket,cyphalpp::qt::qCyphalTimer);
    cy.setAddr( addr.toIPv4Address() );

    // creates a heartbeat publisher
    cyphalpp::HeartbeatPublish<> hb(cy, cyphalpp::qt::qCyphalTimer());

    cy.subscribeMessage<uavcan::node::Heartbeat_1_0>([](const cyphalpp::TransferMetadata& md, const uavcan::node::Heartbeat_1_0& hb){
        qDebug() << "Heartbeat recieved from " << md.data.node_id << hb;
    });
    using ExecuteCommand = uavcan::node::ExecuteCommand::Service_1_1;
    cy.subscribeServiceRequest<ExecuteCommand>([](const cyphalpp::TransferMetadata& /*md*/, const ExecuteCommand::Request& req)->ExecuteCommand::Response{
        if(req.command == ExecuteCommand::Request::COMMAND_POWER_OFF){
            auto t = new QTimer();
            t->setSingleShot(true);
            QObject::connect(t, &QTimer::timeout, t, &QTimer::deleteLater);
            QObject::connect(t, &QTimer::timeout, qApp, &QCoreApplication::quit);
            t->start(10);
        }
        return ExecuteCommand::Response{
            .status = ExecuteCommand::Response::STATUS_SUCCESS
        };
    });
    return a.exec();
}
