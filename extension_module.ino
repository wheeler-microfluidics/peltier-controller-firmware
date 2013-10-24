#include <SPI.h>
#include <Wire.h>
#include "Memory.h"
#include "extension_module_base.h"

using namespace microdrop;

extern ExtensionModuleBase ExtensionModule;

void setup() {
  ExtensionModule.begin();
}

void loop() {
  ExtensionModule.Listen();
}
