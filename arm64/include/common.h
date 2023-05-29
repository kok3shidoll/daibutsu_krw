#ifndef COMMON_H
#define COMMON_H

#include <mach/mach_time.h>     // mach_absolute_time
#include <mach-o/loader.h>

uint64_t nanoseconds_to_mach_time(uint64_t ns);

#define TIMER_START(timer) \
uint64_t timer = mach_absolute_time();

#define TIMER_SLEEP_UNTIL(timer, ns) \
do \
{ \
mach_wait_until(timer + nanoseconds_to_mach_time(ns)); \
} while(0)

#define ADDR "0x%016llx"
#define KERNEL_BASE_ADDRESS 0xffffff8002002000
typedef uint64_t addr_t;
#define MACH_MAGIC MH_MAGIC_64
typedef struct mach_header_64 mach_hdr_t;
typedef struct segment_command_64 mach_seg_t;
typedef struct section_64 mach_sec_t;

#define SIZE "0x%016lx"
typedef struct load_command mach_cmd_t;

#ifndef MIN
#   define MIN(x, y) ((x) > (y) ? (y) : (x))
#endif

#endif
