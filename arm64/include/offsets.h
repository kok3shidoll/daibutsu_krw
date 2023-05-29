#ifndef OFFSETS_H
#define OFFSETS_H

#include <stdbool.h>
#include <stdint.h>
#include <common.h>

addr_t off_vtab(void);

enum koffsets {
    // kslide
    off_reg_anchor,
    // offsets
    OSUnserializeXML_stack,
    is_io_service_open_extended_stack,
    task_itk_space,
    task_itk_self,
    
    /*---- slide ----*/
    // rop gadgets
    gadget_ldp_x9_add_sp_sp_0x10,
    gadget_ldr_x0_sp_0x20_load_x22_x19,
    gadget_add_x0_x0_x19_load_x20_x19,
    gadget_str_x0_x19_load_x20_x19,
    gadget_load_x20_x19,
    gadget_blr_x20_load_x22_x19,
    gadget_ldr_x0_x21_0x5c8_load_x24_x19,
    gadget_OSUnserializeXML_return,
    // functions
    func_current_task,
    func_ipc_port_make_send,
    func_ipc_port_copyout_send,
    func_ldr_x0_x0,
    // frag
    frag_mov_x1_x20_blr_x21,
    // data
    data_kernel_task,
};

extern addr_t* offsets;
bool offsets_initialize(void);
addr_t koffset(enum koffsets offset);
addr_t get_stack_pivot(void);

#endif
