#include "hardware_controller.h"

// Local dependencies
#include "error.h"
#include "magic.h"

// Global
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

#define BLOCK_SIZE 4096

void HardwareController::set_led(bool on)
{
    int mem_fd = open("/dev/gpiomem", O_RDWR | O_SYNC);
    if (mem_fd < 0) THROWF_ERRNO("Failed to open /dev/gpiomem");

    volatile uint32_t *gpio = (uint32_t *)mmap(
        NULL,
        BLOCK_SIZE,
        PROT_READ | PROT_WRITE,
        MAP_SHARED,
        mem_fd,
        0);
    if (gpio == MAP_FAILED) THROWF_ERRNO("Failed to mmap /dev/gpiomem");

    // Set GPIO4 as output
    int reg = LED_GPIO_PIN / 10; // Each GPFSEL controls 10 pins
    int shift = (LED_GPIO_PIN % 10) * 3;
    gpio[reg] &= ~(0b111 << shift); // Clear function bits
    gpio[reg] |= (0b001 << shift);  // Set as output

    // Set GPIO4 high or low
    if (on) gpio[7] = (1 << LED_GPIO_PIN); // GPSET0 is register 7
    else gpio[10] = (1 << LED_GPIO_PIN);   // GPCLR0 is register 10

    if (munmap((void *)gpio, BLOCK_SIZE) < 0) THROWF_ERRNO("Failed to munmap GPIO memory");
    if (close(mem_fd) < 0) THROWF_ERRNO("Failed to close /dev/gpiomem");
}
