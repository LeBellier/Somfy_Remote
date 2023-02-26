/* This sketch allows you to emulate a Somfy RTS or Simu HZ remote.

   This is a fork of MakerMeik's project, to add functionality to control multiple (up to 20) blinds.
   (https://github.com/MakerMeik/Somfy_Remote)

   MakerMeik's code was forked from the original sketch written by Nickduino (https://github.com/Nickduino)

   If you want to learn more about the Somfy RTS protocol, check out https://pushstack.wordpress.com/somfy-rts-protocol/

   The rolling code will be stored in EEPROM, so that you can power the D1 Mini.

   Easiest way to make it work for you:
    - Choose a remote number
    - Choose a starting point for the rolling code. Any unsigned int works, 1 is a good start
    - Upload the sketch
    - Long-press the program button of YOUR ACTUAL REMOTE until your blind goes up and down slightly
    - send 'p' via 'MQTT'
   To make a group command, just repeat the last two steps with another blind (one by one)

   Send a message to the channel number corresponding to the blind you want to program / control.

   e.g. "Somfy-1"

   From the command line, this is:
   mosquitto_pub -h <mqtt server IP address> -m "u" -t "Somfy-1"

   Then:
    - u will make it to go up
    - s make it stop
    - d will make it to go down
    - p sets the program mode
    - you can also send a HEX number directly for any weird command you (0x9 for the sun and wind detector for instance)
 */


#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#include "constant.h"
#include "radio.h"
#include "mqtt.h"

void setup()
{
	Serial.begin(115200);
	Serial.println(" ");
	Serial.println("Starting Somfy");
	Serial.println();
	Serial.print("Connecting to ");
	Serial.println(ssid);
	WiFi.mode(WIFI_STA);
	WiFi.begin(ssid, password);

	while (WiFi.status() != WL_CONNECTED)
	{
		delay(1000);
		Serial.print(".");
	}
	Serial.println("");
	Serial.println("WiFi connected");

	Serial.println(WiFi.localIP());

	radio_setup();

	client.setServer(mqtt_server, mqtt_port);
	client.setCallback(input_cb);

	EEPROM.begin(4 * NB_BLIND);
	Serial.println(" ");
	for (int blind_number = 0; blind_number < NB_BLIND; blind_number++)
	{
		Serial.print("Simulated remote number : ");
		Serial.print(shutters[blind_number].remote_address, HEX);
		Serial.print(" with Current rolling code : ");
		// Erase EEPROM
		// EEPROM.put(EEPROM_ADDRESS + (4 * blind_number), shutters[blind_number].rolling);
		// EEPROM.commit();
		EEPROM.get(EEPROM_ADDRESS + (4 * blind_number), shutters[blind_number].rolling);
		Serial.println(shutters[blind_number].rolling);
	}
}

void loop()
{
	if (!client.connected())
	{
		reconnect();
	}
	client.loop();

	for (int blind_number = 0; blind_number < NB_BLIND; blind_number++)
	{
		Remote* shutter = &shutters[blind_number];
		if (shutter->state_to_send != NULL)
		{
			BuildFrame(blind_number);

			if (mblen((shutter->mqtt_topic_ack), TOPIC_SIZE_MAX) != 0)
				client.publish(shutter->mqtt_topic_ack, shutter->state_to_send->mqtt_ack);
			Serial.println(shutter->state_to_send->log);

			//  We store the value of the rolling code in the
			// EEPROM. It should take up to 2 adresses but the
			// Arduino function takes care of it.
			EEPROM.put(EEPROM_ADDRESS + (4 * blind_number), shutter->rolling);
			EEPROM.commit();

			shutter->state_to_send = NULL;

			Serial.println("");
			SendCommand(true);
			for (int j = 0; j < 2; j++)
			{
				SendCommand(false);
			}
		}
	}
}
