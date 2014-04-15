#ifndef ___PELTIER_CONTROLLER__H___
#define ___PELTIER_CONTROLLER__H___

#include <DallasTemperature.h>
#include <OneWire.h>
#include <ExtensionModuleBase.h>

class PeltierControllerClass : public ExtensionModuleBase {
public:
  // digital pins
  static const uint8_t PIN_IN_A = 7;
  static const uint8_t PIN_IN_B = 8;
  static const uint8_t PIN_PWM = 9;
  static const uint8_t ONE_WIRE_BUS = 2;  

  // analog pins
  static const uint8_t PIN_CS = 0; // A0
  
  // command codes
  static const uint8_t CMD_SET_TARGET_TEMP = 0xA0;
  static const uint8_t CMD_GET_TARGET_TEMP = 0xA1;
  static const uint8_t CMD_SET_RAMP_SPEED =  0xA2;
  static const uint8_t CMD_GET_RAMP_SPEED =  0xA3;
  static const uint8_t CMD_GET_TEMP =        0xA4;
  static const uint8_t CMD_GET_CURRENT =     0xA5;

  static OneWire one_wire_;
  static DallasTemperature sensors_;

  PeltierControllerClass();
  void begin();
  void listen();
  void process_wire_command();
private:
  void set_target_temp(float target_temp);
  void set_ramp_speed(uint8_t ramp_speed);
  void update_temperatures();
  void control();

  float target_temp_;
  uint8_t ramp_speed_;
  float temp_;
  bool heating_;
  float hysteresis_;
  uint16_t delay_;
  long last_update_;
};

extern PeltierControllerClass PeltierController;

#endif // ___PELTIER_CONTROLLER__H___
