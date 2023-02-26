/**
 ******************************************************************************
 * @file           : mqtt.h
 * @author         : LeBellier
 * @date           :
 * @brief          :
 ******************************************************************************
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#pragma once

/* Exported functions prototypes ---------------------------------------------*/
void input_cb(char* topic, byte* payload, unsigned int length);

void reconnect();

/***************************** END OF FILE ************************************/
