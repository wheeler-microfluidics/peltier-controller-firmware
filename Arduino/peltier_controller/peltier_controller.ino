#include <SPI.h>
#include <Wire.h>
#include <DallasTemperature.h>
#include <OneWire.h>
#include "PeltierController.h"

void setup() {
  PeltierController.begin();
}

void loop() {
  PeltierController.listen();
}
