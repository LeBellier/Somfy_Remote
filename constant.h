/**
 ******************************************************************************
 * @file           : constant.h
 * @author         : LeBellier
 * @date           : 22/02/2023
 * @brief          : Group all global const and variable
 ******************************************************************************
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#pragma once

/* Includes ------------------------------------------------------------------*/
#include "c_types.h"
#include "WString.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

/* Exported types ------------------------------------------------------------*/
struct Blind_Action
{
	const char* mqtt_cmd;
	const char* mqtt_ack;
	const char* log;
	int8      opcode;
};

struct Remote
{
	uint32      remote_address;
	const char  * mqtt_topic;
	const char  * mqtt_topic_ack;
	const char  * description;
	Blind_Action* state_to_send;
	uint16      rolling;
};

/* Exported constants --------------------------------------------------------*/
#define SYMBOL            640
#define EEPROM_ADDRESS    0
#define TRANSMIT_PIN      14

#define NB_BLIND          7
#define TOPIC_SIZE_MAX    100


extern Remote shutters[NB_BLIND];
extern const char* ssid;
extern const char* password;

extern const char* mqtt_server;
extern const unsigned int mqtt_port;
extern const char* mqtt_user;
extern const char* mqtt_pass;
extern const char* clientId;

extern WiFiClient espClient;
extern PubSubClient client;

/* Exported function --------------------------------------------------------*/
Blind_Action* get_action_from_mqtt_cmd(char* mqtt_cmd);

/***************************** END OF FILE ************************************/
