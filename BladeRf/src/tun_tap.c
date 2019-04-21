#include "../includes/tun_tap.h"

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



int tun_tap_device_alloc(char *device_name, int is_tun)
{
  struct ifreq s_interface_request;
  int fd, err;

  if( (fd = open("/dev/net/tun", O_RDWR)) < 0 )
    // return tun_alloc_old(dev);

  memset(&s_interface_request, 0, sizeof(s_interface_request));

  /* Flags: IFF_TUN   - TUN device (no Ethernet headers) 
    *        IFF_TAP   - TAP device 
    *        IFF_NO_PI - Do not provide packet information  
    */ 
  if(is_tun){
	  s_interface_request.ifr_flags = IFF_TUN;

  }else{
	  s_interface_request.ifr_flags = IFF_TAP;
  }

  if( *device_name)
      strncpy(s_interface_request.ifr_name, device_name, IFNAMSIZ);

  if( (err = ioctl(fd, TUNSETIFF, (void *) &s_interface_request)) < 0 ){
    if(errno == 16){

    }
    else{
      close(fd);
      printf("ioctl failed and returned errno %s \n",strerror(errno));
      return err;
    }
  }
  strcpy(device_name, s_interface_request.ifr_name);
  return fd;
}


int read_tun_tap_device(int tun_tap_fd, char* buffer, int size){

	return read(tun_tap_fd, buffer, size);

}

int write_tun_tap_device(int tun_tap_fd, char* buffer, int size){
	return write(tun_tap_fd, buffer, size);
}


