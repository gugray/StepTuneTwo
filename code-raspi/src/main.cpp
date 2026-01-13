// Local dependencies
#include "error.h"
#include "hardware_controller.h"
#include "magic.h"

// Global
#include <unistd.h>

int main(int argc, const char *argv[])
{
    try
    {
        HardwareController::set_led(true);
        sleep(5);
        HardwareController::set_led(false);
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
