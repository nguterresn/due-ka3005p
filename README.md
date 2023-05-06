## Arduino Due + KA3005P

Library (run on PlatformIO) to allow the communication between the Due and the power supply ka3005p.

The key to make this board communicate with the power supply is to have a proper implementation of the USBHost Arduino library adapted to the characteristics ([CDC](https://en.wikipedia.org/wiki/USB_communications_device_class)) of the ka3005p USB interface.

This is partially based on the libraries:

- [USB Host 2.0](https://github.com/felis/USB_Host_Shield_2.0)
- [KA3005P](https://github.com/Nicoretti/ka3005p)

