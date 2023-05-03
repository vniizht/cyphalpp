BOOST_CXX_FLAGS?=

all: registry-editor

registry-editor: .b.cli11 .b.messages.cyphal_udp .b.messages.uavcan main.cpp $(shell find ../.. -type f -name '*.hpp')
	$(CXX) \
		$(BOOST_CXX_FLAGS) \
		-DBOOST_BIND_GLOBAL_PLACEHOLDERS \
		-std=c++17 \
		-I generated \
		-I ../../asio \
		-I ../../include \
		-I ../../vendored/ \
		-o registry-editor \
		main.cpp \
		-pthread -D_REENTRANT

.b.support:
	nnvg \
		-Xlang -l cpp \
		--outdir generated \
		--language-standard=c++17 \
		--generate-support only
	touch .b.support

.b.messages.cyphal_udp: .b.support
	nnvg \
		-Xlang -l cpp \
		--outdir generated \
		--language-standard=c++17 \
		--generate-support never \
		../../cyphal_udp
	touch .b.messages.cyphal_udp

.b.messages.uavcan: .b.support .b.prdt_repo
	nnvg \
		-Xlang -l cpp \
		--outdir generated \
		--language-standard=c++17 \
		--generate-support never \
		generated/.prdt/uavcan
	touch .b.messages.uavcan

.b.prdt_repo:
	git clone https://github.com/OpenCyphal/public_regulated_data_types.git generated/.prdt
	touch .b.prdt_repo

.b.cli11:
	wget https://github.com/CLIUtils/CLI11/releases/download/v2.3.2/CLI11.hpp -O generated/CLI11.hpp
	touch .b.cli11
