import ipaddress as _ipa_
import cyphalpypp as _cy_

from .node import Node

def address(a):
    return _cy_.MessageAddr(int(_ipa_.IPv4Address(a)))