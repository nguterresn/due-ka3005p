#include "psu.h"

USBHost Usb;
PSU psu(&Usb, &Serial);

void setup()
{
	cpu_irq_enable();
}

void loop()
{
	Usb.Task();
	uint8_t usb_state = Usb.getUsbTaskState();
	if (usb_state == USB_DETACHED_SUBSTATE_WAIT_FOR_DEVICE) {
		// ka3005p not connected.
		return;
	}
	else if (usb_state == USB_STATE_RUNNING) {
		// USB Enumeration is done, do something.
	}
}
