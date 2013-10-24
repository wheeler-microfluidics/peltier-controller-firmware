#ifndef ___EXTENSION_MODULE_BASE__HPP___
#define ___EXTENSION_MODULE_BASE__HPP___

#include "Arduino.h"
#include <Wire.h>
#include <stdint.h>
#include "Memory.h"


extern "C" {
    void HandleWireRequest();
    void HandleWireReceive(int n_bytes);
}


namespace microdrop {

struct version_t {
    uint16_t major;
    uint16_t minor;
    uint16_t micro;
};

struct config_settings_t {
    version_t version;
    uint8_t i2c_address;
};


class WireHandler {
public:
    virtual void handle_request() = 0;
    virtual void handle_receive(int n_bytes) = 0;
};


class ExtensionModuleWireHandler;

class ExtensionModuleBase {
public:
  static const uint8_t CMD_PING =           0xA7;

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
  static const int FREQ_RANGE_PIN = 4;
  static const float LOG_F_STEP;
  static prog_uchar R1_INDEX[] PROGMEM;
  static prog_uchar R2_INDEX[] PROGMEM;
  static prog_uchar R4_INDEX[] PROGMEM;
  static prog_uchar R5_INDEX[] PROGMEM;

  static const uint16_t EEPROM_CONFIG_SETTINGS = 0;

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

  void Listen();
  void ping();
  void set_i2c_address(uint8_t address);
  version_t ConfigVersion();
  void ProcessWireCommand();

  boolean send_payload_length_;
  uint8_t cmd_;
  uint16_t bytes_read_; // bytes that have been read (by Read methods)
  uint16_t bytes_written_; // bytes that have been written (by Serialize method)
  uint16_t payload_length_;
  boolean wire_command_received_;
  char buffer_[MAX_PAYLOAD_LENGTH];
  char p_buffer[100];
  WireHandler *wire_handler_;

  ExtensionModuleBase();

  virtual ~ExtensionModuleBase() {
    delete this->wire_handler_;
  }
private:
  template<typename T> void Serialize(T data, uint16_t size) {
    Serialize((const uint8_t*)data, size); }
  void Serialize(const uint8_t* u, const uint16_t size);
  const char* ReadString();
  uint16_t ReadUint16();
  int16_t ReadInt16();
  uint8_t ReadUint8();
  int8_t ReadInt8();
  float ReadFloat();

  String VersionString(version_t version);
  boolean NextInt(char* &str, int32_t &value);
  void ProcessSerialInput();
  void Error(uint8_t code);
  void DumpConfig();
  void LoadConfig(bool use_defaults=false);
  void SaveConfig();

  static prog_char SOFTWARE_VERSION_[] PROGMEM;
  static prog_char NAME_[] PROGMEM;
  static prog_char HARDWARE_VERSION_[] PROGMEM;
  static prog_char MANUFACTURER_[] PROGMEM;
  static prog_char URL_[] PROGMEM;
  static prog_char PROTOCOL_NAME_[] PROGMEM;
  static prog_char PROTOCOL_VERSION_[] PROGMEM;
  config_settings_t config_settings_;
  uint8_t return_code_;
};


class ExtensionModuleWireHandler : public WireHandler {
public:
    ExtensionModuleBase *ext_mod_;

    ExtensionModuleWireHandler(ExtensionModuleBase *ext_mod)
            : ext_mod_(ext_mod) {}

    virtual void handle_request() {
        if(this->ext_mod_->send_payload_length_) {
            Wire.write((uint8_t*)&this->ext_mod_->bytes_written_,
                       sizeof(this->ext_mod_->bytes_written_));
            this->ext_mod_->send_payload_length_ = false;
        } else {
            Wire.write((uint8_t*)this->ext_mod_->buffer_,
                       this->ext_mod_->bytes_written_);
        }
    }

    void handle_receive(int n_bytes) {
      this->ext_mod_->cmd_ = Wire.read();
      n_bytes--;
      this->ext_mod_->payload_length_ = n_bytes;
      if(n_bytes <= ExtensionModuleBase::MAX_PAYLOAD_LENGTH) {
        for(int i = 0; i < n_bytes; i++) {
          this->ext_mod_->buffer_[i] = Wire.read();
        }
      }
      this->ext_mod_->wire_command_received_ = true;
    }
};

}

#endif // ___EXTENSION_MODULE_BASE__HPP___
