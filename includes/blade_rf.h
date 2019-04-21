#ifndef INCLUDE_BLADE_RF_H_
#define INCLUDE_BLADE_RF_H_

#include "libbladeRF.h"
#include <liquid/liquid.h>

#define  NUMBER_OF_BUFFERS 16
#define  SAMPLE_SET_SIZE 8192 //must be multiple of 2
#define  BUFFER_SIZE    SAMPLE_SET_SIZE*sizeof(int32_t)
#define  NUMBER_OF_TRANSFERS 8
#define  TIMEOUT_IN_MS 1000

#define  MEGA_HZ 1000000
#define  FREQUENCY_USED (400 * MEGA_HZ)//713000000
#define  BANDWIDTH_USED (10 * MEGA_HZ)
#define  SAMPLING_RATE_USED 600000
#define  PAYLOAD_LENGTH 120

struct channel_config {
    bladerf_channel channel;
    unsigned int frequency;
    unsigned int bandwidth;
    unsigned int samplerate;
    int gain;
};

struct module_config {
    bladerf_module module;
    unsigned int frequency;
    unsigned int bandwidth;
    unsigned int samplerate;
    /* Gains */
    bladerf_lna_gain rx_lna;
    int vga1;
    int vga2;
};



int BladeRFGetDeviceSerials(struct bladerf_devinfo** _devices);
void BladeRFOpenWithSerial(struct bladerf** dev, char *serial);
int BladeRFLoadFPGA(struct bladerf *dev, const char *fpga);
int BladeRFConfigureChannel(struct bladerf *dev, struct module_config *c);
int BladeRFInitializeSyncRx(struct bladerf *dev);
int BladeRFInitializeSyncTx(struct bladerf *dev);
int BladeRFDCCalibration(struct bladerf *dev);
int BladeRFSyncRx(struct bladerf *dev, void* fs, int is_ofdm );
int BladeRFSyncTx(struct bladerf *dev,int16_t *tx_samples, unsigned int samples_len);

#endif  /* INCLUDE_BLADE_RF_H_ */
