/*
 * ofdm_example.c
 *
 *  Created on: Apr 20, 2019
 *      Author: thedog
 */




#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <complex.h>
#include <liquid/liquid.h>

// callback function
int mycallback(unsigned char *  _header,
               int              _header_valid,
               unsigned char *  _payload,
               unsigned int     _payload_len,
               int              _payload_valid,
               framesyncstats_s _stats,
               void *           _userdata)
{
    printf("***** callback invoked!\n");
    printf("  header (%s)\n",  _header_valid  ? "valid" : "INVALID");
    printf("  payload (%s)\n", _payload_valid ? "valid" : "INVALID");
    printf("  payload (%s)\n", _payload );
    printf("  payload len (%d)\n", _payload_len);
    return 0;
}

int main() {
    // options
    unsigned int M           = 64;              // number of subcarriers
    unsigned int cp_len      = 16;              // cyclic prefix length
    unsigned int taper_len   = 4;               // taper length
    unsigned int payload_len = 120;             // length of payload (bytes)
    modulation_scheme ms = LIQUID_MODEM_PSK8;   // payload modulation scheme
    fec_scheme fec0  = LIQUID_FEC_NONE;         // inner FEC scheme
    fec_scheme fec1  = LIQUID_FEC_HAMMING128;   // outer FEC scheme
    crc_scheme check = LIQUID_CRC_32;           // data validity check
    float dphi  = 0.001f;                       // carrier frequency offset
    float SNRdB = 20.0f;                        // signal-to-noise ratio [dB]

    // allocate memory for header, payload, sample buffer
    unsigned int symbol_len = M + cp_len;       // samples per OFDM symbol
    float complex buffer[symbol_len];           // time-domain buffer
    unsigned char header[8];                    // header
    unsigned char payload[payload_len];         // payload

    // create frame generator with default properties
    ofdmflexframegen fg =
        ofdmflexframegen_create(M, cp_len, taper_len, NULL, NULL);

    // create frame synchronizer
    ofdmflexframesync fs =
        ofdmflexframesync_create(M, cp_len, taper_len, NULL, mycallback, NULL);

    unsigned int i;

    // re-configure frame generator with different properties
    ofdmflexframegenprops_s fgprops;
    ofdmflexframegen_getprops(fg,&fgprops); // query the current properties
    fgprops.check           = check;        // set the error-detection scheme
    fgprops.fec0            = fec0;         // set the inner FEC scheme
    fgprops.fec1            = fec1;         // set the outer FEC scheme
    fgprops.mod_scheme      = ms;           // set the modulation scheme
    ofdmflexframegen_setprops(fg,&fgprops); // reconfigure the frame generator

    // initialize header/payload and assemble frame
    for (i=0; i<8; i++)           header[i]  = i      & 0xff;
    for (i=0; i<payload_len; i++) payload[i] = i;//rand() & 0xff;
    ofdmflexframegen_assemble(fg, header, payload, payload_len);
    ofdmflexframegen_print(fg);

    // channel parameters
    float nstd = powf(10.0f, -SNRdB/20.0f); // noise standard deviation
    float phi = 0.0f;                       // channel phase

    // generate frame and synchronize
    int last_symbol=0;

#if 0
    while (!last_symbol) {
        // generate symbol (write samples to buffer)
        last_symbol = ofdmflexframegen_write(fg, buffer, symbol_len);


        // channel impairments
        for (i=0; i<symbol_len; i++) {
            buffer[i] *= cexpf(_Complex_I*phi); // apply carrier offset
            phi += dphi;                        // update carrier phase
            cawgn(&buffer[i], nstd);            // add noise
        }

        // receive symbol (read samples from buffer)
        ofdmflexframesync_execute(fs, buffer, symbol_len);
    }
#endif

#if 1
    int frame_complete = 0;
    int lastpos = 0;
	int16_t *tx_samples = NULL;
	float complex frame_buffer[8192*2];

    int num_of_symbol_in_frame = ofdmflexframegen_getframelen(fg);

	while (!frame_complete) {

		frame_complete = ofdmflexframegen_write(fg, buffer, symbol_len);
		memcpy((void*)(&(frame_buffer[lastpos])), buffer, symbol_len*sizeof(float complex));
		lastpos = lastpos + symbol_len;
	}

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

	liquid_float_complex *rx_frame = NULL;

	rx_frame = (liquid_float_complex *)malloc(num_of_symbol_in_frame *symbol_len * sizeof(liquid_float_complex));
	if (rx_frame != NULL) {
		for(int i = 0; i < num_of_symbol_in_frame *symbol_len ; i++){
			rx_frame[i]= tx_samples[2*i]/2048.0 + tx_samples[2*i+1]/2048.0 * I;
			//rx_frame[i] = (rand() & 0xff) + (rand() & 0xff) * I;
		}
	}

	for (int i=0; i<=num_of_symbol_in_frame *symbol_len; i=i+32)
	{
		ofdmflexframesync_execute(fs, &rx_frame[i], 32);
	}
	free(rx_frame);
#endif


    // destroy objects and return
    ofdmflexframegen_destroy(fg);
    ofdmflexframesync_destroy(fs);
    printf("done.\n");
    return 0;
}
