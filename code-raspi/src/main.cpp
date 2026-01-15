// Local dependencies
#include "error.h"
#include "hardware.h"
#include "magic.h"

// Global
#include <unistd.h>

int main(int argc, const char *argv[])
{
    try
    {
        init_hw();
        set_led(true);
        send_i2c_cmd(0xf1);
        sleep(2);
        send_i2c_cmd(0xf0);
        set_led(false);
        return 0;
    }
    catch (const st_exception &e)
    {
        fprintf(stderr, "Runtime error in file %s line %d: %s:\n%s\n", e.file(), e.line(), e.func(), e.what());
        return -1;
    }
    catch (const std::bad_alloc &e)
    {
        fprintf(stderr, "Out of memory: %s\n", e.what());
        return -1;
    }
    catch (...)
    {
        fprintf(stderr, "Unexpected error\n");
        return -1;
    }
}
