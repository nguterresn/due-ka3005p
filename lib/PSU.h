#ifndef PSU_H_INCLUDED
#define PSU_H_INCLUDED

#include <Arduino.h>
#include <Usb.h>
#include "cdc.h"

#define PSU_BUFFER_LENGTH      15

// Channel ASCII
#define CHANNEL_1              "1"
#define CHANNEL_2              "2"
#define CHANNEL_3              "3"
#define CHANNEL_4              "4"

// Commands
// https://www.distrelec.hu/Web/Downloads/_t/ds/RND%20320-KA%20Control%20Commands_eng_tds.pdf
#define SETTING_OUTPUT_CURRENT "ISET"
#define SETTING_OUTPUT_VOLTAGE "VSET"
#define ACTUAL_OUTPUT_CURRENT  "IOUT"
#define ACTUAL_OUTPUT_VOLTAGE  "VOUT"
#define SETTING_BEEP           "BEEP"
#define SETTING_OUTPUT         "OUT"
#define STATUS                 "STATUS"
#define OCP                    "OCP"
#define OVP                    "OVP"

// Command helpers
#define SEPARATOR              ":"
#define GETTER                 "?"

// Status Mask
#define MODE_MASK              0x01 // 0b0000_0001
#define BEEP_MASK              0x10 // 0b0001_0000
#define OCP_MASK               0x20 // 0b0010_0000
#define OUTPUT_MASK            0x40 // 0b0100_0000
#define OVP_MASK               0x80 // 0b1000_0000

class PSU
{
public:
	PSU(USBHost* usb, Stream* serial = nullptr);
	// Commands
	float getActualCurrent(const char* channel);
	float getActualVoltage(const char* channel);
	float getCurrent(const char* channel);
	float getVoltage(const char* channel);
	uint8_t getStatus();
	void setCurrent(const char* channel, float value);
	void setVoltage(const char* channel, float value);
	void setOCP(bool state);
	void setOutput(bool state);
	void setOVP(bool state);
	void setBeep(bool state);
	// Status
	bool isCCMode();
	bool isCVMode();
	bool isBeepOn();
	bool isOCPOn();
	bool isOutputOn();
	bool isOVPOn();

private:
	CDC pCdc;
	uint8_t status;

	float get(const char* command, const char* channel);
	void sendBool(const char* command, bool state);
	void sendFloat(const char* command, const char* channel, float value);
};

#endif /* PSU_H_INCLUDED */
