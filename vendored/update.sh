#/bin/bash
#
# Copyright © 2022 JSC "VNIIZHT" or its affiliates. All Rights Reserved.
# This software is distributed under the terms of the MIT License.
#

cd `dirname $0`
mkdir -p tl
rm -rf tl/expected.hpp
wget -O tl/expected.hpp https://raw.githubusercontent.com/TartanLlama/expected/master/include/tl/expected.hpp
wget -O tl/COPYING      https://raw.githubusercontent.com/TartanLlama/expected/master/COPYING

mkdir -p mpark
rm -rf mpark/variant.hpp
wget -O mpark/variant.hpp https://raw.githubusercontent.com/mpark/variant/single-header/v1.4.0/variant.hpp
wget -O mpark/LICENSE.md  https://raw.githubusercontent.com/mpark/variant/single-header/LICENSE.md

sha256sum -c <<EOF
227af67703098f6f5bd3655b28a2c6b3e84fc791e34936b2a176279fdd8ee6f2  mpark/variant.hpp
c9bff75738922193e67fa726fa225535870d2aa1059f91452c411736284ad566  mpark/LICENSE.md
a2010f343487d3f7618affe54f789f5487602331c0a8d03f49e9a7c547cf0499  tl/COPYING
12a643a033a2fe0ef51a108e16d099ee0f3ff47d67129bc5065314f1a276ede9  tl/expected.hpp
EOF
