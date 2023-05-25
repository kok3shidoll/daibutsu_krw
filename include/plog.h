#ifndef PLOG_H
#define PLOG_H

#include <stdio.h>

#define LOG(x, ...) \
do { \
printf("[daibutsu:log] "x"\n", ##__VA_ARGS__); \
} while(0)

#define ERR(x, ...) \
do { \
printf("[daibutsu:error] "x"\n", ##__VA_ARGS__); \
} while(0)

#define FATAL(x, ...) \
do { \
printf("[daibutsu:error] FATAL "x"\n", ##__VA_ARGS__); \
} while(0)


#ifdef DEVBUILD
#define DEVLOG(x, ...) \
do { \
printf("[daibutsu:debug] "x"\n", ##__VA_ARGS__); \
} while(0)
#else
#define DEVLOG(x, ...)
#endif

#endif
