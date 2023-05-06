#include "psu.h"

PSU::PSU(USBHost* usb, Stream* serial) : pCdc(usb, serial)
{
}

void PSU::setOCP(bool state)
{
	sendBool(OCP, state);
}

void PSU::setOutput(bool state)
{
	sendBool(SETTING_OUTPUT, state);
}

void PSU::setOVP(bool state)
{
	sendBool(OVP, state);
}

void PSU::setBeep(bool state)
{
	sendBool(SETTING_BEEP, state);
}

void PSU::setCurrent(const char* channel, float value)
{
	sendFloat(SETTING_OUTPUT_CURRENT, channel, value);
}

void PSU::setVoltage(const char* channel, float value)
{
	sendFloat(SETTING_OUTPUT_VOLTAGE, channel, value);
}

float PSU::getActualCurrent(const char* channel)
{
	return get(ACTUAL_OUTPUT_CURRENT, channel);
}

float PSU::getActualVoltage(const char* channel)
{
	return get(ACTUAL_OUTPUT_VOLTAGE, channel);
}

float PSU::getCurrent(const char* channel)
{
	return get(SETTING_OUTPUT_CURRENT, channel);
}

float PSU::getVoltage(const char* channel)
{
	return get(SETTING_OUTPUT_VOLTAGE, channel);
}

uint8_t PSU::getStatus()
{
	pCdc.SndData(sizeof(STATUS GETTER), (uint8_t*)STATUS GETTER);

	delay(50); // give some time to receive the response.

	uint8_t rx_buffer[10];
	uint32_t rcvd = sizeof(rx_buffer);

	pCdc.RcvData(&rcvd, rx_buffer);

	this->status = rx_buffer[0];

	return this->status;
}

bool PSU::isCVMode()
{
	return (status & MODE_MASK) != 0;
}

bool PSU::isCCMode()
{
	return !isCVMode();
}

bool PSU::isBeepOn()
{
	return (status & BEEP_MASK) != 0;
}

bool PSU::isOCPOn()
{
	return (status & OCP_MASK) != 0;
}

bool PSU::isOutputOn()
{
	return (status & OUTPUT_MASK) != 0;
}

bool PSU::isOVPOn()
{
	return (status & OVP_MASK) != 0;
}

// Private Methods

float PSU::get(const char* command, const char* channel)
{
	// Setup output frame
	char tx_buffer[PSU_BUFFER_LENGTH] = { 0 };
	char* w_ptr                       = tx_buffer;

	strcpy(w_ptr, command);
	w_ptr   += strlen(command);
	*w_ptr++ = channel[0];
	*w_ptr++ = GETTER[0];
	uint32_t rcode = pCdc.SndData(w_ptr - tx_buffer, (uint8_t*)tx_buffer);

	if (rcode) {
		return 31.0; // PSU provides up to 30V, 31 is consider to be an invalid/error value.
	}

	delay(50); // give some time to receive the response.

	uint8_t rx_buffer[PSU_BUFFER_LENGTH];
	uint32_t rcvd = sizeof(rx_buffer);

	rcode = pCdc.RcvData(&rcvd, rx_buffer);

	if (rcode) {
		return 32.0;
	}

	rx_buffer[5] = 0; // add null terminator to form a string
	return atof((char*)rx_buffer);
}

void PSU::sendBool(const char* command, bool state)
{
	char tx_buffer[PSU_BUFFER_LENGTH] = { 0 };

	strcpy(tx_buffer, command);
	*(tx_buffer + strlen(command)) = state ? '1' : '0';

	pCdc.SndData(strlen(tx_buffer), (uint8_t*)tx_buffer);
}

void PSU::sendFloat(const char* command, const char* channel, float value)
{
	char tx_buffer[PSU_BUFFER_LENGTH] = { 0 };     // Final string frame.
	char* w_ptr                       = tx_buffer; // Pointer to the first index of tx_buffer

	strcpy(tx_buffer, command);
	w_ptr   += strlen(command);
	*w_ptr++ = channel[0];
	*w_ptr++ = SEPARATOR[0];
	w_ptr   += snprintf(w_ptr, PSU_BUFFER_LENGTH - (w_ptr - tx_buffer) - 1, "%.2f", value);
	pCdc.SndData(w_ptr - tx_buffer, (uint8_t*)tx_buffer);
}
