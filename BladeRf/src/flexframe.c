/*
 * flexframe.c
 *
 *  Created on: Apr 20, 2019
 *      Author: thedog
 */

#include "../includes/flexframe.h"
#include <stdio.h>




void FlexFrameReceive(framesync_callback _callback, struct bladerf *p_bladerf_device){

    int status;
    unsigned int frame_counter = 0;
    flexframesync fs;

    fs = flexframesync_create(_callback, &frame_counter);
    status =  BladeRFSyncRx(p_bladerf_device, fs, 0);

    flexframesync_destroy(fs);
}

int FlexFrameTransmit(unsigned char tx_header[8],  unsigned char* tx_payload, int tx_payload_size, struct bladerf *p_bladerf_device){


	int status = 0;
	int lastpos = 0;
	int16_t *tx_samples;

	float complex y[BUFFER_SIZE];          // frame samples
	unsigned int  buf_len = PAYLOAD_LENGTH;
	float complex buf[buf_len];
	unsigned int symbol_len;
	int frame_complete = 0;



	flexframegenprops_s ffp;
	flexframegenprops_init_default(&ffp);
	//ffp.check = false;
	ffp.fec0 = LIQUID_FEC_NONE;
	ffp.fec1 = LIQUID_FEC_NONE;
	ffp.mod_scheme = LIQUID_MODEM_QAM4;

	flexframegen fg = flexframegen_create(&ffp);
	//flexframegen_print(fg);


	flexframegen_assemble(fg, tx_header, tx_payload, tx_payload_size);
	symbol_len = flexframegen_getframelen(fg);

	frame_complete = 0;
	lastpos = 0;
	while (!frame_complete) {
		frame_complete = flexframegen_write_samples(fg, buf, buf_len);
		memcpy(&y[lastpos], buf, buf_len*sizeof(float complex));
		lastpos = lastpos + buf_len;
	}

	printf("number of samples %u %u\n", symbol_len, lastpos);
	//tx_samples = convert_comlexfloat_to_sc16q11( y, symbol_len );


	tx_samples = NULL;
	tx_samples = (int16_t *)malloc(symbol_len * 2 * sizeof(int16_t));
    if (tx_samples != NULL) {
		for(int i = 0; i < symbol_len ; i++){
			tx_samples[2*i]= round( crealf(y[i]) * 2048); // Since bladeRF uses Q4.11 complex 2048=2^11
			tx_samples[2*i+1]= round( cimagf(y[i]) * 2048);
				if ( tx_samples[2*i] > 2047  ) tx_samples[2*i]=2047;
				if ( tx_samples[2*i] < -2048  ) tx_samples[2*i]=-2048;
				if ( tx_samples[2*i+1] > 2047  ) tx_samples[2*i+1]=2047;
				if ( tx_samples[2*i+1] < -2048  ) tx_samples[2*i+1]=-2048;
		}
    }

	if (tx_samples == NULL) {
		fprintf(stdout, "malloc error: %s\n", bladerf_strerror(status));
		//return BLADERF_ERR_MEM;
	}

	status =  BladeRFSyncTx(p_bladerf_device, tx_samples, symbol_len);
	if (status != 0) {
		fprintf(stderr, "Failed to sync_tx(). Exiting. %s\n", bladerf_strerror(status));
	}
	fprintf(stdout, "TxPacket: %s\n", tx_payload);
	usleep(10000);

	flexframegen_destroy(fg);

	return status;
}
