// Local dependencies
#include "error.h"
#include "magic.h"

// Global

int main(int argc, const char *argv[])
{
    try
    {
        printf("\nGoodbye!\n");
        return 0;
    }
    catch (const st_exception &e)
    {
        fprintf(stderr, "Runtime error in file %s line %d: %s:\n%s\n", e.file(), e.line(), e.func(), e.what());
        cleanup_horrors();
        return -1;
    }
    catch (const std::bad_alloc &e)
    {
        fprintf(stderr, "Out of memory: %s\n", e.what());
        cleanup_horrors();
        return -1;
    }
    catch (...)
    {
        fprintf(stderr, "Unexpected error\n");
        cleanup_horrors();
        return -1;
    }
}
