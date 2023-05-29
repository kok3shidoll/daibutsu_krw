#ifndef UAF_H
#define UAF_H

#include <stddef.h>

#include <io.h>
#include <common.h>

addr_t* uaf_rop_stack(void);
void uaf_rop(void);

#endif
