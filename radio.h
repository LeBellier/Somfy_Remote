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
void BuildFrame(int blind_number);

void SendCommandCC1101();
void SendCommand(bool first_frame);

/***************************** END OF FILE ************************************/
