#include "psu.h"

// #define USE_SERIAL

#ifdef USE_SERIAL
PSU psu;
#else
PSU psu(&Serial);
#endif

void setup()
{
	cpu_irq_enable();
}

void loop()
{
	psu.task();
	uint8_t usb_state = psu.getUsbTaskState();
	if (usb_state == USB_DETACHED_SUBSTATE_WAIT_FOR_DEVICE) {
		// ka3005p not connected.
		return;
	}
	else if (usb_state == USB_STATE_RUNNING) {
		// USB Enumeration is done, do something.
	}
}
