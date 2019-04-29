#ifndef INCLUDE_BLADERF_CONFIGS_H_
#define INCLUDE_BLADERF_CONFIGS_H_

#include "libbladeRF.h"
#include <liquid/liquid.h>

#define  NUMBER_OF_BUFFERS 16
#define  SAMPLE_SET_SIZE 65536 //must be multiple of 2
#define  BUFFER_SIZE    SAMPLE_SET_SIZE*sizeof(int32_t)
#define  NUMBER_OF_TRANSFERS 8
#define  TIMEOUT_IN_MS 1000

#define  MEGA_HZ 1000000
#define  TX_FREQUENCY_USED (300 * MEGA_HZ)
#define  RX_FREQUENCY_USED (350 * MEGA_HZ)
#define  BANDWIDTH_USED (10 * MEGA_HZ)
#define  SAMPLING_RATE_USED 600000
#define  PAYLOAD_LENGTH 1500 // MTU

struct channel_config {
    bladerf_channel channel;
    unsigned long long frequency;
    unsigned long long bandwidth;
    unsigned long long samplerate;
    int gain;
};

struct module_config {
    bladerf_module module;
    unsigned long long frequency;
    unsigned long long bandwidth;
    unsigned long long samplerate;
    /* Gains */
    bladerf_lna_gain rx_lna;
    int vga1;
    int vga2;
};



int bladerf_configs_get_device_serials(struct bladerf_devinfo** _devices);
void bladerf_configs_open_device_with_serial(struct bladerf** dev, char *serial);
int bladerf_configs_load_fpga(struct bladerf *dev, const char *fpga);
int bladerf_configs_configure_channel(struct bladerf *dev, struct module_config *c);
int bladerf_configs_config_sync_rx(struct bladerf *dev);
int bladerf_configs_config_sync_tx(struct bladerf *dev);
int bladerf_configs_dc_calibration(struct bladerf *dev);
int bladerf_configs_sync_rx(struct bladerf *dev, void* fs);
int bladerf_configs_sync_tx(struct bladerf *dev,int16_t *tx_samples, unsigned int samples_len);

#endif  /* INCLUDE_BLADERF_CONFIGS_H_ */
