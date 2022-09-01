from cytypes import uavcan
from ._sqlitedict import SqliteDict as _SqliteDict_


# def _reg_enc(x):
#     r = bytes(x.EXTENT_BYTES)
#     x.serialize(r)
#     return r

# def _reg_dec(b):
#     r = uavcan._register.Value_1_0()
#     r.deserialize(b)
#     return r


class Registry(_SqliteDict_):
    EMPTY = uavcan._register.Value_1_0(empty=uavcan.primitive.Empty_1_0())
    def __init__(self, cy, filename, changed_value_callback=None):
        super().__init__(filename, autocommit=True) #encode=_reg_enc, decode=_reg_dec)
        cy.subscribeServiceRequest(uavcan._register.Access_1_0, self.__handle_access)
        cy.subscribeServiceRequest(uavcan._register.List_1_0, self.__handle_list)
        self.changed_value_callback = changed_value_callback


    def setdefault(self, key, default_):
        try:
            return self[key]
        except KeyError:
            self[key] = default_
            return self[key]
    
    def __setitem__(self, key, value):
        if value is None:
            del super()[key]
            return None
        if isinstance(value, bool):
            value = uavcan._register.Value_1_0(bit=uavcan.primitive.array.Bit_1_0(value=[value]))
        elif isinstance(value, int):
            value = uavcan._register.Value_1_0(integer64=uavcan.primitive.array.Integer64_1_0(value=[value]))
        elif isinstance(value, float):
            value = uavcan._register.Value_1_0(real64=uavcan.primitive.array.Real64_1_0(value=[value]))
        elif isinstance(value, str):
            value = uavcan._register.Value_1_0(string=uavcan.primitive.String_1_0(value=list(value.encode())))
        elif isinstance(value, bytes):
            value = uavcan._register.Value_1_0(unstructured=uavcan.primitive.Unstructured_1_0(value=list(value)))
        if not isinstance(value, uavcan._register.Value_1_0):
            raise TypeError(f"Expected uavcan._register.Value_1_0, {str(type(value))}")
        ret = super().__setitem__(key, value)
        try:
            if self.changed_value_callback:
                self.changed_value_callback(key, value)
        except:
            from warnings import warn
            from traceback import format_exc
            warn(format_exc())
        return ret
    
    def __handle_access(self, tm, req):
        k = "".join(map(chr, req.name.name))
        v = req.value
        try:
            e = v.empty
        except ValueError:
            self[k] = v
        v = self.get(k, Registry.EMPTY)
        try:
            was_empty = v.empty or True
        except ValueError:
            was_empty = False
            self[k] = v
        return uavcan._register.Access_1_0.Response(
            value = v,
            _mutable=not was_empty,
            persistent=not was_empty
        )
    
    def __handle_list(self, tm, req):
        ks = sorted(self.keys())
        if req.index < 0 or req.index >= len(ks):
            return uavcan._register.List_1_0.Response()
        return uavcan._register.List_1_0.Response(
            uavcan._register.Name_1_0(
                name=list(map(ord, ks[req.index]))))
