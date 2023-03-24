/**
 ******************************************************************************
 * @file           : radio.h
 * @author         : LeBellier
 * @date           :
 * @brief          :
 ******************************************************************************
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#pragma once

/* Exported functions prototypes ---------------------------------------------*/
void radio_setup();
void sendCC1101Command(int blind_number, int repeat);
void sendCommand(int blind_number, int repeat);

/***************************** END OF FILE ************************************/
