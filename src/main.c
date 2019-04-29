
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

    char tun_interface_name[IFNAMSIZ] = "tundevice0"; 
    params.tun_file_descriptor = tuntap_device_alloc(tun_interface_name/*argv[1]*/, 1);
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
<<<<<<< HEAD
    config_rx.frequency = 1330 * MEGA_HZ ;// (unsigned int)atoi(argv[2]);
=======
    config_rx.frequency = 1310 * MEGA_HZ ;// (unsigned int)atoi(argv[2]);
>>>>>>> c6b852fa1367a6095e870dfe007d7228dcfcd526
    config_rx.bandwidth = BLADERF_BANDWIDTH_MAX ;//(unsigned int)atoi(argv[3]);
    config_rx.samplerate = BLADERF_SAMPLERATE_REC_MAX / 10;//(unsigned int)atoi(argv[4]);
    config_rx.rx_lna = BLADERF_LNA_GAIN_MAX;//argv[5];
    config_rx.vga1 = 30;//atoi(argv[6]);
    config_rx.vga2 = 15;//atoi(argv[7]);

    config_tx.module     = BLADERF_MODULE_TX;
<<<<<<< HEAD
    config_tx.frequency = 1290 * MEGA_HZ; //(unsigned int)atoi(argv[8]);
=======
    config_tx.frequency = 1290 * MEGA_HZ;//(unsigned int)atoi(argv[8]);
>>>>>>> c6b852fa1367a6095e870dfe007d7228dcfcd526
    config_tx.bandwidth = BLADERF_BANDWIDTH_MAX;//(unsigned int)atoi(argv[9]);
    config_tx.samplerate = BLADERF_SAMPLERATE_REC_MAX / 10;//(unsigned int)atoi(argv[10]);
    config_tx.rx_lna = BLADERF_LNA_GAIN_MAX;//argv[11];
    config_tx.vga1 = -4;//atoi(argv[12]);
    config_tx.vga2 = 25;//atoi(argv[13]);
        

    total_device_number = bladerf_configs_get_device_serials(&p_dev_info);

    bladerf_configs_open_device_with_serial(&(params.p_bladerf_device), (p_dev_info)->serial);

    bladerf_configs_load_fpga(params.p_bladerf_device,  "./bladerf_fpga/hostedx115.rbf");

    status = bladerf_configs_configure_channel(params.p_bladerf_device, &config_rx);
    status = bladerf_configs_configure_channel(params.p_bladerf_device, &config_tx);

    status = bladerf_configs_config_sync_rx(params.p_bladerf_device);
    status = bladerf_configs_config_sync_tx(params.p_bladerf_device);

    status = bladerf_configs_dc_calibration(params.p_bladerf_device);
    
    ofdm_flexframe_init();

    //pthread_create(&receive_thread_id, NULL, receive_thread, (void *)(&params) );
    pthread_create(&transmit_thread_id, NULL, transmit_thread, (void *)(&params));
   
	
    while(1);

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
    unsigned char header[8] = {0};     
    unsigned char read_buffer[READ_BUFFER_SIZE]={0};//

    for(int i = 0; i < 8; i++){
    	header[i] = i;
    }


	while(0 == ret){
		int payload_size;
        payload_size = tuntap_read(p_params->tun_file_descriptor, (char*)read_buffer, READ_BUFFER_SIZE);
        if(payload_size > 0){
            ret = ofdm_flexframe_transmit(header, read_buffer, payload_size, p_params->p_bladerf_device);
        }
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

	S_Main_Params* p_params = (S_Main_Params*)_userdata;

	if ( _payload_valid  )
	{

		static int counter = 0;

		//printf("[%u]: (%s):  \tRSSI=(%5.5f)\n", counter, _payload, _stats.rssi);

		int len = tuntap_write(p_params->tun_file_descriptor, (char*)_payload, _payload_len);

		counter++;
	}
    else{
        static int invalid_header_cnt = 0;
        invalid_header_cnt++;
        printf("\ninvalid header number: %d\n", invalid_header_cnt++);
    }


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
