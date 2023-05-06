/* Copyright (C) 2011 Circuits At Home, LTD. All rights reserved.

This software may be distributed and modified under the terms of the GNU
General Public License version 2 (GPL2) as published by the Free Software
Foundation and appearing in the file GPL2.TXT included in the packaging of
this file. Please note that GPL2 Section 2[b] requires that all works based
on this software must also be made publicly available under the terms of
the GPL2 ("Copyleft").

Contact information
-------------------

Circuits At Home, LTD
Web      :  http://www.circuitsathome.com
e-mail   :  support@circuitsathome.com

* Copyright (c) Kisi Incorporated 2023-present

This code was slightly changed to comply with our requirements.
*/

#ifndef CDC_H_INCLUDED
#define CDC_H_INCLUDED

#include <stdint.h>
#include "usb_ch9.h"
#include "Usb.h"
#include "hid.h"
#include "confdescparser.h"

#define totalEndpoints 3

/**
 * \class CDC definition, class type USB_CLASS_COM_AND_CDC_CTRL.
 */
class CDC : public USBDeviceConfig, public UsbConfigXtracter
{
private:
	static const uint32_t epDataInIndex;  // DataIn endpoint index
	static const uint32_t epDataOutIndex; // DataOUT endpoint index

	/* Mandatory members */
	USBHost* pUsb;
	uint32_t bAddress;  // Device USB address
	uint32_t bConfNum;  // configuration number

	uint32_t bNumEP;    // total number of EP in the configuration

	Stream* serial;

	/* Endpoint data structure describing the device EP */
	EpInfo epInfo[totalEndpoints];

public:
	CDC(USBHost* pUsb, Stream* serial = nullptr);

	// Methods for receiving and sending data
	uint32_t SndData(uint32_t nbytes, uint8_t* dataptr);
	uint32_t RcvData(uint32_t* bytes_rcvd, uint8_t* dataptr);

	// USBDeviceConfig implementation
	virtual uint32_t Init(uint32_t parent, uint32_t port, uint32_t lowspeed);
	virtual uint32_t Release();
	virtual uint32_t Poll()
	{
		return 0;
	};
	virtual uint32_t GetAddress()
	{
		return bAddress;
	};

	virtual void EndpointXtract(uint32_t conf, uint32_t iface, uint32_t alt, uint32_t proto, const USB_ENDPOINT_DESCRIPTOR* ep);
};

#endif /* CDC_H_INCLUDED */
