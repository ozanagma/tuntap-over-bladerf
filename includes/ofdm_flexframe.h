
#ifndef INCLUDES_OFDM_FLEXFRAME_H_
#define INCLUDES_OFDM_FLEXFRAME_H_


#include "../includes/bladerf_configs.h"

void ofdm_flexframe_init(void);

void ofdm_flexframe_receive(framesync_callback _callback, struct bladerf *p_bladerf_device, void* user_data);

int ofdm_flexframe_transmit(unsigned char tx_header[8],  unsigned char* tx_payload, int tx_payload_size, struct bladerf *p_bladerf_device);





#endif /* INCLUDES_OFDM_FLEXFRAME_H_ */
