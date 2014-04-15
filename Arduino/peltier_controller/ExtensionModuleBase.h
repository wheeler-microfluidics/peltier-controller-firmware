#ifndef ___EXTENSION_MODULE_BASE__HPP___
#define ___EXTENSION_MODULE_BASE__HPP___

#include "Arduino.h"
#include <Wire.h>
#include <stdint.h>


class ExtensionModuleBase {
public:
  struct Version {
      uint16_t major;
      uint16_t minor;
      uint16_t micro;
  };
  
  struct ConfigSettings {
      Version version;
      uint8_t i2c_address;
  };

  // reserved commands
  static const uint8_t CMD_GET_PROTOCOL_NAME =        0x80;
  static const uint8_t CMD_GET_PROTOCOL_VERSION =     0x81;
  static const uint8_t CMD_GET_DEVICE_NAME =          0x82;
  static const uint8_t CMD_GET_MANUFACTURER =         0x83;
  static const uint8_t CMD_GET_HARDWARE_VERSION =     0x84;
  static const uint8_t CMD_GET_SOFTWARE_VERSION =     0x85;
  static const uint8_t CMD_GET_URL =                  0x86;

  // reserved return codes
  static const uint8_t RETURN_OK =                    0x00;
  static const uint8_t RETURN_GENERAL_ERROR =         0x01;
  static const uint8_t RETURN_UNKNOWN_COMMAND =       0x02;
  static const uint8_t RETURN_TIMEOUT =               0x03;
  static const uint8_t RETURN_NOT_CONNECTED =         0x04;
  static const uint8_t RETURN_BAD_INDEX =             0x05;
  static const uint8_t RETURN_BAD_PACKET_SIZE =       0x06;
  static const uint8_t RETURN_BAD_CRC =               0x07;
  static const uint8_t RETURN_BAD_VALUE =             0x08;
  static const uint8_t RETURN_MAX_PAYLOAD_EXCEEDED =  0x09;

  static const uint16_t MAX_PAYLOAD_LENGTH = 100;
  static const uint32_t BAUD_RATE = 115200;

  static const uint16_t EEPROM_CONFIG_SETTINGS = 0;

  static void handle_wire_receive(int n_bytes);
  static void handle_wire_request();
  
  ExtensionModuleBase() { debug_ = false; }
  virtual void begin();
  // local accessors
  const char* name() { return prog_string(NAME_); }
  const char* hardware_version() { return prog_string(HARDWARE_VERSION_); }
  const char* url() { return prog_string(URL_); }
  const char* software_version() { return prog_string(SOFTWARE_VERSION_); }
  const char* protocol_name() { return prog_string(PROTOCOL_NAME_); }
  const char* protocol_version() { return prog_string(PROTOCOL_VERSION_); }
  const char* manufacturer() { return prog_string(MANUFACTURER_); }
  const char* prog_string(prog_char* str) { strcpy_P(buffer_, str); return buffer_; }


  virtual void listen();
  void set_i2c_address(uint8_t address);
  Version config_version();
  bool match_function(const char* function_name);
  void set_debug(bool debug) { debug_ = debug; }
  static bool send_payload_length_;
  static uint8_t cmd_;
  static uint16_t bytes_read_; // bytes that have been read (by Read methods)
  static uint16_t bytes_written_; // bytes that have been written (by Serialize method)
  static uint16_t payload_length_;
  static bool wire_command_received_;
  static char buffer_[MAX_PAYLOAD_LENGTH];
  static char p_buffer_[100];
protected:
  virtual void process_wire_command();
  virtual bool process_serial_input();
  ConfigSettings config_settings_;
  uint8_t return_code_;
  template<typename T> void serialize(T data, uint16_t size) {
    serialize((const uint8_t*)data, size); }
  void serialize(const uint8_t* u, const uint16_t size);
  const char* read_string();
  template <typename T> T read() {
    T result = *(T *)(buffer_ + bytes_read_);
    uint32_t size = sizeof(T);
    bytes_read_ += size;
    return result;
  }
  int32_t read_int_from_serial();
  float read_float_from_serial();

  String version_string(Version version);
  bool next_int(char* &str, int32_t &value);
  bool read_serial_command();
  void error(uint8_t code);
  void dump_config();
  void load_config(bool use_defaults=false);
  void save_config();

  static prog_char SOFTWARE_VERSION_[] PROGMEM;
  static prog_char NAME_[] PROGMEM;
  static prog_char HARDWARE_VERSION_[] PROGMEM;
  static prog_char MANUFACTURER_[] PROGMEM;
  static prog_char URL_[] PROGMEM;
  static prog_char PROTOCOL_NAME_[] PROGMEM;
  static prog_char PROTOCOL_VERSION_[] PROGMEM;
  
  bool debug_;
};

#endif // ___EXTENSION_MODULE_BASE__HPP___
