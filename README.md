# cyphalpp

This is a header-only Cyphal/UDP implementation for C++14. See [reference](https://vniizht.github.io/cyphalpp/).


It is written in a implementation-agnostic asyncronous way - you would need to provide a backend implementation for an asynchronous socket and timer.


## Limitations

* **Only single-frame transfers** are supported. UDP max message transfer unit (MTU) is assumed to be 1200 bytes.
* **Inherently single-threaded**. There is only one serialization buffer, which is used for all outgoing messages.

## Implementations

This repository also includes backend implementations for:
* [Qt5](#qt5)
* [boost.asio](#boost)



To use this library, you would need to include [file](include/cyphalpp.hpp) include/cyphalpp.hpp.
But besides that file, you would also need an implementation for an async socket on a target platform and transpiled DSDL definitions.
You would need to transpile definitions for this repository also (see cyphal_udp [folder](./cyphal_udp)).

A very basic example is as follows:

```cpp
#include "cyphalpp.hpp"
#include // your target implementation

// transpiled DSDL
#include <uavcan/node/Heartbeat_1_0.hpp>
#include <uavcan/node/ExecuteCommand_1_1.hpp>
#include <uavcan/si/sample/length/Scalar.1.0.hpp>

int main(){
    // initialize target async framework
    cyphalpp::CyphalUdp cy(/*target socket factory*/, /*target timer factory*/);
    cy.setAddr(cyphalpp::MessageAddr(/* this should be IPv4 uint32_t local address in host byte order*/));

    // subscribe to a fixed port-id message
    cy.subscribe<uavcan::node::Heartbeat_1_0>([](const cyphalpp::TransferMetadata& md, const uavcan::node::Heartbeat_1_0& hb){
       // do whatever with hb object.
    });

    // subscribe to uavcan.si.sample.length.Scalar.1.0 (non-fixed port-id) on port-id 100
    cy.subscribeMessage<uavcan::si::sample::length::Scalar_1_0>(
        [](
                const TransferMetadata& tm, const uavcan::si::sample::length::Scalar_1_0& sample
        ){
            // do whatever with sample object
        },
        fixed_port_id<100>);

    // listen to calls of a fixed port-id service
    using ExecuteCommand = uavcan::node::ExecuteCommand::Service_1_1;
    cy.subscribeServiceRequest<ExecuteCommand> ([](const cyphalpp::TransferMetadata& tm, const ExecuteCommand::Request& req) -> ExecuteCommand::Response {
       // process request req
       // ....

       // return response object
       ExecuteCommand::Response resp{};
       return resp;
    });

    // start and enter target event loop
    return 0;
}

```

## Qt5

The requirements for this implementation are Qt5, qmake5 and [nunavut](https://github.com/OpenCyphal/nunavut) installed in your system.


* Clone this repository to a known location (`/some/path/to/cyphalpp` is used here as an example).
* Add the following code to your qmake project file:

```pro

# Put additional variables here to configure DSDL here.

include(/some/path/to/cyphalpp/qt/cyphalpp.pri){
}else{
  error("Failed to include cyphalpp.pri !")
}

```


## boost.asio

