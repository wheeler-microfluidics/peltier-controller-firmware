#include <SPI.h>
#include <Wire.h>

#include <DallasTemperature.h>
#include <OneWire.h>
#include <ExtensionModuleBase.h>

#include "PeltierController.h"

void setup() {
  PeltierController.begin();
}

void loop() {
  PeltierController.listen();
}
