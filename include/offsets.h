#ifndef OFFSETS_H
#define OFFSETS_H

#include <stdbool.h>
#include <stdint.h>
#include <common.h>

addr_t off_vtab(void);

enum koffsets {
    off_OSSerializer_serialize,   // OSSerializer::serialize
    off_OSSymbol_getMetaClass,    // OSSymbol::getMetaClass
    off_calend_gettime,           // calend_gettime
    off_bufattr_cpx,              // _bufattr_cpx
    off_clock_ops,                // clock_ops
    off_copyin,                   // _copyin
    off_bx_lr,                    // BX LR /* armv7 */
    off_write_gadget,             // write_gadget: str r1, [r0, #0xc] , bx lr /* armv7 */
    off_vm_kernel_addrperm,       // vm_kernel_addrperm
    off_kernel_pmap,              // kernel_pmap
    off_flush_dcache,             // flush_dcache
    off_invalidate_tlb,           // invalidate_tlb
    off_task_for_pid,             // task_for_pid
    off_pid_check,                // pid_check_addr offset
    off_posix_check,              // posix_check_ret_addr offset
    off_mac_proc_check,           // mac_proc_check_ret_addr offset
    off_allproc,                  // allproc
    off_p_pid,                    // proc_t::p_pid
    off_p_ucred,                  // proc_t::p_ucred
    /*
     __ZTV8OSString:        // vtable for OSString
              ffffff8002496cd0      dq  0x0000000000000000
              ffffff8002496cd8      dq  0x0000000000000000
     this -> [ffffff8002496ce0]     dq  0xffffff80023a2fdc

     */
    off_reg_vtab,                 // vtab
    off_ipc_port_ip_kobject,      // IPC_PORT_IP_KOBJECT
    off_task_bsd_info,            // TASK_BSD_INFO
    off_p_fd,                     // p->p_fd
    off_filedesc_fd_ofiles,       // FILEDESC_FD_OFILES
    off_fileproc_f_fglob,         // FILEPROC_F_FGLOB
    off_fileglob_fg_data,         // FILEGLOB_FG_DATA
    off_pipe_buffer,              // KSTRUCT_OFFSET_PIPE_BUFFER
};

extern addr_t* offsets;
addr_t koffset(enum koffsets offset);

#endif
