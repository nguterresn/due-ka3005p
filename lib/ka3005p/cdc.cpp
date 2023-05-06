/* Copyright (C) 2011 Circuits At Home, LTD. All rights reserved.
 *
 * This software may be distributed and modified under the terms of the GNU
 * General Public License version 2 (GPL2) as published by the Free Software
 * Foundation and appearing in the file GPL2.TXT included in the packaging of
 * this file. Please note that GPL2 Section 2[b] requires that all works based
 * on this software must also be made publicly available under the terms of
 * the GPL2 ("Copyleft").
 *
 * Contact information
 * -------------------
 *
 * Circuits At Home, LTD
 * Web      :  http://www.circuitsathome.com
 * e-mail   :  support@circuitsathome.com
 *
 * Copyright (c) Kisi Incorporated 2023-present
 *
 * This code was slightly changed to comply with our requirements.
 */

#include "cdc.h"

const uint32_t CDC::epDataInIndex  = 1;
const uint32_t CDC::epDataOutIndex = 2;

CDC::CDC(Stream* serial) : bAddress(0), bNumEP(1), serial(serial)
{
	// Initialize endpoint data structures
	for (uint32_t i = 0; i < totalEndpoints; ++i) {
		epInfo[i].deviceEpNum = 0;
		epInfo[i].hostPipeNum = 0;
		epInfo[i].maxPktSize  = (i) ? 0 : 8;
		epInfo[i].epAttribs   = 0;
		epInfo[i].bmNakPower  = (i) ? USB_NAK_NOWAIT : USB_NAK_MAX_POWER;
	}

	// Register in USB subsystem
  Usb.RegisterDeviceClass(this);
}

uint32_t CDC::Init(uint32_t parent, uint32_t port, uint32_t lowspeed)
{
	uint8_t buf[sizeof(USB_DEVICE_DESCRIPTOR)];
	uint32_t rcode       = 0;
	UsbDevice* p         = NULL;
	EpInfo* oldep_ptr    = NULL;
	uint32_t num_of_conf = 0;

	// Get memory address of USB device address pool
	AddressPool& addrPool = Usb.GetAddressPool();

	// Check if address has already been assigned to an instance
	if (bAddress) {
		return USB_ERROR_CLASS_INSTANCE_ALREADY_IN_USE;
	}

	// Get pointer to pseudo device with address 0 assigned
	p = addrPool.GetUsbDevicePtr(0);

	if (!p) {
		return USB_ERROR_ADDRESS_NOT_FOUND_IN_POOL;
	}

	if (!p->epinfo) {
		return USB_ERROR_EPINFO_IS_NULL;
	}

	// Save old pointer to EP_RECORD of address 0
	oldep_ptr = p->epinfo;
	// Temporary assign new pointer to epInfo to p->epinfo in order to avoid toggle inconsistence
	p->epinfo   = epInfo;
	p->lowspeed = lowspeed;
	// Get device descriptor
	rcode = Usb.getDevDescr(0, 0, sizeof(USB_DEVICE_DESCRIPTOR), (uint8_t*)buf);
	// Restore p->epinfo
	p->epinfo = oldep_ptr;

	if (rcode) {
		goto FailGetDevDescr;
	}

	// Allocate new address according to device class
	bAddress = addrPool.AllocAddress(parent, false, port);
	// Extract Max Packet Size from device descriptor
	epInfo[0].maxPktSize = (uint8_t)((USB_DEVICE_DESCRIPTOR*)buf)->bMaxPacketSize0;

	// Assign new address to the device
	rcode = Usb.setAddr(0, 0, bAddress);
	if (rcode) {
		p->lowspeed = false;
		addrPool.FreeAddress(bAddress);
		bAddress = 0;
		return rcode;
	}

	p->lowspeed = false;

	//get pointer to assigned address record
	p = addrPool.GetUsbDevicePtr(bAddress);
	if (!p) {
		return USB_ERROR_ADDRESS_NOT_FOUND_IN_POOL;
	}

	p->lowspeed = lowspeed;

	// Assign epInfo to epinfo pointer - only EP0 is known
	rcode = Usb.setEpInfoEntry(bAddress, 1, epInfo);
	if (rcode) {
		goto FailSetDevTblEntry;
	}

	num_of_conf = ((USB_DEVICE_DESCRIPTOR*)buf)->bNumConfigurations;

	for (uint8_t i = 0; i < num_of_conf; i++) {
		// Class ID needs to be USB_CLASS_CDC_DATA and MASK 0xff
		ConfigDescParser<USB_CLASS_CDC_DATA, 0, 0, 0xFF> confDescrParser(this);

		delay(1);
		rcode = Usb.getConfDescr(bAddress, 0, i, &confDescrParser);

		if (rcode) {
			goto FailGetConfDescr;
		}

		if (bNumEP > 2) {
			break;
		}
	}

	if (bNumEP == 3) {
		// Assign epInfo to epinfo pointer - this time all 3 endpoins
		rcode = Usb.setEpInfoEntry(bAddress, 3, epInfo);
		if (rcode) {
			goto FailSetDevTblEntry;
		}
	}

	// Set Configuration Value
	rcode = Usb.setConf(bAddress, 0, bConfNum);

	if (rcode) {
		goto FailSetConfDescr;
	}

	return 0;

	// Diagnostic messages
FailGetDevDescr:
	goto Fail;

FailSetDevTblEntry:
	goto Fail;

FailGetConfDescr:
	goto Fail;

FailSetConfDescr:
	goto Fail;

Fail:
	Release();
	return rcode;
}

void CDC::EndpointXtract(uint32_t conf, uint32_t iface, uint32_t alt, uint32_t proto, const USB_ENDPOINT_DESCRIPTOR* pep)
{
	if (bNumEP == 3) {
		return;
	}

	bConfNum = conf;

	uint32_t index = 0;
	uint32_t pipe  = 0;

	if ((pep->bmAttributes & 0x02) == 2) {
		index = ((pep->bEndpointAddress & 0x80) == 0x80) ? epDataInIndex : epDataOutIndex;
	}

	// Fill in the endpoint info structure
	epInfo[index].deviceEpNum = pep->bEndpointAddress & 0x0F;
	epInfo[index].maxPktSize  = pep->wMaxPacketSize;

	if (index == epDataInIndex) {
		pipe = UHD_Pipe_Alloc(bAddress, epInfo[index].deviceEpNum, UOTGHS_HSTPIPCFG_PTYPE_BLK, UOTGHS_HSTPIPCFG_PTOKEN_IN, epInfo[index].maxPktSize, 0, UOTGHS_HSTPIPCFG_PBK_1_BANK);
	}
	else if (index == epDataOutIndex) {
		pipe = UHD_Pipe_Alloc(bAddress, epInfo[index].deviceEpNum, UOTGHS_HSTPIPCFG_PTYPE_BLK, UOTGHS_HSTPIPCFG_PTOKEN_OUT, epInfo[index].maxPktSize, 0, UOTGHS_HSTPIPCFG_PBK_1_BANK);
	}

	// Ensure pipe allocation is okay
	if (pipe == 0) {
		// Enumeration failed, so user should not perform write/read since isConnected will return false
		return;
	}

	epInfo[index].hostPipeNum = pipe;

	bNumEP++;
}

uint32_t CDC::Release()
{
	// Free allocated host pipes
	UHD_Pipe_Free(epInfo[epDataInIndex].hostPipeNum);
	UHD_Pipe_Free(epInfo[epDataOutIndex].hostPipeNum);

	// Free allocated USB address
	Usb.GetAddressPool().FreeAddress(bAddress);

	// Must have to be reset to 1
	bNumEP = 1;

	bAddress = 0;

	return 0;
}

uint32_t CDC::RcvData(uint32_t* bytes_rcvd, uint8_t* dataptr)
{
	return Usb.inTransfer(bAddress, epInfo[epDataInIndex].deviceEpNum, bytes_rcvd, dataptr);
}

uint32_t CDC::SndData(uint32_t nbytes, uint8_t* dataptr)
{
	return Usb.outTransfer(bAddress, epInfo[epDataOutIndex].deviceEpNum, nbytes, dataptr);
}
