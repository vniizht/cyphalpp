from .registry import Registry
import asyncio
import cyphalpypp
from cytypes import uavcan

class AsyncSub:
    def __init__(self, cy, cls):
        self._q = asyncio.Queue()
        cy.subscribeMessage(cls, self.__handler)
    
    def __handler(self, tm, o):
        #print(tm, o)
        self._q.put_nowait(o)
    
    async def get(self, timeout=1.0):
        g = asyncio.ensure_future(self._q.get())
        try:
            o,_ = await asyncio.wait([g], timeout=timeout)
        except CancelledError:
            g.cancel()
            raise
        if len(o) == 0:
            g.cancel()
            await asyncio.wait([g])
            return None
        else:
            return next(iter(o)).result()



class Node:
    @staticmethod
    def get_local_addr():
        import socket
        import ipaddress
        import psutil
        addrs = [ 
            (eth, [ a.address for a in addrs if a.family == socket.AF_INET ])  
            for eth, addrs in psutil.net_if_addrs().items()
            if eth.startswith("enp") or eth.startswith("eth")
        ]
        iface, addrs = addrs[0]
        return cyphalpypp.MessageAddr(int(ipaddress.IPv4Address(addrs[0])))
    
    async def __publish_heart(self):
        while True:
            self.heart.uptime += 1
            self._cy.sendMessage(self.heart)
            await asyncio.sleep(1.0)
    
    def __handle_get_info(self, tm, req: uavcan.node.GetInfo_1_0.Request) -> uavcan.node.GetInfo_1_0.Response:
        return self.node_info

    def __init__(self, addr=None, registry_db=":memory:"):
        if isinstance(addr, str):
            addr = cyphalpypp.MessageAddr(int(ipaddress.IPv4Address(addr)))
        self._cy = cyphalpypp.CyphalUdp()
        if addr is None:
            addr = Node.get_local_addr()
        self._cy.setAddr(addr)
        self.registry = Registry(self._cy, registry_db)
        self.heart = uavcan.node.Heartbeat_1_0()
        self.sendMessage = self._cy.sendMessage
        self.subscribeMessage = self._cy.subscribeMessage
        self.subscribeServiceRequest = self._cy.subscribeServiceRequest
        self.node_info = uavcan.node.GetInfo_1_0.Response(
            protocol_version=uavcan.node.Version_1_0(1,0),
            hardware_version=uavcan.node.Version_1_0(0,0),
            software_version=uavcan.node.Version_1_0(0,0),
            software_vcs_revision_id=0,
            name=list(map(ord, "ru.vniizht.test"))
        )
        self._cy.subscribeServiceRequest(uavcan.node.GetInfo_1_0, self.__handle_get_info)
        self._tasks = [ ]
    
    def __enter__(self):
        self.registry.__enter__()
        return self
    
    def __exit__(self, *exc_info):
        self.registry.__exit__(*exc_info)
    
    
    def make_subscriber(self, cls):
        return AsyncSub(self._cy, cls)
    
    async def run(self):
        self._tasks.append(self.__publish_heart())
        await asyncio.gather(*self._tasks)




