/*
 * utils.h
 *
 *  Created on: Apr 18, 2019
 *      Author: thedog
 */

#ifndef INCLUDES_UTILS_H_
#define INCLUDES_UTILS_H_

#include <complex.h>
#include <liquid/liquid.h>

typedef struct{
    float real;
    float imag;
}Complex_Float;

void convert_sc16q11_to_comlexfloat 	( int16_t * in, int16_t inlen, liquid_float_complex ** p_out );

int16_t* convert_comlexfloat_to_sc16q11(float complex *in, unsigned int  inlen);


#endif /* INCLUDES_UTILS_H_ */
