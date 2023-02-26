/**
 ******************************************************************************
 * @file           : mqtt.cpp
 * @author         : LeBellier
 * @date           : 22/02/2023
 * @brief          :
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "constant.h"
#include "mqtt.h"

/* Exported functions --------------------------------------------------------*/
void input_cb(char* topic, byte* payload, unsigned int length)
{
	Serial.print("Message received on \"");
	Serial.print(topic);
	for (size_t blind_number = 0; blind_number < NB_BLIND; blind_number++)
	{
		Remote* shutter = &shutters[blind_number];
		if (strcmp(topic, shutter->mqtt_topic) == 0)
		{
			Serial.print("\" for id=");
			Serial.print(blind_number);

			if (length == 1)
			{
				char demand = *payload;
				shutters[blind_number].state_to_send = get_action_from_mqtt_cmd(&demand);
				if (shutters[blind_number].state_to_send != NULL)
				{
					Serial.print(" and action is ");
					Serial.println(shutters[blind_number].state_to_send->mqtt_cmd);
				}
			}
			break;
		}
	}
}

void reconnect()
{
	// Loop until we're reconnected
	while (!client.connected())
	{
		Serial.print("Attempting MQTT connection...");
		if (client.connect(clientId, mqtt_user, mqtt_pass))
		{
			Serial.println("connected. ");
			for (int blind_number = 0; blind_number < NB_BLIND; blind_number++)
			{
				client.subscribe(shutters[blind_number].mqtt_topic);
				if (mblen((shutters[blind_number].mqtt_topic_ack), TOPIC_SIZE_MAX) != 0)
					client.subscribe(shutters[blind_number].mqtt_topic_ack);
			}
		}
		else
		{
			Serial.print("failed, rc=");
			Serial.print(client.state());
			Serial.println(" try again in 3 seconds");
			delay(3000);
		}
	}
}

/***************************** END OF FILE ************************************/
