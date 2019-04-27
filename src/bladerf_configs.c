#include "../includes/bladerf_configs.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "libbladeRF.h"


/*Local Function Defs*/
int process_samples_flex_frame(int16_t * samples, unsigned int sample_length, flexframesync fs);
int process_samples_ofdm_flex_frame(int16_t * samples, unsigned int sample_length, ofdmflexframesync fs);


/*Global Functions*/
int bladerf_configs_get_device_serials(struct bladerf_devinfo** _devices)
{
	int total_device_number = bladerf_get_device_list(_devices);

    return total_device_number;
}

void bladerf_configs_open_device_with_serial(struct bladerf **dev, char *serial)
{
    int status;
    struct bladerf_devinfo info;
   
    bladerf_init_devinfo(&info);

    strncpy(info.serial, serial, BLADERF_SERIAL_LENGTH - 1);
    info.serial[BLADERF_SERIAL_LENGTH - 1] = '\0';
    status = bladerf_open_with_devinfo(dev, &info);
    if (status == BLADERF_ERR_NODEV) {
        printf("No devices available with serial=%s\n", serial);
        return;   
    }

    fprintf(stderr, "Failed to open device with serial=%s (%s)\n", serial, bladerf_strerror(status)); 
    return; 

}

int bladerf_configs_load_fpga(struct bladerf *dev, const char *fpga)
{
    int status = bladerf_load_fpga(dev,  fpga);

    if (status != 0) {
        fprintf(stderr, "Unable to bladerf_load_fpga  device 1: %s\n", bladerf_strerror(status));
        return status;
    }

    fprintf(stdout, "bladerf_load_fpga device 1: %s\n", bladerf_strerror(status));
    return status;
}

int bladerf_configs_configure_channel(struct bladerf *dev, struct module_config *c)
{
    int status;
    status = bladerf_set_frequency(dev, c->module, c->frequency);
    if (status != 0) {
        fprintf(stderr, "Failed to set frequency = %u: %s\n",
                c->frequency, bladerf_strerror(status));
        return status;
    }
    status = bladerf_set_sample_rate(dev, c->module, c->samplerate, NULL);
    if (status != 0) {
        fprintf(stderr, "Failed to set samplerate = %u: %s\n",
                c->samplerate, bladerf_strerror(status));
        return status;
    }
    status = bladerf_set_bandwidth(dev, c->module, c->bandwidth, NULL);
    if (status != 0) {
        fprintf(stderr, "Failed to set bandwidth = %u: %s\n",
                c->bandwidth, bladerf_strerror(status));
        return status;
    }
    switch (c->module) {
        case BLADERF_MODULE_RX:
            status = bladerf_set_lna_gain(dev, c->rx_lna);

            if (status != 0) {
                fprintf(stderr, "Failed to set RX LNA gain: %s\n",
                        bladerf_strerror(status));
                return status;
            }
            status = bladerf_set_rxvga1(dev, c->vga1);

            if (status != 0) {
                fprintf(stderr, "Failed to set RX VGA1 gain: %s\n",
                        bladerf_strerror(status));
                return status;
            }
            status = bladerf_set_rxvga2(dev, c->vga2);

            if (status != 0) {
                fprintf(stderr, "Failed to set RX VGA2 gain: %s\n",
                        bladerf_strerror(status));
                return status;
            }
            break;
        case BLADERF_MODULE_TX:

            status = bladerf_set_txvga1(dev, c->vga1);

            if (status != 0) {
                fprintf(stderr, "Failed to set TX VGA1 gain: %s\n",
                        bladerf_strerror(status));
                return status;
            }
            status = bladerf_set_txvga2(dev, c->vga2);

            if (status != 0) {
                fprintf(stderr, "Failed to set TX VGA2 gain: %s\n",
                        bladerf_strerror(status));
                return status;
            }
            break;
        default:
            status = BLADERF_ERR_INVAL;
            fprintf(stderr, "%s: Invalid module specified (%d)\n",
                    __FUNCTION__, c->module);
    }
    return status;
}


int bladerf_configs_config_sync_rx(struct bladerf *dev)
{

    /* These items configure the underlying asynch stream used by the sync
     * interface. The "buffer" here refers to those used internally by worker
     * threads, not the user's sample buffers.
     *
     * It is important to remember that TX buffers will not be submitted to
     * the hardware until `buffer_size` samples are provided via the
     * bladerf_sync_tx call.  Similarly, samples will not be available to
     * RX via bladerf_sync_rx() until a block of `buffer_size` samples has been
     * received.
     *
     * Configure both the device's RX and TX modules for use with the synchronous
     * interface. SC16 Q11 samples *without* metadata are used. 
     */
    int status;
    
    status = bladerf_sync_config(dev,
                                 BLADERF_RX_X1,
                                 BLADERF_FORMAT_SC16_Q11,
                                 NUMBER_OF_BUFFERS,
                                 BUFFER_SIZE,
                                 NUMBER_OF_TRANSFERS,
                                 TIMEOUT_IN_MS);
    if (status != 0) {
        fprintf(stderr, "Failed to configure RX sync interface: %s\n", bladerf_strerror(status));
        return status;
    }
    status = bladerf_enable_module(dev, BLADERF_MODULE_RX, true);
    if (status != 0) {
        fprintf(stderr, "Failed to enable RX module: %s\n", bladerf_strerror(status));
        return status;
    }
    return status;
}


int bladerf_configs_config_sync_tx(struct bladerf *dev)
{
    
    /* These items configure the underlying asynch stream used by the sync
     * interface. The "buffer" here refers to those used internally by worker
     * threads, not the user's sample buffers.
     *
     * It is important to remember that TX buffers will not be submitted to
     * the hardware until `buffer_size` samples are provided via the
     * bladerf_sync_tx call.  Similarly, samples will not be available to
     * RX via bladerf_sync_rx() until a block of `buffer_size` samples has been
     * received.
     *
     * Configure both the device's RX and TX modules for use with the synchronous
     * interface. SC16 Q11 samples *without* metadata are used. 
     */
    int status;
    
    status = bladerf_sync_config(dev,
                                 BLADERF_TX_X1,
                                 BLADERF_FORMAT_SC16_Q11,
                                 NUMBER_OF_BUFFERS,
                                 BUFFER_SIZE,
                                 NUMBER_OF_TRANSFERS,
                                 TIMEOUT_IN_MS);
    if (status != 0) {
        fprintf(stderr, "Failed to configure TX sync interface: %s\n",
                bladerf_strerror(status));
    }
    status = bladerf_enable_module(dev, BLADERF_MODULE_TX, true);
    if (status != 0) {
        fprintf(stderr, "Failed to enable RX module: %s\n",
                bladerf_strerror(status));
        return status;
    }

    return status;
}

int bladerf_configs_dc_calibration(struct bladerf *dev)
{
	int status = 0 ;
	status = bladerf_calibrate_dc(dev, BLADERF_DC_CAL_LPF_TUNING);
	status = bladerf_calibrate_dc(dev, BLADERF_DC_CAL_TX_LPF);
	status = bladerf_calibrate_dc(dev, BLADERF_DC_CAL_RX_LPF);
	status = bladerf_calibrate_dc(dev, BLADERF_DC_CAL_RXVGA2);


	return status;
}

int bladerf_configs_sync_rx(struct bladerf *dev, void* fs, int is_ofdm )
{
    int status=0, ret;
    bool process_status = false;
    /* "User" samples buffers and their associated sizes, in units of samples.
     * Recall that one sample = two int16_t values. */
    int16_t *rx_samples = NULL;
    unsigned int samples_len = SAMPLE_SET_SIZE; /* May be any (reasonable) size */
    /* Allocate a buffer to store received samples in */

    rx_samples = (int16_t *)malloc(samples_len * 2 * sizeof(int16_t));
    if (rx_samples == NULL) {
        fprintf(stdout, "malloc error: %s\n", bladerf_strerror(status));
        return BLADERF_ERR_MEM;
    }

    bladerf_enable_module(dev, BLADERF_MODULE_RX, true);
    struct bladerf_metadata meta;
    memset(&meta, 0, sizeof(meta));
    /* Retrieve the current timestamp */
    if ((status=bladerf_get_timestamp(dev, BLADERF_RX, &meta.timestamp)) != 0) {
        fprintf(stderr,"Failed to get current RX timestamp: %s\n",bladerf_strerror(status));
    }
    else
    {
        printf("Current RX timestamp: 0x%016"PRIx64"\n", meta.timestamp);
    }

      meta.flags = BLADERF_META_FLAG_RX_NOW;

    while (status == 0) {
        /* Receive samples */
        status = bladerf_sync_rx(dev, rx_samples, samples_len, &meta, 5000);
        //fprintf(stdout, "Meta Flag Actual Count = %u\n", meta.actual_count );
        if (status == 0) {
            /* TODO Process these samples, and potentially produce a response to transmit */
        	if(is_ofdm){
        		process_status = process_samples_ofdm_flex_frame(rx_samples, meta.actual_count, fs);
        	} else {
        		process_status = process_samples_flex_frame(rx_samples, meta.actual_count, fs);
        	}


        	if(process_status != 0){
        		fprintf(stderr, "Failed to Process samples: %s\n", bladerf_strerror(process_status));
        	}
        	//printf("Process_samples run: %d\n", status);


        } else {
            fprintf(stderr, "Failed to RX samples: %s\n", bladerf_strerror(status));
        }
    }
    if (status == 0) {
        /* Wait a few seconds for any remaining TX samples to finish
         * reaching the RF front-end */
        usleep(2000000);
    }

	ret = status;
	/* Free up our resources */
	free(rx_samples);
	return ret;

}

int bladerf_configs_sync_tx(struct bladerf *dev,int16_t *tx_samples, unsigned int samples_len)
{
	int status = 0;

	bladerf_enable_module(dev, BLADERF_MODULE_TX, true);
    struct bladerf_metadata meta;
    memset(&meta, 0, sizeof(meta));

 //   meta.flags = BLADERF_META_FLAG_TX_NOW;
    meta.flags = BLADERF_META_FLAG_TX_BURST_START;


	status = bladerf_sync_tx(dev, tx_samples, samples_len, &meta, 5000);
	if (meta.status != 0) {
		fprintf(stderr, "Failed to TX samples: %s\n",bladerf_strerror(meta.status));
	}
	if (meta.actual_count > 0 )
		fprintf(stdout, "Meta Flag Actual Count = %u\n", meta.actual_count );


	return status;
}


/*Local Functions*/
int process_samples_flex_frame(int16_t * samples, unsigned int sample_length, flexframesync fs) {
	int status=0;


	liquid_float_complex *y = NULL;
	//convert_sc16q11_to_comlexfloat(samples, sample_length, &y);
	y = (liquid_float_complex *)malloc(sample_length * sizeof(liquid_float_complex));
    if (y != NULL) {
		for(int i = 0; i < sample_length ; i++){
			y[i]= samples[2*i]/2048.0 + samples[2*i+1]/2048.0 * I;
		}
    }

	if ( y != NULL )
	{
		for (int i=0; i<=sample_length; i=i+32)
		{
			flexframesync_execute(fs, &y[i], 32);
		}
		free(y);
	}
	else
	{
		status = BLADERF_ERR_MEM;
	}
    return status;
}


int process_samples_ofdm_flex_frame(int16_t * samples, unsigned int sample_length, ofdmflexframesync fs) {
	int status=0;

	liquid_float_complex *rec_frame = NULL;

	rec_frame = (liquid_float_complex *)malloc(sample_length * sizeof(liquid_float_complex));
    if (rec_frame != NULL) {
		for(int i = 0; i < sample_length ; i++){
			rec_frame[i]= samples[2*i]/2048.0 + samples[2*i+1]/2048.0 * I;
		}
    }

	if ( rec_frame != NULL )
	{
		for (int i=0; i<=sample_length; i++)
		{
			ofdmflexframesync_execute(fs, &rec_frame[i], 1);
		}
		free(rec_frame);
	}
	else
	{
		status = BLADERF_ERR_MEM;
	}
    return status;
}
