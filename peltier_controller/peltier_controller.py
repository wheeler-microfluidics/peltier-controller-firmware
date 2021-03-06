from base_node.base_node import BaseNode

CMD_SET_TARGET_TEMP = 0xA0
CMD_GET_TARGET_TEMP = 0xA1
CMD_SET_RAMP_SPEED  = 0xA2
CMD_GET_RAMP_SPEED  = 0xA3
CMD_GET_TEMP        = 0xA4
CMD_GET_CURRENT     = 0xA5


class PeltierController(BaseNode):
    def __init__(self, proxy, address):
        BaseNode.__init__(self, proxy, address)

    def target_temp(self):
        self.send_command(CMD_GET_TARGET_TEMP)
        return self.read_float()

    def set_target_temp(self, temp):
        self.data = []
        self.serialize_float(temp)
        self.send_command(CMD_SET_TARGET_TEMP)

    def ramp_speed(self):
        self.send_command(CMD_GET_RAMP_SPEED)
        return self.read_uint8()

    def set_ramp_speed(self, ramp_speed):
        self.data = []
        self.serialize_uint8(ramp_speed)
        self.send_command(CMD_SET_RAMP_SPEED)

    def temp(self):
        self.data = []
        self.send_command(CMD_GET_TEMP)
        return self.read_float()
