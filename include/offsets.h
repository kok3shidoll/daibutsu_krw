#ifndef OFFSETS_H
#define OFFSETS_H

#include <stdbool.h>
#include <stdint.h>
#include <common.h>

addr_t off_vtab(void);

enum koffsets {
    off_OSSerializer_serialize, // OSSerializer::serialize
    off_OSSymbol_getMetaClass,  // OSSymbol::getMetaClass
    off_calend_gettime,         // calend_gettime
    off_bufattr_cpx,            // _bufattr_cpx
    off_clock_ops,              // clock_ops
    off_copyin,                 // _copyin
    off_bx_lr,                  // ret
    off_vm_kernel_addrperm,     // vm_kernel_addrperm
    off_current_task,           // current_task
    off_ipc_port_make_send,     // ipc_port_make_send
    off_ipc_port_copyout_send,  // ipc_port_copyout_send
    off_kernel_task,
    /*
     __ZTV8OSString:            // vtable for OSString
              ffffff8002496cd0      dq  0x0000000000000000
              ffffff8002496cd8      dq  0x0000000000000000
     this -> [ffffff8002496ce0]     dq  0xffffff80023a2fdc

     */
    off_reg_vtab,               // vtab
    off_ipc_port_ip_kobject,    // IPC_PORT_IP_KOBJECT
    off_task_itk_self,          // task->itk_self
    off_task_itk_space,         // task->itk_space
    off_task_bsd_info,          // TASK_BSD_INFO
    off_p_fd,                   // p->p_fd
    off_filedesc_fd_ofiles,     // FILEDESC_FD_OFILES
    off_fileproc_f_fglob,       // FILEPROC_F_FGLOB
    off_fileglob_fg_data,       // FILEGLOB_FG_DATA
    off_pipe_buffer,            // KSTRUCT_OFFSET_PIPE_BUFFER
};

extern addr_t* offsets;
addr_t koffset(enum koffsets offset);

#endif
