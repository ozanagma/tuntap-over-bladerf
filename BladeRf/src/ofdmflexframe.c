/*
 * ofdmflexframe.c
 *
 *  Created on: Apr 20, 2019
 *      Author: thedog
 */

#include "../includes/ofdmflexframe.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <complex.h>
#include <liquid/liquid.h>


typedef struct{
	unsigned int num_of_sc ;              // number of subcarriers
	unsigned int cp_len;              // cyclic prefix length
	unsigned int taper_len;               // taper length
	unsigned int payload_len;

	modulation_scheme mod_sch;   // payload modulation scheme
	fec_scheme fec0;         // inner FEC scheme
	fec_scheme fec1;   // outer FEC scheme
	crc_scheme check;
}S_OFDM_PARAMS;


static S_OFDM_PARAMS ofdm_params;

void OfdmFlexFrameInit(void){

	memset(&ofdm_params, 0, sizeof(S_OFDM_PARAMS));

	ofdm_params.num_of_sc = 64;
	ofdm_params.cp_len = 16;
	ofdm_params.taper_len = 4;
	ofdm_params.payload_len = 120;
	ofdm_params.mod_sch = LIQUID_MODEM_PSK2;
	ofdm_params.fec0 = LIQUID_FEC_NONE;
	ofdm_params.fec1 = LIQUID_FEC_HAMMING128;
	ofdm_params.check = LIQUID_CRC_32;


}

void OfdmFlexFrameReceive(framesync_callback _callback, struct bladerf *p_bladerf_device, void* user_data){

    int status;
    unsigned int frame_counter = 0;
    unsigned char subcarrier_allocation[ofdm_params.num_of_sc];

    ofdmframe_init_sctype_range(ofdm_params.num_of_sc, -0.25, 0.25, subcarrier_allocation);

    //ofdmframe_init_default_sctype(ofdm_params.num_of_sc, subcarrier_allocation);

    ofdmframe_print_sctype(subcarrier_allocation, ofdm_params.num_of_sc);

    ofdmflexframesync fs = ofdmflexframesync_create(ofdm_params.num_of_sc, ofdm_params.cp_len, ofdm_params.taper_len, subcarrier_allocation, _callback, user_data);
    ofdmflexframesync_debug_enable(fs);

    status =  BladeRFSyncRx(p_bladerf_device, fs, 1);


    ofdmflexframesync_destroy(fs);
}

int OfdmFlexFrameTransmit(unsigned char tx_header[8],  unsigned char* tx_payload, int tx_payload_size, struct bladerf *p_bladerf_device){


	int status = 0;
	int16_t *tx_samples = NULL;

	// allocate memory for header, payload, sample buffer
	unsigned int symbol_len = ofdm_params.num_of_sc + ofdm_params.cp_len;       // samples per OFDM symbol
	unsigned int num_of_symbol_in_frame = 0;
	float complex buffer[symbol_len];           // time-domain buffer
	float complex frame_buffer[BUFFER_SIZE];           // frame buffer
	unsigned char subcarrier_allocation[ofdm_params.num_of_sc];




    // re-configure frame generator with different properties
    ofdmflexframegenprops_s fgprops;

    ofdmflexframegenprops_init_default(&fgprops);

    fgprops.check           = ofdm_params.check;        // set the error-detection scheme
    fgprops.fec0            = ofdm_params.fec0;         // set the inner FEC scheme
    fgprops.fec1            = ofdm_params.fec1;         // set the outer FEC scheme
    fgprops.mod_scheme      = ofdm_params.mod_sch;           // set the modulation scheme

	ofdmframe_init_sctype_range(ofdm_params.num_of_sc, -0.25, 0.25, subcarrier_allocation);

    // create frame generator with default properties
    ofdmflexframegen fg = ofdmflexframegen_create(ofdm_params.num_of_sc, ofdm_params.cp_len, ofdm_params.taper_len, subcarrier_allocation, &fgprops);


    ofdmflexframegen_assemble(fg, tx_header, tx_payload, tx_payload_size);
    //ofdmflexframegen_print(fg);


    num_of_symbol_in_frame = ofdmflexframegen_getframelen(fg);

	int lastpos = 0;
	int frame_complete = 0;

	while (!frame_complete) {

		frame_complete = ofdmflexframegen_write(fg, buffer, symbol_len);
		memcpy((void*)(&(frame_buffer[lastpos])), buffer, symbol_len*sizeof(float complex));
		lastpos = lastpos + symbol_len;
	}

	printf("number of samples %u %u\n", symbol_len, lastpos);

	tx_samples = (int16_t *)malloc(num_of_symbol_in_frame * 2 * sizeof(int16_t)*symbol_len);
    if (tx_samples != NULL) {
		for(int i = 0; i < num_of_symbol_in_frame * symbol_len ; i++){
			tx_samples[2*i]= round( crealf(frame_buffer[i]) * 2048); // Since bladeRF uses Q4.11 complex 2048=2^11
			tx_samples[2*i+1]= round( cimagf(frame_buffer[i]) * 2048);
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

	status =  BladeRFSyncTx(p_bladerf_device, tx_samples, (num_of_symbol_in_frame * symbol_len ));
	if (status != 0) {
		fprintf(stderr, "Failed to sync_tx(). Exiting. %s\n", bladerf_strerror(status));
	}
	fprintf(stdout, "TxPacket: %s\n", tx_payload);
	usleep(10000);

    ofdmflexframegen_destroy(fg);

	return status;

}


