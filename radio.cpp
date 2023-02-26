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
void digital_slot(uint8_t first_level, unsigned int time1, unsigned int time2);
void print_frame(const char* title);

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

void BuildFrame(int blind_number)
{
	Remote* shutter = &shutters[blind_number];

	frame[0] = 0xA7; // Encryption key. Doesn't matter much

	frame[1] = shutter->state_to_send->opcode << 4; // Which button did  you press? The 4 LSB will be the checksum
	frame[2] = shutter->rolling >> 8; // Rolling code (big endian)
	frame[3] = shutter->rolling; // Rolling code
	frame[4] = shutter->remote_address >> 16; // Remote address
	frame[5] = shutter->remote_address >> 8; // Remote address
	frame[6] = shutter->remote_address; // Remote address

	print_frame("Frame         ");

	// Checksum calculation: a XOR of all the nibbles
	byte checksum = 0;

	for (byte i = 0; i < 7; i++)
	{
		checksum = checksum ^ frame[i] ^ (frame[i] >> 4);
	}
	checksum &= 0b1111; // We keep the last 4 bits only

	//Checksum integration
	frame[1] |= checksum; //  If a XOR of all the nibbles is equal to 0, the blinds will
	// consider the checksum ok.

	print_frame("With checksum ");

	// Obfuscation: a XOR of all the bytes
	for (byte i = 1; i < 7; i++)
	{
		frame[i] ^= frame[i - 1];
	}

	print_frame("Obfuscated    ");
	Serial.print("Rolling Code  : ");
	Serial.println(shutter->rolling++);
}

void SendCommandCC1101()
{
	ELECHOUSE_cc1101.SendData(frame, 7, 100);
}

void SendCommand(bool first_frame)
{
	uint8 sync = 7;

	if (first_frame) // Only with the first frame.
	// Wake-up pulse & Silence
	{
		digital_slot(HIGH, 9415, 89565);
		sync = 2;
	}

	// Hardware sync: two sync for the first frame, seven for the following ones.
	for (int i = 0; i < sync; i++)
	{
		digital_slot(HIGH, 4 * SYMBOL, 4 * SYMBOL);
	}

	// Software sync
	digital_slot(HIGH, 4550, SYMBOL);



	//Data: bits are sent one by one, starting with the MSB.
	for (byte i = 0; i < 56; i++)
	{
		if (((frame[i / 8] >> (7 - (i % 8))) & 1) == 1)
			digital_slot(LOW, SYMBOL, SYMBOL);
		else
			digital_slot(HIGH, SYMBOL, SYMBOL);
	}

	digitalWrite(TRANSMIT_PIN, LOW);
	delayMicroseconds(30415); // Inter-frame silence
}

/* Private functions ---------------------------------------------------------*/
void digital_slot(uint8_t first_level, unsigned int time1, unsigned int time2)
{
	digitalWrite(TRANSMIT_PIN, first_level);
	delayMicroseconds(time1);
	digitalWrite(TRANSMIT_PIN, (first_level + 1) % 2);
	delayMicroseconds(time2);
}

void print_frame(const char* title)
{
	Serial.print(title);
	Serial.print(": ");
	for (byte i = 0; i < 7; i++)
	{
		if (frame[i] >> 4 == 0) //  Displays leading zero in case the most significant
		{
			Serial.print("0"); // nibble is a 0.
		}
		Serial.print(frame[i], HEX);
		Serial.print(" ");
	}
	Serial.println("");
}

/***************************** END OF FILE ************************************/
