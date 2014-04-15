from struct import pack, unpack

# command codes
CMD_GET_PROTOCOL_NAME =        0x80;
CMD_GET_PROTOCOL_VERSION =     0x81;
CMD_GET_DEVICE_NAME =          0x82;
CMD_GET_MANUFACTURER =         0x83;
CMD_GET_HARDWARE_VERSION =     0x84;
CMD_GET_SOFTWARE_VERSION =     0x85;
CMD_GET_URL =                  0x86;

# reserved return codes
RETURN_OK =                    0x00;
RETURN_GENERAL_ERROR =         0x01;
RETURN_UNKNOWN_COMMAND =       0x02;
RETURN_TIMEOUT =               0x03;
RETURN_NOT_CONNECTED =         0x04;
RETURN_BAD_INDEX =             0x05;
RETURN_BAD_PACKET_SIZE =       0x06;
RETURN_BAD_CRC =               0x07;
RETURN_BAD_VALUE =             0x08;
RETURN_MAX_PAYLOAD_EXCEEDED =  0x09;

class ExtensionModule():
    def __init__(self, control_board, address):
        self.control_board = control_board
        self.address = address
        self.write_buffer = []

    def protocol_name(self):
        return self._get_string(CMD_GET_PROTOCOL_NAME)

    def protocol_version(self):
        return self._get_string(CMD_GET_PROTOCOL_VERSION)

    def name(self):
        return self._get_string(CMD_GET_DEVICE_NAME)

    def manufacturer(self):
        return self._get_string(CMD_GET_MANUFACTURER)

    def hardware_version(self):
        return self._get_string(CMD_GET_HARDWARE_VERSION)
        
    def software_version(self):
        return self._get_string(CMD_GET_SOFTWARE_VERSION)

    def url(self):
        return self._get_string(CMD_GET_URL)

    def send_command(self, cmd):
        self.data = self.control_board.i2c_send_command(self.address,
            cmd, self.write_buffer).tolist()
        self.write_buffer = []

    def _get_string(self, cmd):
        self.send_command(cmd)
        return pack('B'*len(self.data), *self.data)

    def read_float(self):
        num = self.data[0:4]
        self.data = self.data[4:]
        return unpack('f', pack('BBBB', *num))[0]

    def read_uint8(self):
        return self.data.pop(0)

    def serialize_uint8(self, num):
        self.write_buffer.append(num)

    def serialize_float(self, num):
        self.write_buffer.extend(unpack('BBBB', pack('f', num)))

