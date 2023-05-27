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

/* 8.4.1 */
static addr_t koffsets_S5L895xX_12H321[] =
{
    0x802da865, // OSSerializer::serialize
    0x802dc985, // OSSymbol::getMetaClass
    0x8001e301, // calend_gettime
    0x800c75f5, // _bufattr_cpx
    0x803b2cdc, // clock_ops
    0x800b486c, // _copyin
    0x800c75f6, // BX LR
    0x803f9258, // vm_kernel_addrperm
    0x8003a3f1, // current_task
    0x80016275, // ipc_port_make_send
    0x800162fd, // ipc_port_copyout_send
    0x8040a098, // kernel_task
    0x8039dd08, // vtab
    0x00000044, // IPC_PORT_IP_KOBJECT
    0x000000a4, // task->itk_self
    0x000001a8, // task->itk_space
    0x000001f0, // TASK_BSD_INFO
    0x00000090, // p->p_fd
    0x00000000, // FILEDESC_FD_OFILES
    0x00000008, // FILEPROC_F_FGLOB
    0x00000028, // FILEGLOB_FG_DATA
    0x00000010, // KSTRUCT_OFFSET_PIPE_BUFFER
};
static addr_t koffsets_S5L894xX_12H321[] =
{   // untested
    0x802d5a1d, // OSSerializer::serialize
    0x802d7afd, // OSSymbol::getMetaClass
    0x8001e0a1, // calend_gettime
    0x800c4719, // _bufattr_cpx
    0x803adcdc, // clock_ops
    0x800b2744, // _copyin
    0x800c471a, // BX LR
    0x803f4128, // vm_kernel_addrperm
    0x80039f2d, // current_task
    0x80016079, // ipc_port_make_send
    0x80016101, // ipc_port_copyout_send
    0x80405098, // kernel_task
    0x80398d08, // vtab
    0x00000044, // IPC_PORT_IP_KOBJECT
    0x000000a4, // task->itk_self
    0x000001a8, // task->itk_space
    0x000001f0, // TASK_BSD_INFO
    0x00000090, // p->p_fd
    0x00000000, // FILEDESC_FD_OFILES
    0x00000008, // FILEPROC_F_FGLOB
    0x00000028, // FILEGLOB_FG_DATA
    0x00000010, // KSTRUCT_OFFSET_PIPE_BUFFER
};

/* 8.4 (untested, by offsetfinder) */
static addr_t koffsets_S5L895xX_12H143[] =
{   // n42
    0x802da759, // OSSerializer_serialize
    0x802dc879, // OSSymbol_getMetaClass
    0x8001e301, // calend_gettime
    0x800c75f5, // bufattr_cpx
    0x803b2cdc, // clock_ops
    0x800b486c, // copyin
    0x800c75f6, // bx_lr
    0x803f9258, // vm_kernel_addrperm
    0x8003a3f1, // current_task
    0x80016275, // ipc_port_make_send
    0x800162fd, // ipc_port_copyout_send
    0x8040a098, // kernel_task
    0x8039dd08, // vtab
    0x00000044, // ip_kobject
    0x000000a4, // itk_self
    0x000001a8, // task->itk_space
    0x000001f0, // TASK_BSD_INFO
    0x00000090, // p->p_fd
    0x00000000, // FILEDESC_FD_OFILES
    0x00000008, // FILEPROC_F_FGLOB
    0x00000028, // FILEGLOB_FG_DATA
    0x00000010, // KSTRUCT_OFFSET_PIPE_BUFFER
};
static addr_t koffsets_S5L894xX_12H143[] =
{   // n94
    0x802d59b9, // OSSerializer_serialize
    0x802d7a99, // OSSymbol_getMetaClass
    0x8001e0a1, // calend_gettime
    0x800c46f9, // bufattr_cpx
    0x803adcdc, // clock_ops
    0x800b2724, // copyin
    0x800c46fa, // bx_lr
    0x803f4128, // vm_kernel_addrperm
    0x80039f2d, // current_task
    0x80016079, // ipc_port_make_send
    0x80016101, // ipc_port_copyout_send
    0x80405098, // kernel_task
    0x80398d08, // vtab
    0x00000044, // ip_kobject
    0x000000a4, // itk_self
    0x000001a8, // task->itk_space
    0x000001f0, // TASK_BSD_INFO
    0x00000090, // p->p_fd
    0x00000000, // FILEDESC_FD_OFILES
    0x00000008, // FILEPROC_F_FGLOB
    0x00000028, // FILEGLOB_FG_DATA
    0x00000010, // KSTRUCT_OFFSET_PIPE_BUFFER
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


