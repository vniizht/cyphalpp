//#define PYBIND11_DETAILED_ERROR_MESSAGES
#include <iostream>
#include <pybind11/pybind11.h>
#include <pybind11/operators.h>
#include <cyphalpp.hpp>
#include <sys/socket.h>
#include <netinet/in.h>
#include <queue>

namespace py = pybind11;
using namespace py::literals;
using namespace cyphalpp;

template<typename F>
void destructor_helper(const char* func, F&& f){
    try {
        f();
    } catch (py::error_already_set &e) {
        // error_context should be information about where/why the occurred,
        // e.g. use __func__ to get the name of the current function
        e.discard_as_unraisable(func);
    }
}
struct ResultVisitor{
    template<typename T>
    void operator()(const T& e){
        throw pybind11::value_error(std::string(py::repr(py::cast(e))));
    }
};

template<typename T>
T result(tl::expected<T, cyphalpp::Error>&& r){
    if(r){
        return std::move(std::move(r).value());
    }
    
    mpark::visit(ResultVisitor{}, r.error());
}


void result(tl::expected<void, cyphalpp::Error>&& r){
    if(r){
        return;
    }
    mpark::visit(ResultVisitor{}, r.error());
}


struct D{
    const char* f;
    D(const char* f_): f(f_){
        if(log()){ std::cout << "<<<" << f << std::endl; }
    }
    ~D(){ 
        if(log()){ std::cout << ">>>" << f << std::endl; }
    }
    static bool log(){
        static bool ret = getenv("DBG") != nullptr;
        return ret;
    }
};
#define DBG D d{__PRETTY_FUNCTION__};

namespace asyncio{
    py::object& mod(){
        DBG
        static auto ret = new py::object(py::module_::import("asyncio"));
        return *ret;
    }
    py::object get_event_loop(){
        DBG
        static auto py_get_event_loop = new py::function(mod().attr("get_event_loop"));
        return (*py_get_event_loop)();
    }
}

namespace ipaddress{
    py::object& mod(){
        py::gil_scoped_acquire gil;
        static auto ret = new py::object(py::module_::import("ipaddress"));
        return *ret;
    }

    py::object IPv4Address_cls(){
        py::gil_scoped_acquire gil;
        static auto cls = new py::function(mod().attr("IPv4Address"));
        return *cls;
    }

    py::object IPv4Address(uint32_t a){
        return IPv4Address_cls()(a);
    }
}


struct PyAsyncioUdpSocket: public UdpSocketImpl{
    struct Msg{
        std::string data;
        NetworkAddress addr;
        Msg(std::string data_, NetworkAddress addr_):data(std::move(data_)), addr(addr_){}
    };
    MessageAddr base_addr_{0};
    MessageAddr mcast_addr_{0};
    packetRecievedHandler handler_{nullptr};
    py::object transport_{py::none()};
    NetworkAddress local_address_{{0},0};
    std::queue<Msg> tx_queue_{};
    bool write_bound_{false};

    PyAsyncioUdpSocket(){
        DBG
    }
    void send_all_from_queue(){
        DBG
        if(transport_.is(py::none())){
            return;
        }
        while(not tx_queue_.empty()){
            auto& msg = tx_queue_.front();
            auto addr = py::make_tuple( py::str(ipaddress::IPv4Address(msg.addr.addr)), msg.addr.port);
            py::print("Sending message to addr", addr);
            transport_.attr("sendto")(
                py::bytes(msg.data),
                addr);
            tx_queue_.pop();
        }
    }
    virtual void prepare(const MessageAddr& base_addr, packetRecievedHandler handler){
        DBG
        base_addr_ = base_addr;
        handler_ = handler;
    }
    virtual size_t writeTo(const NetworkAddress& addr, const uint8_t*const data, size_t size){
        DBG
        if(not write_bound_){
            auto loop = asyncio::get_event_loop();
            auto create_datagram_endpoint = loop.attr("create_datagram_endpoint");
            auto base_v4 = py::str(ipaddress::IPv4Address(base_addr_));
            auto res = create_datagram_endpoint(
                py::cpp_function([this]() -> PyAsyncioUdpSocket* {return this;}, py::return_value_policy::reference),
                "local_addr"_a=py::make_tuple(base_v4, 0));
            write_bound_ = true;
            loop.attr("create_task")(res);
        }
        if(transport_.is(py::none())){
            tx_queue_.emplace(std::string{reinterpret_cast<const char*>(data), size}, addr);
        }else{
            transport_.attr("sendto")(
                py::bytearray(py::memoryview::from_memory(data, size)),
                py::make_tuple(py::str(ipaddress::IPv4Address(addr.addr)), addr.port));
        }
        return size;
    }
    virtual void listenUnicast(const NetworkAddress& addr){
        DBG
        auto loop = asyncio::get_event_loop();
        auto create_datagram_endpoint = loop.attr("create_datagram_endpoint");
        auto base_v4 = py::str(ipaddress::IPv4Address(base_addr_));
        auto res = create_datagram_endpoint(
            py::cpp_function([this]() -> PyAsyncioUdpSocket* {return this;}, py::return_value_policy::reference),
            "local_addr"_a=py::make_tuple(base_v4, addr.port));
        write_bound_ = true;
        loop.attr("create_task")(res);
    }
    virtual void listenMulticast(const NetworkAddress& subscription){
        DBG
        auto loop = asyncio::get_event_loop();
        auto create_datagram_endpoint = loop.attr("create_datagram_endpoint");
        auto base_v4 = py::str(ipaddress::IPv4Address(subscription.addr));
        mcast_addr_ = subscription.addr;
        auto res = create_datagram_endpoint(
            py::cpp_function([this]() -> PyAsyncioUdpSocket* {return this;}, py::return_value_policy::reference),
            "local_addr"_a=py::make_tuple(base_v4, subscription.port), "reuse_port"_a=true);
        write_bound_ = true;
        loop.attr("create_task")(res);
    }

    void connection_made(py::object transport){
        DBG
        transport_ = transport;
        auto sock = transport_.attr("get_extra_info")(py::cast("socket"));
        py::tuple localaddr = sock.attr("getsockname")();
        auto localip = py::int_(ipaddress::IPv4Address_cls()(localaddr[0])).cast<uint32_t>();
        auto localport = py::int_(localaddr[1]).cast<uint16_t>();
        local_address_ = NetworkAddress{MessageAddr{localip}, localport};
        py::print(reinterpret_cast<std::uintptr_t>(this), "Localaddr changed:", local_address_);
        py::function setsockopt_ = sock.attr("setsockopt");
        setsockopt_(SOL_SOCKET, SO_REUSEADDR, 1);
        setsockopt_(SOL_SOCKET, SO_REUSEPORT, 1);
        if(mcast_addr_.isValidMessage()){
            struct ip_mreq mreq;
            mreq.imr_multiaddr.s_addr = htonl(mcast_addr_);
            mreq.imr_interface.s_addr = htonl(base_addr_);
            setsockopt_(
                static_cast<uint64_t>(IPPROTO_IP), static_cast<uint64_t>(IP_ADD_MEMBERSHIP), 
                py::memoryview::from_memory(reinterpret_cast<const void*>(&mreq), sizeof(mreq)));
        }
        send_all_from_queue();
    }
        
        // sock = self.transport.get_extra_info('socket')
        // ttl = struct.pack('b', 1)
        // sock.setsockopt(socket.IPPROTO_IP, socket.IP_MULTICAST_TTL, ttl)

        // print('Send:', DISCOVERY_MESSAGE)
        // self.transport.sendto(DISCOVERY_MESSAGE)

    void datagram_received(py::object odata, py::object oaddr){
        DBG
        py::buffer data = odata;
        py::tuple addr = oaddr;
        auto info = py::buffer(data).request();
        auto remoteIp = py::int_(ipaddress::IPv4Address_cls()(addr[0])).cast<uint32_t>();
        auto remotePort = py::int_(addr[1]).cast<uint16_t>();
        try{
            result(handler_(
                NetworkAddress{MessageAddr{remoteIp}, remotePort}, 
                local_address_,  
                reinterpret_cast<const uint8_t*>(info.ptr), info.shape[0]));
        }
        catch(const std::exception& e) {
            py::print("Caught exception \"", e.what(), "\"");
        }
    }

    void error_received(py::object exc){
        DBG
        py::print("Error received:", exc);
    }

    void connection_lost(py::object exc){
        DBG
        transport_ = py::none();
        py::print("Socket closed", exc);
    }

    virtual ~PyAsyncioUdpSocket() override;
};
PyAsyncioUdpSocket::~PyAsyncioUdpSocket(){
    DBG
    destructor_helper(__func__, [this](){
        if(transport_.is(py::none())){
            transport_.attr("close")();
        }
    });
}

struct PyAsyncioTimer final: public TimerImpl{
    py::object call_later;
    py::object handle;
    PyAsyncioTimer(){
        DBG
        py::gil_scoped_acquire gil;
        call_later = asyncio::get_event_loop().attr("call_later");
        handle = py::none();
    }
    virtual void startSingleShot(uint32_t millis, callback_t callback) override{
        DBG
        py::gil_scoped_acquire gil;
        handle = call_later(float(millis) / 1000., py::cpp_function(callback));
    }
    virtual void stop() override{
        DBG
        py::gil_scoped_acquire gil;
        if(not handle) return;
        if(handle.is(py::none())) return;
        if(not handle.attr("cancelled")()){
            handle.attr("cancel")();
        }
    }
    ~PyAsyncioTimer() override;
};
PyAsyncioTimer::~PyAsyncioTimer(){
    DBG
    destructor_helper(__func__, [this](){
        stop();
    });
}


PYBIND11_MODULE(cyphalpypp, m) {
    m.doc() = "cyphalpypp"; // optional module docstring
    {
        py::module_ d = m.def_submodule("_detail");
        py::class_<PyAsyncioUdpSocket>(m, "PyAsyncioUdpSocket")
            .def("connection_made", &PyAsyncioUdpSocket::connection_made)
            .def("datagram_received", &PyAsyncioUdpSocket::datagram_received)
            .def("error_received", &PyAsyncioUdpSocket::error_received)
            .def("connection_lost", &PyAsyncioUdpSocket::connection_lost);

    }
    {
        using namespace cyphalpp::errors;
        using NunavutError = nunavut::support::Error;
        py::module_ e = m.def_submodule("errors");
        py::enum_<DataSpecifierConversion>(e, "DataSpecifierConversion")
            .value("WrongSubnet", DataSpecifierConversion::WrongSubnet)
            .value("WrongPort", DataSpecifierConversion::WrongPort)
            .value("PortTooSmall", DataSpecifierConversion::PortTooSmall);
        
        py::enum_<ParsePacket>(e, "ParsePacket")
            .value("MessageTooSmall", ParsePacket::MessageTooSmall)
            .value("RolesMismatch", ParsePacket::RolesMismatch)
            .value("SubjectIdMismatch", ParsePacket::SubjectIdMismatch)
            .value("HeaderWrongProtocolVersion", ParsePacket::HeaderWrongProtocolVersion)
            .value("MultiframeTransfersNotSupported", ParsePacket::MultiframeTransfersNotSupported);

        py::enum_<ServiceCall>(e, "ServiceCall")
            .value("TransferIdOutOfSync", ServiceCall::TransferIdOutOfSync);

        py::enum_<NunavutError>(e, "NunavutError")
            .value("SERIALIZATION_INVALID_ARGUMENT", NunavutError::SERIALIZATION_INVALID_ARGUMENT)
            .value("SERIALIZATION_BUFFER_TOO_SMALL", NunavutError::SERIALIZATION_BUFFER_TOO_SMALL)
            .value("REPRESENTATION_BAD_ARRAY_LENGTH", NunavutError::REPRESENTATION_BAD_ARRAY_LENGTH)
            .value("REPRESENTATION_BAD_UNION_TAG", NunavutError::REPRESENTATION_BAD_UNION_TAG)
            .value("REPRESENTATION_BAD_DELIMITER_HEADER", NunavutError::REPRESENTATION_BAD_DELIMITER_HEADER);
    }


    auto ds = py::class_<DataSpecifier>(m, "DataSpecifier");
    ds.doc() = "Describes data transfer - to/from which node it goes";
    py::enum_<DataSpecifier::Role>(ds, "Role")
        .value("Message", DataSpecifier::Role::Message)
        .value("ServiceRequest", DataSpecifier::Role::ServiceRequest)
        .value("ServiceResponce", DataSpecifier::Role::ServiceResponce);
    ds.def(py::init<const DataSpecifier::Role&, const uint16_t&, const uint16_t&>())
        .def_readwrite("role", &DataSpecifier::role)
        .def_readwrite("node_id", &DataSpecifier::node_id)
        .def_readwrite("subject_id", &DataSpecifier::subject_id)
        .def(py::self == py::self)
        .def("__repr__",
            [](const DataSpecifier &a) -> std::string {
                return "<cyphalpypp.DataSpecifier "\
                    " role=" + std::string(py::repr(py::cast(a.role))) + ", "\
                    " node_id=" + std::to_string(a.node_id) + ","\
                    " subject_id=" + std::to_string(a.subject_id) + " >";
            }
        );


    auto na = py::class_<MessageAddr>(m, "MessageAddr");
    na.doc() = "IPv4 address with port";
    na.def(py::init<const uint32_t&>());
    na.def("__repr__",
            [](const MessageAddr &a) -> std::string {
                return "<cyphalpypp.MessageAddr "\
                    + std::string(py::repr(ipaddress::IPv4Address(uint32_t(a)))) + \
                    " >";
            }
        );
        // .def_readwrite("addr", &MessageAddr::addr)
        // .def_readwrite("port", &MessageAddr::port);
        ;

    auto ma = py::class_<NetworkAddress>(m, "NetworkAddress");
    ma.doc() = "IPv4 address with port";
    ma.def(py::init<const MessageAddr &, const uint16_t&>())
        .def_readwrite("addr", &NetworkAddress::addr)
        .def_readwrite("port", &NetworkAddress::port)
        .def("__repr__",
            [](const NetworkAddress &a) -> std::string {
                return "<cyphalpypp.NetworkAddress "\
                    " addr=" + std::string(py::repr(py::cast(a.addr))) + ", "\
                    " port=" + std::to_string(a.port) + ","\
                    " >";
            }
        );
    
    auto tm = py::class_<TransferMetadata>(m, "TransferMetadata");
    tm.def(py::init<const DataSpecifier &, const uint64_t&, const uint8_t&>())
        .def_readwrite("data", &TransferMetadata::data)
        .def_readwrite("transfer_id", &TransferMetadata::transfer_id)
        .def_readwrite("priority", &TransferMetadata::priority)
        .def("__repr__",
            [](const TransferMetadata &a) -> std::string {
                return "<cyphalpypp.TransferMetadata "\
                    " data=" + std::string(py::repr(py::cast(a.data))) + ", "\
                    " transfer_id=" + std::to_string(a.transfer_id) + ","\
                    " priority=" + std::to_string(a.priority) + " >";
            }
        );


    auto cy = py::class_<CyphalUdp>(m, "CyphalUdp");
    cy.def(py::init<>([](){
            return new CyphalUdp([](){return std::make_unique<PyAsyncioUdpSocket>(); }, [](){return std::make_unique<PyAsyncioTimer>(); } );
        }), py::call_guard<py::gil_scoped_acquire>())
        .def("setAddr", &CyphalUdp::setAddr)
        .def("sendMessage", 
            [](CyphalUdp& self, py::object msg, uint8_t priority){
                if(not msg.attr("HasFixedPortID").cast<bool>()){
                    throw py::value_error("This message does not have a fixed port id");
                }
                auto port_id = msg.attr("FixedPortId").cast<uint16_t>();
                auto extent = msg.attr("EXTENT_BYTES").cast<size_t>();
                return result(self.sendMessage(
                    extent, 
                    [msg](uint8_t* buf, size_t size)->size_t{
                        py::function serialize = msg.attr("serialize");
                        auto buffer = py::memoryview::from_memory(buf, size);
                        return serialize(buffer).cast<size_t>();
                    }, 
                    [port_id]()->uint16_t{
                        return port_id;
                    }));
            }, 
            py::arg("message"), py::arg("priority") = cyphal_udp::Header_1_0::PriorityNominal)
        .def("subscribeMessage", 
            [](CyphalUdp& self, py::object msgcls, py::function callback){
                if(not msgcls.attr("HasFixedPortID").cast<bool>()){
                    throw py::value_error("This message does not have a fixed port id");
                }
                auto port_id = msgcls.attr("FixedPortId").cast<uint16_t>();
                self.subscribeRawMessage([msgcls, callback](const TransferMetadata& tm, const uint8_t* buf, size_t size)->tl::expected<void, Error>{
                    auto msg = msgcls();
                    py::function deserialize = msg.attr("deserialize");
                    auto buffer = py::memoryview::from_memory(buf, size);
                    try{
                        deserialize(buffer).cast<size_t>();
                        callback(tm, msg);
                    }
                    catch(const std::exception& e) {
                        py::print("Caught exception \"", e.what(), "\" while handling message ", msgcls);
                        return tl::unexpected<Error>(-1);
                    }
                    return {};
                },
                [port_id]()->uint16_t{
                    return port_id;
                });
            }, 
            py::arg("messageType"), py::arg("callback"))
        .def("subscribeServiceRequest", 
            [](CyphalUdp& self, py::object msgcls, py::function callback){
                py::object request = msgcls.attr("Request");
                if(request.is(py::none())){
                    throw py::type_error("Expected service class - no Request class is None");
                }
                if(not request.attr("HasFixedPortID").cast<bool>()){
                    throw py::value_error("Request does not have a fixed port id");
                }
                auto port_id = request.attr("FixedPortId").cast<uint16_t>();
                self.subscribeRawServiceRequest(
                [port_id]()->uint16_t{
                    return port_id;
                },
                [&self, msgcls, request, callback](const TransferMetadata& tm, const uint8_t* buf, size_t size)->tl::expected<CyphalUdp::RawResponse, Error>{
                    try{
                        auto msg = request();
                        py::function deserialize = msg.attr("deserialize");
                        auto buffer = py::memoryview::from_memory(buf, size);
                        deserialize(buffer).cast<size_t>();
                        py::object ret = callback(tm, msg);
                        auto extent = ret.attr("EXTENT_BYTES").cast<size_t>();
                        return CyphalUdp::RawResponse{
                            [ret](uint8_t* buf, size_t size)->size_t{
                                py::function serialize = ret.attr("serialize");
                                auto buffer = py::memoryview::from_memory(buf, size);
                                return serialize(buffer).cast<size_t>();
                            },
                            extent
                        };
                    }
                    catch(const std::exception& e) {
                        py::print("Caught exception \"", e.what(), "\" while handling request ", msgcls);
                        return tl::unexpected<Error>(-1);
                    }
                    return {};
                });
            }, 
            py::arg("messageType"), py::arg("callback"));
}

