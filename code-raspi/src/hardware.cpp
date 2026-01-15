#include "hardware.h"

// Local dependencies
#include "error.h"
#include "lock.h"
#include "magic.h"

// Global
#include <errno.h>
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <vector>

#define BLOCK_SIZE 4096
#define HWCTRL_CYCLE_MSEC 20

static int file_i2c = -1;
static pthread_t thread;
static pthread_mutex_t mut;
static std::vector<uint8_t> commands;
static bool quitting;

static void *loop(void *);

void set_led(bool on)
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

void init_hw()
{
    // Open I2C bus
    char *filename = (char *)I2C_NODE;
    if ((file_i2c = open(filename, O_RDWR)) < 0)
    {
        cleanup_hw();
        THROWF_ERRNO("Failed to open the I2C bus '%s'", filename);
    }

    // Access I2C
    int addr = I2C_TARGET_ADDRESS;
    if (ioctl(file_i2c, I2C_SLAVE, addr) < 0)
    {
        cleanup_hw();
        THROWF_ERRNO("Failed to acquire I2C bus access");
    }

    // Create mutex
    int r = pthread_mutex_init(&mut, NULL);
    if (r != 0)
    {
        cleanup_hw();
        THROWF("Failed to initialize mutex: %d: %s", r, strerror(r));
    }

    // Start worker thread
    r = pthread_create(&thread, NULL, loop, NULL);
    if (r != 0)
    {
        cleanup_hw();
        THROWF("Failed to create thread: %d: %s", r, strerror(r));
    }
    pthread_detach(thread);
}

void cleanup_hw()
{
    quitting = true;
}

void *loop(void *)
{
    while (!quitting)
    {
        usleep(HWCTRL_CYCLE_MSEC * 1000);

        // Send commands from queue
        while (true)
        {
            Lock lock(&mut);
            size_t len = commands.size();
            if (len == 0) continue;
            if (write(file_i2c, &commands[0], len) != (ssize_t)len)
            {
                printf("Failed to write command to the I2C bus; it's lost now: %d: %s\n", errno, strerror(errno));
            }
            commands.clear();
        }
    }
    if (file_i2c != -1) close(file_i2c);
    file_i2c = -1;
    return nullptr;
}

void send_i2c_cmd(uint8_t cmd)
{
    Lock lock(&mut);
    commands.push_back(cmd);
}
