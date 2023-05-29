
#include <common.h>

uint64_t nanoseconds_to_mach_time(uint64_t ns)
{
    static struct mach_timebase_info timebase = { .numer = 0, .denom = 0 };
    if(timebase.denom == 0)
    {
        mach_timebase_info(&timebase);
    }
    return ns * timebase.denom / timebase.numer;
}
