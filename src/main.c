
#include "../includes/tuntap.h"
#include "../includes/bladerf_configs.h"
#include "../includes/ofdm_flexframe.h"

#include <math.h>
#include <complex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/if.h>
#include <linux/if_tun.h>
#include <fcntl.h>
#include <arpa/inet.h> 
#include <errno.h>
#include <stdarg.h>
#include <pthread.h> 
#include <sys/types.h>
#include <sys/wait.h>


typedef struct{
	struct bladerf *p_bladerf_device;
	int tun_file_descriptor;
}S_Main_Params;


void* receive_thread(void* vargp);
void* transmit_thread(void* vargp);
int fork_child_process(char* path_to_exe);

static int mycallback(unsigned char *  _header,
                      int              _header_valid,
                      unsigned char *  _payload,
                      unsigned int     _payload_len,
                      int              _payload_valid,
                      framesyncstats_s _stats,
                      void *           _userdata);




int main(int argc, char *argv[])
{
    S_Main_Params params;
    memset(&params, 0, sizeof(S_Main_Params));


    params.tun_file_descriptor = tuntap_device_alloc(argv[1], 1);
	if (params.tun_file_descriptor < 0  ) {
		printf("Error connecting to tun/tap interface %s!\n", argv[1]);
	}


    int status = 0;
    int total_device_number = 0;
    struct module_config config_rx;
    struct module_config config_tx;
    struct bladerf_devinfo* p_dev_info;

    pthread_t receive_thread_id;
    pthread_t transmit_thread_id;


    config_rx.module     = BLADERF_MODULE_RX;
    config_rx.frequency = (unsigned int)atoi(argv[2]);
    config_rx.bandwidth = (unsigned int)atoi(argv[3]);
    config_rx.samplerate = (unsigned int)atoi(argv[4]);
    config_rx.rx_lna = BLADERF_LNA_GAIN_MAX;//argv[5];
    config_rx.vga1 = atoi(argv[6]);
    config_rx.vga2 = atoi(argv[7]);

    config_tx.module     = BLADERF_MODULE_TX;
    config_tx.frequency = (unsigned int)atoi(argv[8]);
    config_tx.bandwidth = (unsigned int)atoi(argv[9]);
    config_tx.samplerate = (unsigned int)atoi(argv[10]);
    config_tx.rx_lna = BLADERF_LNA_GAIN_MAX;//argv[11];
    config_tx.vga1 = atoi(argv[12]);
    config_tx.vga2 = atoi(argv[13]);
        

    total_device_number = bladerf_configs_get_device_serials(&p_dev_info);

    bladerf_configs_open_device_with_serial(&(params.p_bladerf_device), (p_dev_info)->serial);

    bladerf_configs_load_fpga(params.p_bladerf_device,  "../../bladerf_fpga/hostedx115.rbf");

    status = bladerf_configs_configure_channel(params.p_bladerf_device, &config_rx);
    status = bladerf_configs_configure_channel(params.p_bladerf_device, &config_tx);

    status = bladerf_configs_config_sync_rx(params.p_bladerf_device);
    status = bladerf_configs_config_sync_tx(params.p_bladerf_device);

    status = bladerf_configs_dc_calibration(params.p_bladerf_device);


    ofdm_flexframe_init();

    pthread_create(&receive_thread_id, NULL, receive_thread, (void *)(&params) );
    pthread_create(&transmit_thread_id, NULL, transmit_thread, (void *)(&params));
   
	
    return 0;
}


void* receive_thread(void* vargp)
{
	S_Main_Params* p_params = (S_Main_Params*)vargp;

	ofdm_flexframe_receive(mycallback, p_params->p_bladerf_device, vargp);

}


void* transmit_thread(void* vargp)
{

	S_Main_Params* p_params = (S_Main_Params*)vargp;
	int ret = 0;
	int cnt = 0;
    unsigned char header[8] = {0};        // data header
    unsigned char payload[PAYLOAD_LENGTH]={0};//

    for(int i = 0; i < 8; i++){
    	header[i] = i;
    }


	while(0 == ret){
		//TODO fill payload
		cnt++;

		memset(payload, 0x00, PAYLOAD_LENGTH); //
		sprintf((char*)payload,"Packet (%d)",cnt);
		memset(&payload[13], 0x00, PAYLOAD_LENGTH-13);

		ret = ofdm_flexframe_transmit(header, payload, PAYLOAD_LENGTH, p_params->p_bladerf_device);

	}
}

static int mycallback(unsigned char *  _header,
                      int              _header_valid,
                      unsigned char *  _payload,
                      unsigned int     _payload_len,
                      int              _payload_valid,
                      framesyncstats_s _stats,
                      void *           _userdata)
{

	static int counter = 0;



	if ( _header_valid  )
	{
		//printf("Packet %u contains (%s) with RSSI %5.5f\n", *counter, _payload, _stats.rssi);

        unsigned int n = 6;
        unsigned char payload[PAYLOAD_LENGTH]={0};
        snprintf((char * )payload, 8, "Packet ");
        unsigned int num_bit_errors = count_bit_errors_array(payload, _payload, n);
        printf("[%u]: (%s):  %3u / %3u\tRSSI=(%5.5f)\n", counter, _payload, num_bit_errors, n*8, _stats.rssi);
        //TODO: _payload => tuntap_write
	}

	counter++;

    return 0;
}

int fork_child_process(char* path_to_exe)
{
	char *arg[] = {NULL};
	int status;
    pid_t child_pid;
    pid_t tpid;
    int child_status;
    child_pid = fork();

       if(child_pid == 0){

           status = execv(path_to_exe, arg);
           printf("Error execv");
           exit(0);
       }
       else{
           do{int
               tpid = wait(&child_status);
               if(tpid != child_pid){
                   //process_terminated(tpid);
               }

           }while (tpid != child_pid);

           return child_status;

       }
}
