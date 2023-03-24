/**
 ******************************************************************************
 * @file           : radio.cpp
 * @author         : LeBellier
 * @date           :
 * @brief          :
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include <Arduino.h>
#include "constant.h"
#include "radio.h"
#include <ELECHOUSE_CC1101_SRC_DRV.h>

/* Private function prototypes -----------------------------------------------*/
void printFrame(byte* frame);
void buildFrame(int blind_number);
void sendFrame(byte sync);
void sendHigh(uint16_t durationInMicroseconds);
void sendLow(uint16_t durationInMicroseconds);

/* Private variables ---------------------------------------------------------*/
byte frame[7];

/* Exported functions --------------------------------------------------------*/
void radio_setup()
{
	pinMode(TRANSMIT_PIN, OUTPUT); // Pin D1 on the Wemos D1 mini
	digitalWrite(TRANSMIT_PIN, LOW);

	if (ELECHOUSE_cc1101.getCC1101()) // Check the CC1101 Spi connection.
		Serial.println("Connection OK");
	else
		Serial.println("Connection Error");

	ELECHOUSE_cc1101.Init(); // must be set to initialize the cc1101!
	ELECHOUSE_cc1101.setCCMode(1); // set config for internal transmission mode.
	ELECHOUSE_cc1101.setModulation(0); // set modulation mode. 0 = 2-FSK, 1 = GFSK, 2 = ASK/OOK, 3 = 4-FSK, 4 = MSK.
	ELECHOUSE_cc1101.setMHZ(433.42); // Here you can set your basic frequency. The lib calculates the frequency automatically (default = 433.92).The cc1101 can: 300-348 MHZ, 387-464MHZ and 779-928MHZ. Read More info from datasheet.
	ELECHOUSE_cc1101.setSyncMode(2); // Combined sync-word qualifier mode. 0 = No preamble/sync. 1 = 16 sync word bits detected. 2 = 16/16 sync word bits detected. 3 = 30/32 sync word bits detected. 4 = No preamble/sync, carrier-sense above threshold. 5 = 15/16 + carrier-sense above threshold. 6 = 16/16 + carrier-sense above threshold. 7 = 30/32 + carrier-sense above threshold.
	// ELECHOUSE_cc1101.setPA(10);      // set TxPower. The following settings are possible depending on the frequency band.  (-30  -20  -15  -10  -6    0    5    7    10   11   12) Default is max!
	// ELECHOUSE_cc1101.setCrc(1); // 1 = CRC calculation in TX and CRC check in RX enabled. 0 = CRC disabled for TX and RX.

	Serial.println("Tx Mode");
}

void sendCC1101Command(int blind_number, int repeat)
{
	ELECHOUSE_cc1101.SetTx();
	sendCommand(blind_number, repeat);
	ELECHOUSE_cc1101.setSidle();
}

void sendCommand(int blind_number, int repeat)
{
	buildFrame(blind_number);
	sendFrame(2);
	for (int i = 0; i < repeat; i++)
	{
		sendFrame(7);
	}
}

/* Private functions ---------------------------------------------------------*/
void printFrame(byte* frame)
{
	for (byte i = 0; i < 7; i++)
	{
		if (frame[i] >> 4 == 0) //  Displays leading zero in case the most significant
		{
			Serial.print("0"); // nibble is a 0.
		}
		Serial.print(frame[i], HEX);
		Serial.print(" ");
	}
	Serial.println();
}

void buildFrame(int blind_number)
{
	Remote* shutter = &shutters[blind_number];

	frame[0] = 0xA7; // Encryption key. Doesn't matter much

	frame[1] = shutter->state_to_send->opcode << 4; // Which button did  you press? The 4 LSB will be the checksum
	frame[2] = shutter->rolling >> 8; // Rolling code (big endian)
	frame[3] = shutter->rolling; // Rolling code
	frame[4] = shutter->remote_address >> 16; // Remote address
	frame[5] = shutter->remote_address >> 8; // Remote address
	frame[6] = shutter->remote_address; // Remote address

#ifdef DEBUG
	Serial.print("Frame         : ");
	printFrame(frame);
#endif

	// Checksum calculation: a XOR of all the nibbles
	byte checksum = 0;
	for (byte i = 0; i < 7; i++)
	{
		checksum = checksum ^ frame[i] ^ (frame[i] >> 4);
	}
	checksum &= 0b1111; // We keep the last 4 bits only

	// Checksum integration
	frame[1] |= checksum;

#ifdef DEBUG
	Serial.print("With checksum : ");
	printFrame(frame);
#endif

	// Obfuscation: a XOR of all the bytes
	for (byte i = 1; i < 7; i++)
	{
		frame[i] ^= frame[i - 1];
	}

#ifdef DEBUG
	Serial.print("Obfuscated    : ");
	printFrame(frame);
#endif
	Serial.print("Rolling Code  : ");
	Serial.println(shutter->rolling++);
}

void sendFrame(byte sync)
{
	if (sync == 2) // Only with the first frame.
	// Wake-up pulse & Silence
	{
		sendHigh(9415);
		sendLow(9565);
		delay(80);
	}

	// Hardware sync: two sync for the first frame, seven for the following ones.
	for (int i = 0; i < sync; i++)
	{
		sendHigh(4 * SYMBOL);
		sendLow(4 * SYMBOL);
	}

	// Software sync
	sendHigh(4550);
	sendLow(SYMBOL);

	// Data: bits are sent one by one, starting with the MSB.
	for (byte i = 0; i < 56; i++)
	{
		if (((frame[i / 8] >> (7 - (i % 8))) & 1) == 1)
		{
			sendLow(SYMBOL);
			sendHigh(SYMBOL);
		}
		else
		{
			sendHigh(SYMBOL);
			sendLow(SYMBOL);
		}
	}

	// Inter-frame silence
	sendLow(415);
	delay(30);
}

void sendHigh(uint16_t durationInMicroseconds)
{
	digitalWrite(TRANSMIT_PIN, HIGH);
	delayMicroseconds(durationInMicroseconds);
}

void sendLow(uint16_t durationInMicroseconds)
{
	digitalWrite(TRANSMIT_PIN, LOW);
	delayMicroseconds(durationInMicroseconds);
}

/***************************** END OF FILE ************************************/
