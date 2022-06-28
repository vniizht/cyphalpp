# cyphalpp

This is a header-only Cyphal/UDP implementation for C++14.


It is written in a implementation-agnostic asyncronous way - you would need to provide a backend implementation for an asynchronous socket and timer.

This repository also includes backend implementations for:
* [Qt5](#qt5)
* [boost.asio](#boost)



To use this library, you would need to include [file](include/cyphalpp.hpp) include/cyphalpp.hpp.
But besides that file, you would also need an implementaion.

## Qt5

The requirements for this implementations are Qt5 and a `qmake` project.

* Clone this repository to a known location (`/some/path/to/cyphalpp` is used here as an example).
* Add the following code to the qmake project file:

```pro

# Put additional variables here

include(/some/path/to/cyphalpp/qt/cyphalpp.pri){
}else{
  error("Failed to include cyphalpp.pri !")
}

```

## boost.asio

