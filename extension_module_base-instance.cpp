/*
 * This file contains all instance-specific definitions for the
 * extension-module-base.
 */
#include "extension_module_base.h"

using namespace microdrop;

ExtensionModuleBase ExtensionModule;

prog_char ExtensionModuleBase::PROTOCOL_NAME_[] PROGMEM = "Extension module protocol";
prog_char ExtensionModuleBase::PROTOCOL_VERSION_[] PROGMEM = "0.1";
prog_char ExtensionModuleBase::MANUFACTURER_[] PROGMEM = "Wheeler Microfluidics Lab";
prog_char ExtensionModuleBase::NAME_[] PROGMEM = "Extension module board";
prog_char ExtensionModuleBase::HARDWARE_VERSION_[] PROGMEM = "1.0";
prog_char ExtensionModuleBase::SOFTWARE_VERSION_[] PROGMEM = "0.1";
prog_char ExtensionModuleBase::URL_[] PROGMEM = "http://microfluidics.utoronto.ca/dropbot";

/*
 * We need to declare the `HandleWireReceive` and `HandleWireRequest` wrapper
 * functions, since the `Wire.onRequest` and `Wire.onReceive` accept function
 * pointers, rather than functors.
 *
 * __NB__ By using a functor class to provide the handler methods, we can more
 *   easily share functionality between the   firmware for different extension
 *   modules.
 */
void HandleWireReceive(int n_bytes) {
    ExtensionModule.wire_handler_->handle_receive(n_bytes);
}

void HandleWireRequest() {
    ExtensionModule.wire_handler_->handle_request();
}
