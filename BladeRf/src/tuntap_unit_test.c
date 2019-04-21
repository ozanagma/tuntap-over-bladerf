/*
 * tuntap_test_driver.c
 *
 *  Created on: Apr 16, 2019
 *      Author: thedog
 */


#include "../includes/tun_tap.h"
#include <linux/if_tun.h>
#include <linux/if.h>
#include <pthread.h>
#include "err.h"
#include "errno.h"

typedef struct{
	int tun_client_fd;
	int tun_server_fd;
}S_TUN_FDS;

#define BUFFER_SIZE 1500


void tun_client(void* arg);
void tun_server(void* arg);


int main (int argc, char *argv[]){

	S_TUN_FDS tun_fds;
	char tun_if_client_name[IFNAMSIZ] = "tun_client";
	char tun_if_server_name[IFNAMSIZ] = "tun_server";


	tun_fds.tun_client_fd = tun_tap_device_alloc(tun_if_client_name, 1);
	if(tun_fds.tun_client_fd < 0){
		perror("tun client device not alloced");
	}

	tun_fds.tun_server_fd = tun_tap_device_alloc(tun_if_server_name, 1);
	if(tun_fds.tun_server_fd < 0){
		perror("tun server device not alloced");
	}

	// Create threads
	pthread_t tun_client_t, tun_server_t;
	int ret1, ret2;

	printf("Starting threads\n");
	ret1 = pthread_create(&tun_client_t, NULL, &tun_client, (void *) &tun_fds);
	ret2 = pthread_create(&tun_server_t, NULL, &tun_server, (void *) &tun_fds);

	while(1){
		sleep(10);
	}

	return 0;
}

void tun_client(void* arg){

	S_TUN_FDS* p_tun_fds = arg;
	int ret, r_size;
	char buffer[BUFFER_SIZE] = {0};

	while(1){

		r_size = read_tun_tap_device(p_tun_fds->tun_server_fd, buffer, BUFFER_SIZE);

		if(r_size < 0){
			perror(errno);
			continue;
		}

		ret = write_tun_tap_device(p_tun_fds->tun_client_fd, buffer, r_size);
		if(ret < 0){
			perror(errno);
			continue;
		}
	}


}
void tun_server(void* arg){

	S_TUN_FDS* p_tun_fds = arg;
	int ret, r_size;
	char buffer[BUFFER_SIZE] = {0};

	while(1){

		r_size = read_tun_tap_device(p_tun_fds->tun_client_fd, buffer, BUFFER_SIZE);

		if(r_size < 0){
			perror(errno);
			continue;
		}

		ret = write_tun_tap_device(p_tun_fds->tun_server_fd, buffer, r_size);
		if(ret < 0){
			perror(errno);
			continue;
		}
	}
}
