#ifndef HARDWARE_H
#define HARDWARE_H

#include <stdint.h>

void init_hw();
void cleanup_hw();
void set_led(bool on);
void send_i2c_cmd(uint8_t cmd);
void send_i2c_cmd1(uint8_t cmd, uint32_t arg);

#endif
