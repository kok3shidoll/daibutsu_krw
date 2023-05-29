#ifndef ROP_H
#define ROP_H

#include <stddef.h>

#include <io.h>
#include <common.h>

void build_rop_chain(addr_t **chain, task_t *task);

#endif
