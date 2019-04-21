
#ifndef INCLUDE_TUN_TAP_H_
#define INCLUDE_TUN_TAP_H_


int tun_tap_device_alloc(char *device_name, int is_tun);

int read_tun_tap_device(int tun_tap_fd, char* buffer, int size);

int write_tun_tap_device(int tun_tap_fd, char* buffer, int size);


#endif /* INCLUDE_TUN_TAP_H_ */
