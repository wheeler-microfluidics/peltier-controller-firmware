#include "Arduino.h"
#include "extension_module_base.h"
#include <avr/eeprom.h>
#include <math.h>
#include <SPI.h>
#include <Wire.h>

#define P(str) (strcpy_P(this->p_buffer, PSTR(str)), this->p_buffer)

using namespace microdrop;


ExtensionModuleBase::ExtensionModuleBase() {
    ExtensionModuleBase(new ExtensionModuleWireHandler(this));
}

ExtensionModuleBase::ExtensionModuleBase(
        ExtensionModuleWireHandler *wire_handler) {
    this->wire_handler_ = wire_handler;
}

/* Initialize the communications for the extension module. */
void ExtensionModuleBase::begin() {
    SPI.setBitOrder(MSBFIRST);
    SPI.setDataMode(SPI_MODE0);
    SPI.begin();
    Serial.begin(115200);
    LoadConfig();
    DumpConfig();
    Wire.onRequest(HandleWireRequest);
    Wire.onReceive(HandleWireReceive);
    Serial.print(P("ram=")); Serial.println(ram_size(), DEC);
    Serial.print(P(".data=")); Serial.println(data_size(), DEC);
    Serial.print(P(".bss=")); Serial.println(bss_size(), DEC);
    Serial.print(P("heap=")); Serial.println(heap_size(), DEC);
    Serial.print(P("stack=")); Serial.println(stack_size(), DEC);
    Serial.print(P("free memory=")); Serial.println(free_memory(), DEC);
}

bool ExtensionModuleBase::ReadSerialCommand() {
    if(Serial.available()) {
        byte len = Serial.readBytesUntil('\n', this->buffer_,
                                         sizeof(this->buffer_));
        this->buffer_[len]=0;
        return true;
    }
    return false;
}

/* Process any available requests on the serial port, or through Wire/I2C. */
void ExtensionModuleBase::Listen() {
    if (this->ReadSerialCommand()) {
        // A new-line-terminated command was successfully read into the buffer.
        // Call `ProcessSerialInput` to handle process command string.
        if (!this->ProcessSerialInput()) {
            // Command was not handled.
            Error(1);
        }
    }
    if (this->wire_command_received_) {
      ProcessWireCommand();
    }
}

/* Write a sequence of `size` bytes to the buffer. */
void ExtensionModuleBase::Serialize(const uint8_t* u, const uint16_t size) {
    for(uint16_t i = 0; i < size; i++) {
      this->buffer_[this->bytes_written_+i] = u[i];
    }
    this->bytes_written_ += size;
}

/* Read a `float` value from the buffer. */
float ExtensionModuleBase::ReadFloat() {
    // TODO check that we're not reading past the end of the buffer
    this->bytes_read_ += sizeof(float);
    return *(float*)(this->buffer_ + this->bytes_read_ - sizeof(float));
}

/* Read a `uint8_t` value from the buffer. */
uint8_t ExtensionModuleBase::ReadUint8() {
    // TODO check that we're not reading past the end of the buffer
    this->bytes_read_ += sizeof(uint8_t);
    return *(uint8_t*)(this->buffer_ + this->bytes_read_ - sizeof(uint8_t));
}

/* Print the configuration of the extension module to the serial port. */
void ExtensionModuleBase::DumpConfig() {
    Serial.println(String(name()) + " v" + String(hardware_version()));
    Serial.println(P("Firmware v") + String(software_version()));
    Serial.println(url());
    Serial.println(P("config_version=") + VersionString(ConfigVersion()));
    Serial.println(P("i2c_address=") +
                   String(this->config_settings_.i2c_address, DEC));
}

/* If there is a request pending on the serial port, process it. */
bool ExtensionModuleBase::ProcessSerialInput() {
    char* substr;

    if(strcmp(this->buffer_, P("reset_config()"))==0) {
        LoadConfig(true);
        DumpConfig();
        return true;
    }

    if(strcmp(this->buffer_, P("config()"))==0) {
        DumpConfig();
        return true;
    }

    if(strcmp(this->buffer_, P("i2c_address()"))==0) {
        Serial.println(P("i2c_address=") +
                       String(this->config_settings_.i2c_address, DEC));
        return true;
    }

    substr = strstr(this->buffer_, P("set_i2c_address("));
    if (substr != NULL && substr == this->buffer_
        && substr[strlen(substr) - 1] == ')') {
      buffer_[strlen(substr) - 1] = 0;
      set_i2c_address(atoi(substr + strlen(P("set_i2c_address("))));
      return true;
    }
    /* Command was not processed */
    return false;
}

/* If there is a request pending from the I2C bus, process it. */
void ExtensionModuleBase::ProcessWireCommand() {
    this->bytes_written_ = 0;
    this->bytes_read_ = 0;
    this->send_payload_length_ = true;
    this->return_code_ = RETURN_GENERAL_ERROR;
    switch(this->cmd_) {
        case CMD_PING:
          if(this->payload_length_==0) {
            Serialize("pong", sizeof(5));
            this->return_code_ = RETURN_OK;
          } else {
            this->return_code_ = RETURN_BAD_PACKET_SIZE;
          }
          break;
        default:
          break;
    }
    Serialize(&this->return_code_, sizeof(this->return_code_));
    this->wire_command_received_ = false;
}

void ExtensionModuleBase::Error(uint8_t code) {
  Serial.println(P("Error ") + String(code, DEC));
}

boolean ExtensionModuleBase::NextInt(char* &str, int32_t &value) {
  char* end = strstr(str, ",");
  if(end==NULL) {
    end = strstr(str, ")");
  }
  if(end==NULL) {
    return false;
  }
  char num_str[end - str + 1];
  memcpy(num_str, str, end - str);
  num_str[end - str] = 0;
  str = end + 1;
  value = atoi(num_str);
  return true;
}

String ExtensionModuleBase::VersionString(version_t version) {
  return String(version.major) + "." + String(version.minor) + "." +
         String(version.micro);
}

version_t ExtensionModuleBase::ConfigVersion() {
  version_t config_version;
  eeprom_read_block((void*)&config_version, (void*)EEPROM_CONFIG_SETTINGS,
                    sizeof(version_t));
  return config_version;
}

void ExtensionModuleBase::LoadConfig(bool use_defaults) {
  eeprom_read_block((void*)&this->config_settings_,
                    (void*)EEPROM_CONFIG_SETTINGS, sizeof(config_settings_t));

  // If we're not at the expected version by the end of the upgrade path,
  // set everything to default values.
  if(!(this->config_settings_.version.major==0 &&
     this->config_settings_.version.minor==0 &&
     this->config_settings_.version.micro==0) || use_defaults) {

    this->config_settings_.version.major=0;
    this->config_settings_.version.minor=0;
    this->config_settings_.version.micro=0;
    this->config_settings_.i2c_address = 10;
    SaveConfig();
  }
  Wire.begin(this->config_settings_.i2c_address);
}

void ExtensionModuleBase::SaveConfig() {
  eeprom_write_block((void*)&this->config_settings_,
                     (void*)EEPROM_CONFIG_SETTINGS, sizeof(config_settings_t));
}

void ExtensionModuleBase::set_i2c_address(uint8_t address) {
  this->config_settings_.i2c_address = address;
  Wire.begin(this->config_settings_.i2c_address);
  Serial.println(P("i2c_address=") + String(this->config_settings_.i2c_address,
                 DEC));
  SaveConfig();
}
