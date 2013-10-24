#include "extension_module_base.h"
#include <SPI.h>
#include <Wire.h>
#include "Memory.h"

using namespace microdrop;

extern ExtensionModuleBase ExtensionModule;

void setup() {
  ExtensionModule.begin();
}

void loop() {
  ExtensionModule.Listen();
}
