
#ifndef INCLUDE_TUNTAP_H_
#define INCLUDE_TUNTAP_H_


int tuntap_device_alloc(char *device_name, int is_tun);

int tuntap_read(int tun_tap_fd, char* buffer, int size);

int tuntap_write(int tun_tap_fd, char* buffer, int size);


#endif /* INCLUDE_TUNTAP_H_ */
