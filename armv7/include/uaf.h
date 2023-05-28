#ifndef UAF_H
#define UAF_H

#include <stddef.h>

#include <io.h>
#include <common.h>

void uaf_get_bytes(const OSString *fake, char *buf, size_t len);
void uaf_read_naive(const char *addr, char *buf, size_t len);
addr_t kread32_via_uaf(addr_t addr);

#endif
