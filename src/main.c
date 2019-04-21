
#include "../includes/tun_tap.h"
#include "../includes/blade_rf.h"
#include "../includes/ofdmflexframe.h"

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

//struct bladerf *p_bladerf_device = NULL;

void receive_thread(void* vargp);
void transmit_thread(void* vargp);
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

    char* create_tun_interface_prog_path = "../../bash_scripts/create_tun_interface";
    char* remove_tun_interface_prog_path = "../../bash_scripts/remove_tun_interface";
    char tun_interface_name[IFNAMSIZ] = "tun_client";

    S_Main_Params params;
    memset(&params, 0, sizeof(S_Main_Params));

/*
    params.tun_file_descriptor = tun_tap_device_alloc(tun_interface_name, 1);
	if (params.tun_file_descriptor < 0  ) {
		printf("Error connecting to tun/tap interface %s!\n", tun_interface_name);
	}
*/

    int status;
    int total_device_number;
    struct module_config config_rx;
    struct module_config config_tx;
    struct bladerf_devinfo* p_dev_info;

    pthread_t receive_thread_id;
    pthread_t transmit_thread_id;
    
    //fork_child_process(remove_tun_interface_prog_path);

    //fork_child_process(create_tun_interface_prog_path);

    total_device_number = BladeRFGetDeviceSerials(&p_dev_info);

  
    BladeRFOpenWithSerial(&(params.p_bladerf_device), (p_dev_info)->serial);
    BladeRFLoadFPGA(params.p_bladerf_device,  "../../bladerf_fpga/hostedx115.rbf");


   
    config_rx.module     = BLADERF_MODULE_RX;
    config_tx.module     = BLADERF_MODULE_TX;
    config_tx.frequency  = config_rx.frequency  = FREQUENCY_USED;
    config_tx.bandwidth  = config_rx.bandwidth  = BANDWIDTH_USED;
    config_tx.samplerate = config_rx.samplerate = SAMPLING_RATE_USED;
    config_tx.rx_lna     = config_rx.rx_lna     = BLADERF_LNA_GAIN_MAX;

    config_rx.vga1 = 30;
    config_rx.vga2 = 15;

    config_tx.vga1 = -4;
    config_tx.vga2 = 25;


    status = BladeRFConfigureChannel(params.p_bladerf_device, &config_rx);
    status = BladeRFConfigureChannel(params.p_bladerf_device, &config_tx);

    status = BladeRFInitializeSyncRx(params.p_bladerf_device);
    status = BladeRFInitializeSyncTx(params.p_bladerf_device);

    status = BladeRFDCCalibration(params.p_bladerf_device);


    OfdmFlexFrameInit();
    //pthread_create(&receive_thread_id, NULL, (void *)&receive_thread, (void *)(&params) );
    pthread_create(&transmit_thread_id, NULL, (void *)&transmit_thread, (void *)(&params));
   


//TODO: sleep yerine main'in return etmeyeceği bir kod koyulmalı. while(1) olabilir
    while(1){
    	static int wc = 0;
    	sleep(60);
    	wc++;
    	printf("waiting count : %d minutes", &wc);

    }

   // execv(remove_tun_interface_prog_path, arg);
	
    return 0;
}


void receive_thread(void* vargp)
{
	S_Main_Params* p_params = (S_Main_Params*)vargp;

	OfdmFlexFrameReceive(mycallback, p_params->p_bladerf_device, vargp);

}


void transmit_thread(void* vargp)
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

		ret = OfdmFlexFrameTransmit(header, payload, PAYLOAD_LENGTH, p_params->p_bladerf_device);

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
 /*  printf("***** callback invoked!\n");
    printf("  header (%s)\n",  _header_valid  ? "valid" : "INVALID");
    printf("  payload (%s)\n", _payload_valid ? "valid" : "INVALID");
    printf("  payload length (%d)\n", _payload_len);*/
//
//    // type-cast, de-reference, and increment frame counter
  // unsigned int * counter = (unsigned int *) _userdata;
   // (*counter)++;
	static int counter = 0;

  // framesyncstats_print(&_stats);


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
