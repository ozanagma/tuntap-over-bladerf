/*
 * flexframe.h
 *
 *  Created on: Apr 20, 2019
 *      Author: thedog
 */

#ifndef INCLUDES_FLEXFRAME_H_
#define INCLUDES_FLEXFRAME_H_


#include "../includes/blade_rf.h"


void FlexFrameReceive(framesync_callback _callback, struct bladerf *p_bladerf_device);

int FlexFrameTransmit(unsigned char tx_header[8],  unsigned char* tx_payload, int tx_payload_size, struct bladerf *p_bladerf_device);




#endif /* INCLUDES_FLEXFRAME_H_ */
