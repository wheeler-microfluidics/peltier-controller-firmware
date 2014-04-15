#include "Arduino.h"
#include <avr/eeprom.h>
#include <math.h>
#include <SPI.h>
#include <Wire.h>

#include "ExtensionModuleBase.h"

#define P(str) (strcpy_P(p_buffer_, PSTR(str)), p_buffer_)


// initialize static members
bool ExtensionModuleBase::send_payload_length_ = false;
uint8_t ExtensionModuleBase::cmd_ = 0;
uint16_t ExtensionModuleBase::bytes_read_ = 0;
uint16_t ExtensionModuleBase::bytes_written_ = 0;
uint16_t ExtensionModuleBase::payload_length_ = 0;
bool ExtensionModuleBase::wire_command_received_ = false;
char ExtensionModuleBase::buffer_[MAX_PAYLOAD_LENGTH];
char ExtensionModuleBase::p_buffer_[100];

void ExtensionModuleBase::handle_wire_receive(int n_bytes) {
  cmd_ = Wire.read();
  n_bytes--;
  payload_length_ = n_bytes;
  if (n_bytes <= MAX_PAYLOAD_LENGTH) {
    for (int i = 0; i < n_bytes; i++) {
      buffer_[i] = Wire.read();
    }
  }
  wire_command_received_ = true;
}

void ExtensionModuleBase::handle_wire_request() {
  if (send_payload_length_) {
    Wire.write((uint8_t*)&bytes_written_,
               sizeof(bytes_written_));
    send_payload_length_ = false;
  } else {
    Wire.write((uint8_t*)buffer_, bytes_written_);
  }
}

/* Initialize the communications for the extension module. */
void ExtensionModuleBase::begin() {
    SPI.setBitOrder(MSBFIRST);
    SPI.setDataMode(SPI_MODE0);
    SPI.begin();
    Serial.begin(115200);
    load_config();
    dump_config();
    Wire.onRequest(handle_wire_request);
    Wire.onReceive(handle_wire_receive);
}

bool ExtensionModuleBase::read_serial_command() {
    if (Serial.available()) {
        byte len = Serial.readBytesUntil('\n', buffer_,
                                         sizeof(buffer_));
        buffer_[len]=0;
        return true;
    }
    return false;
}

/* Process any available requests on the serial port, or through Wire/I2C. */
void ExtensionModuleBase::listen() {
    if (read_serial_command()) {
        // A new-line-terminated command was successfully read into the buffer.
        // Call `ProcessSerialInput` to handle process command string.
        if (!process_serial_input()) {
            // Command was not handled.
            error(1);
        }
    }
    if (wire_command_received_) {
      bytes_written_ = 0;
      bytes_read_ = 0;
      send_payload_length_ = true;
      return_code_ = RETURN_GENERAL_ERROR;
      process_wire_command();
      serialize(&return_code_, sizeof(return_code_));
      wire_command_received_ = false;
    }
}

/* Write a sequence of `size` bytes to the buffer. */
void ExtensionModuleBase::serialize(const uint8_t* u, const uint16_t size) {
    for (uint16_t i = 0; i < size; i++) {
      buffer_[bytes_written_+i] = u[i];
    }
    bytes_written_ += size;
}

/* Print the configuration of the extension module to the serial port. */
void ExtensionModuleBase::dump_config() {
    Serial.println(String(name()) + " v" + String(hardware_version()));
    Serial.println(P("Firmware v") + String(software_version()));
    Serial.println(url());
    Serial.println(P("config_version=") + version_string(config_version()));
    Serial.println(P("i2c_address=") +
                   String(config_settings_.i2c_address, DEC));
}

/* If there is a request pending on the serial port, process it. */
bool ExtensionModuleBase::process_serial_input() {
    if (match_function(P("reset_config()"))) {
        load_config(true);
        dump_config();
        return true;
    }

    if (match_function(P("config()"))) {
        dump_config();
        return true;
    }

    if (match_function(P("set_i2c_address("))) {
      set_i2c_address(read_int_from_serial());
      return true;
    }
    /* Command was not processed */
    return false;
}

int32_t ExtensionModuleBase::read_int_from_serial() {
  return atoi(buffer_ + bytes_read_);
}

float ExtensionModuleBase::read_float_from_serial() {
  return atof(buffer_ + bytes_read_);
}

bool ExtensionModuleBase::match_function(const char* function_name) {
  if (strstr(buffer_, "()")) {
    return strcmp(buffer_, function_name)==0;
  } else {
    char* substr = strstr(buffer_, function_name);
    if (substr != NULL && substr == buffer_
        && substr[strlen(substr) - 1] == ')') {
      buffer_[strlen(substr) - 1] = 0;
      bytes_read_ += strlen(function_name);
      return true;
    } else {
      return false;
    }
  }
}


/* If there is a request pending from the I2C bus, process it. */
void ExtensionModuleBase::process_wire_command() {
  switch (cmd_) {
  case CMD_GET_PROTOCOL_NAME:
    if (payload_length_ == 0) {
      serialize(protocol_name(), strlen(protocol_name()));
      return_code_ = RETURN_OK;
    } else {
      return_code_ = RETURN_BAD_PACKET_SIZE;
    }
    break;
  case CMD_GET_PROTOCOL_VERSION:
    if (payload_length_ == 0) {
      serialize(protocol_version(), strlen(protocol_version()));
      return_code_ = RETURN_OK;
    } else {
      return_code_ = RETURN_BAD_PACKET_SIZE;
    }
    break;
  case CMD_GET_DEVICE_NAME:
    if (payload_length_ == 0) {
      serialize(name(), strlen(name()));
      return_code_ = RETURN_OK;
    } else {
      return_code_ = RETURN_BAD_PACKET_SIZE;
    }
    break;
  case CMD_GET_MANUFACTURER:
    if (payload_length_ == 0) {
      serialize(manufacturer(), strlen(manufacturer()));
      return_code_ = RETURN_OK;
    } else {
      return_code_ = RETURN_BAD_PACKET_SIZE;
    }
    break;
  case CMD_GET_SOFTWARE_VERSION:
    if (payload_length_ == 0) {
      serialize(software_version(), strlen(software_version()));
      return_code_ = RETURN_OK;
    } else {
      return_code_ = RETURN_BAD_PACKET_SIZE;
    }
    break;
  case CMD_GET_HARDWARE_VERSION:
    if (payload_length_ == 0) {
      serialize(hardware_version(), strlen(hardware_version()));
      return_code_ = RETURN_OK;
    } else {
      return_code_ = RETURN_BAD_PACKET_SIZE;
    }
    break;
  case CMD_GET_URL:
    if (payload_length_ == 0) {
      serialize(url(), strlen(url()));
      return_code_ = RETURN_OK;
    } else {
      return_code_ = RETURN_BAD_PACKET_SIZE;
    }
    break;
  default:
    break;
  }
}

void ExtensionModuleBase::error(uint8_t code) {
  Serial.println(P("Error ") + String(code, DEC));
}

bool ExtensionModuleBase::next_int(char* &str, int32_t &value) {
  char* end = strstr(str, ",");
  if (end==NULL) {
    end = strstr(str, ")");
  }
  if (end==NULL) {
    return false;
  }
  char num_str[end - str + 1];
  memcpy(num_str, str, end - str);
  num_str[end - str] = 0;
  str = end + 1;
  value = atoi(num_str);
  return true;
}

String ExtensionModuleBase::version_string(Version version) {
  return String(version.major) + "." + String(version.minor) + "." +
         String(version.micro);
}

ExtensionModuleBase::Version ExtensionModuleBase::config_version() {
  Version config_version;
  eeprom_read_block((void*)&config_version, (void*)EEPROM_CONFIG_SETTINGS,
                    sizeof(config_version));
  return config_version;
}

void ExtensionModuleBase::load_config(bool use_defaults) {
  eeprom_read_block((void*)&config_settings_,
                    (void*)EEPROM_CONFIG_SETTINGS, sizeof(config_settings_));

  // If we're not at the expected version by the end of the upgrade path,
  // set everything to default values.
  if (!(config_settings_.version.major==0 &&
     config_settings_.version.minor==0 &&
     config_settings_.version.micro==0) || use_defaults) {

    config_settings_.version.major=0;
    config_settings_.version.minor=0;
    config_settings_.version.micro=0;
    config_settings_.i2c_address = 10;
    save_config();
  }
  Wire.begin(config_settings_.i2c_address);
}

void ExtensionModuleBase::save_config() {
  eeprom_write_block((void*)&config_settings_,
                     (void*)EEPROM_CONFIG_SETTINGS, sizeof(config_settings_));
}

void ExtensionModuleBase::set_i2c_address(uint8_t address) {
  config_settings_.i2c_address = address;
  Wire.begin(config_settings_.i2c_address);
  Serial.println(P("i2c_address=") + String(config_settings_.i2c_address,
                 DEC));
  save_config();
}
