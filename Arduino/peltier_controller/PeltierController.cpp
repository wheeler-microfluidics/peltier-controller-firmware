#include "PeltierController.h"

#define P(str) (strcpy_P(p_buffer_, PSTR(str)), p_buffer_)

PeltierControllerClass PeltierController;

prog_char BaseNode::PROTOCOL_NAME_[] PROGMEM = "DropBot protocol";
prog_char BaseNode::PROTOCOL_VERSION_[] PROGMEM = "0.1";
prog_char BaseNode::MANUFACTURER_[] PROGMEM = "Wheeler Microfluidics Lab";
prog_char BaseNode::NAME_[] PROGMEM = "Peltier controller";
prog_char BaseNode::HARDWARE_VERSION_[] PROGMEM = "1.0";
prog_char BaseNode::SOFTWARE_VERSION_[] PROGMEM = "0.1";
prog_char BaseNode::URL_[] PROGMEM = "http://microfluidics.utoronto.ca/dropbot";

OneWire PeltierControllerClass::one_wire_ = OneWire(ONE_WIRE_BUS);
DallasTemperature PeltierControllerClass::sensors_ = DallasTemperature(&one_wire_);


PeltierControllerClass::PeltierControllerClass() {
  target_temp_ = 20;
  hysteresis_ = 2;
  heating_ = false;
  set_ramp_speed(255);
}

void PeltierControllerClass::process_wire_command() {
  switch (cmd_) {
  case CMD_SET_TARGET_TEMP:
    if(payload_length_ == sizeof(float)) {
      set_target_temp(read<float>());
      return_code_ = RETURN_OK;
    } else {
      return_code_ = RETURN_BAD_PACKET_SIZE;
    }
    break;
  case CMD_GET_TARGET_TEMP:
    if(payload_length_ == 0) {
      serialize(&target_temp_, sizeof(target_temp_));
      return_code_ = RETURN_OK;
    } else {
      return_code_ = RETURN_BAD_PACKET_SIZE;
    }
    break;
  case CMD_SET_RAMP_SPEED:
    if(payload_length_ == sizeof(uint8_t)) {
      set_ramp_speed(read<uint8_t>());
      return_code_ = RETURN_OK;
    } else {
      return_code_ = RETURN_BAD_PACKET_SIZE;
    }
    break;
  case CMD_GET_RAMP_SPEED:
    if(payload_length_ == 0) {
      serialize(&ramp_speed_, sizeof(ramp_speed_));
      return_code_ = RETURN_OK;
    } else {
      return_code_ = RETURN_BAD_PACKET_SIZE;
    }
    break;
  case CMD_GET_TEMP:
    if(payload_length_ == 0) {
      serialize(&temp_, sizeof(temp_));
      return_code_ = RETURN_OK;
    } else {
      return_code_ = RETURN_BAD_PACKET_SIZE;
    }
    break;
  default:
    BaseNode::process_wire_command();
    break;
  }
}


void PeltierControllerClass::set_target_temp(float target_temp) {
  target_temp_ = target_temp;
  Serial.print("target_temp=");
  Serial.println(target_temp_);
}

void PeltierControllerClass::set_ramp_speed(uint8_t ramp_speed) {
  ramp_speed_ = ramp_speed;
  analogWrite(PIN_PWM, ramp_speed_);
  Serial.print("ramp_speed=");
  Serial.println(ramp_speed_);
}

void PeltierControllerClass::begin() {
  BaseNode::begin();
  pinMode(PIN_PWM, OUTPUT);
  pinMode(PIN_IN_A, OUTPUT);
  pinMode(PIN_IN_B, OUTPUT);
  sensors_.begin();
  if(!sensors_.isParasitePowerMode()) {
    Serial.println("Non-parasitic power mode");
    sensors_.setWaitForConversion(false);
    delay_ = 0;
  } else {
    Serial.println("Parasitic power mode");
    sensors_.setWaitForConversion(true);
    delay_ = 750;
  }
  last_update_ = millis();
}

void PeltierControllerClass::listen() {
  long time;
  do {
    time = millis();
    BaseNode::listen();
  } while(time-last_update_ < delay_);
  last_update_ = time;
  update_temperatures();
  control();
}

void PeltierControllerClass::update_temperatures() {
  sensors_.requestTemperatures(); // Send the command to get temperatures
  temp_ = sensors_.getTempCByIndex(0);
}

void PeltierControllerClass::control() {
  if(debug_) {
    Serial.print(String(millis()) + ", " + String(temp_) + ", " + 
                 String(target_temp_) + ", " + String(ramp_speed_) + ", ");
  }
  if(heating_ && temp_ < target_temp_ || !heating_ && temp_ < target_temp_ - hysteresis_) {
    if(debug_) {
      Serial.println(P("Heating"));
    }
    digitalWrite(PIN_IN_A, HIGH);
    digitalWrite(PIN_IN_B, LOW);
    heating_ = true;
  } else if(!heating_ && temp_ > target_temp_ || heating_ && temp_ > target_temp_ + hysteresis_) {
    if(debug_) {
      Serial.println(P("Cooling"));
    }
    digitalWrite(PIN_IN_A, LOW);
    digitalWrite(PIN_IN_B, HIGH);
    heating_ = false;
  } else {
    if(debug_) {
      Serial.println(P("Holding"));
    }
    digitalWrite(PIN_IN_A, LOW);
    digitalWrite(PIN_IN_B, LOW);
  }  
}

