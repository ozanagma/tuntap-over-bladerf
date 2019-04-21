/*
 * ofdmflexframe.h
 *
 *  Created on: Apr 20, 2019
 *      Author: thedog
 */

#ifndef INCLUDES_OFDMFLEXFRAME_H_
#define INCLUDES_OFDMFLEXFRAME_H_


#include "../includes/blade_rf.h"

void OfdmFlexFrameInit(void);

void OfdmFlexFrameReceive(framesync_callback _callback, struct bladerf *p_bladerf_device, void* user_data);

int OfdmFlexFrameTransmit(unsigned char tx_header[8],  unsigned char* tx_payload, int tx_payload_size, struct bladerf *p_bladerf_device);





#endif /* INCLUDES_OFDMFLEXFRAME_H_ */
