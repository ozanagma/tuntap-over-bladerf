/*
 * utils.c
 *
 *  Created on: Apr 18, 2019
 *      Author: thedog
 */

#include "../includes/utils.h"
#include <unistd.h>



void convert_sc16q11_to_comlexfloat 	( int16_t * in, int16_t inlen, liquid_float_complex ** out_pointer )
{

	int i=0;
	//liquid_float_complex * out = NULL;
	*out_pointer = NULL;
	*out_pointer = (liquid_float_complex *)malloc(inlen * sizeof(liquid_float_complex));
    if (*out_pointer != NULL) {
		for(i = 0; i < inlen ; i++){
			*out_pointer[i]= in[2*i]/2048.0 + in[2*i+1]/2048.0 * I;
                //out[i].real = in[2*i]/2048.0;
                //out[i].imag = in[2*i+1]/2048.0;
		}
    }
	//return out;
}

int16_t* convert_comlexfloat_to_sc16q11(float complex *in, unsigned int  inlen)
{
	int i=0;
	int16_t * out = NULL;
    out = (int16_t *)malloc(inlen * 2 * sizeof(int16_t));
    if (out != NULL) {
		for(i = 0; i < inlen ; i++){
				out[2*i]= round( crealf(in[i]) * 2048); // Since bladeRF uses Q4.11 complex 2048=2^11
				out[2*i+1]= round( cimagf(in[i]) * 2048);
				if ( out[2*i] > 2047  ) out[2*i]=2047;
				if ( out[2*i] < -2048  ) out[2*i]=-2048;
				if ( out[2*i+1] > 2047  ) out[2*i+1]=2047;
				if ( out[2*i+1] < -2048  ) out[2*i+1]=-2048;
		}
    }
	return (int16_t *)out;
}
