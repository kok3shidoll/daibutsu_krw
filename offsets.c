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
static addr_t S5L8950X_12H321[] =
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
static addr_t S5L8940X_12H321[] =
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
static addr_t S5L8950X_12H143[] =
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
static addr_t S5L8940X_12H143[] =
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


/* 8.3 (untested, by offsetfinder) */
static addr_t S5L8950X_12F69[] =
{   // n42
    0x802da6e5, // OSSerializer_serialize
    0x802dc805, // OSSymbol_getMetaClass
    0x8001e2e1, // calend_gettime
    0x800c75f5, // bufattr_cpx
    0x803b2cdc, // clock_ops
    0x800b484c, // copyin
    0x800c75f6, // bx_lr
    0x803f9254, // vm_kernel_addrperm
    0x8003a191, // current_task
    0x80016255, // ipc_port_make_send
    0x800162dd, // ipc_port_copyout_send
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
static addr_t S5L8940X_12F69[] =
{   // n94
    0x802d5939, // OSSerializer_serialize
    0x802d7a19, // OSSymbol_getMetaClass
    0x8001e061, // calend_gettime
    0x800c46f9, // bufattr_cpx
    0x803adcdc, // clock_ops
    0x800b2724, // copyin
    0x800c46fa, // bx_lr
    0x803f4124, // vm_kernel_addrperm
    0x80039c5d, // current_task
    0x80016039, // ipc_port_make_send
    0x800160c1, // ipc_port_copyout_send
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


/* 8.2 (untested, by offsetfinder) */
static addr_t S5L8950X_12D508[] =
{   // n42
    0x802ce791, // OSSerializer_serialize
    0x802d08b1, // OSSymbol_getMetaClass
    0x8001d21d, // calend_gettime
    0x800c4825, // bufattr_cpx
    0x803a5ce0, // clock_ops
    0x800b186c, // copyin
    0x800c4826, // bx_lr
    0x803ec200, // vm_kernel_addrperm
    0x80038089, // current_task
    0x80015221, // ipc_port_make_send
    0x800152a9, // ipc_port_copyout_send
    0x803fd098, // kernel_task
    0x80391c88, // vtab
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
static addr_t S5L8940X_12D508[] =
{   // n94
    0x802c9a1d, // OSSerializer_serialize
    0x802cbafd, // OSSymbol_getMetaClass
    0x8001cf39, // calend_gettime
    0x800c18f9, // bufattr_cpx
    0x8039fce0, // clock_ops
    0x800af744, // copyin
    0x800c18fa, // bx_lr
    0x803e60d0, // vm_kernel_addrperm
    0x80037ab5, // current_task
    0x80014f7d, // ipc_port_make_send
    0x80015005, // ipc_port_copyout_send
    0x803f7098, // kernel_task
    0x8038bc88, // vtab
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


/* 8.1.3 (untested, by offsetfinder) */
static addr_t S5L8950X_12B466[] =
{   // n42
    0x802cb9a1, // OSSerializer_serialize
    0x802cdac1, // OSSymbol_getMetaClass
    0x8001cf5d, // calend_gettime
    0x800c14c5, // bufattr_cpx
    0x803a2ce0, // clock_ops
    0x800ae86c, // copyin
    0x800c14c6, // bx_lr
    0x803e9208, // vm_kernel_addrperm
    0x80038bd5, // current_task
    0x80014f19, // ipc_port_make_send
    0x80014fa1, // ipc_port_copyout_send
    0x803fb098, // kernel_task
    0x8038ece8, // vtab
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
static addr_t S5L8940X_12B466[] =
{   // n94
    0x802c6bd1, // OSSerializer_serialize
    0x802c8cb1, // OSSymbol_getMetaClass
    0x8001cb99, // calend_gettime
    0x800be589, // bufattr_cpx
    0x8039dce0, // clock_ops
    0x800ac724, // copyin
    0x800be58a, // bx_lr
    0x803e40d8, // vm_kernel_addrperm
    0x800385ed, // current_task
    0x80014c3d, // ipc_port_make_send
    0x80014cc5, // ipc_port_copyout_send
    0x803f6098, // kernel_task
    0x80389ce8, // vtab
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


/* 8.1.1, 8.1.2 (untested, by offsetfinder) */
static addr_t S5L8950X_12B435[] =
{   // n42
    0x802cb861, // OSSerializer_serialize
    0x802cd981, // OSSymbol_getMetaClass
    0x8001ce85, // calend_gettime
    0x800c1305, // bufattr_cpx
    0x803a2ce0, // clock_ops
    0x800ae86c, // copyin
    0x800c1306, // bx_lr
    0x803e9208, // vm_kernel_addrperm
    0x80038b95, // current_task
    0x80014e8d, // ipc_port_make_send
    0x80014f15, // ipc_port_copyout_send
    0x803fb098, // kernel_task
    0x8038ece8, // vtab
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
static addr_t S5L8940X_12B435[] =
{   // n94
    0x802c6809, // OSSerializer_serialize
    0x802c88e9, // OSSymbol_getMetaClass
    0x8001caa9, // calend_gettime
    0x800be339, // bufattr_cpx
    0x8039cce0, // clock_ops
    0x800ac744, // copyin
    0x800be33a, // bx_lr
    0x803e30d8, // vm_kernel_addrperm
    0x80038499, // current_task
    0x80014b9d, // ipc_port_make_send
    0x80014c25, // ipc_port_copyout_send
    0x803f5098, // kernel_task
    0x80388ce8, // vtab
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


/* 8.1 (untested, by offsetfinder) */
static addr_t S5L8950X_12B410[] =
{   // n42
    0x802cb3e1, // OSSerializer_serialize
    0x802cd501, // OSSymbol_getMetaClass
    0x8001ce61, // calend_gettime
    0x800c12f5, // bufattr_cpx
    0x803a2ce0, // clock_ops
    0x800ae86c, // copyin
    0x800c12f6, // bx_lr
    0x803e91f8, // vm_kernel_addrperm
    0x80038b75, // current_task
    0x80014e69, // ipc_port_make_send
    0x80014ef1, // ipc_port_copyout_send
    0x803fb098, // kernel_task
    0x8038ece8, // vtab
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
static addr_t S5L8940X_12B410[] =
{   // n94
    0x802c6389, // OSSerializer_serialize
    0x802c8469, // OSSymbol_getMetaClass
    0x8001ca85, // calend_gettime
    0x800be319, // bufattr_cpx
    0x8039cce0, // clock_ops
    0x800ac724, // copyin
    0x800be31a, // bx_lr
    0x803e30c8, // vm_kernel_addrperm
    0x80038469, // current_task
    0x80014b79, // ipc_port_make_send
    0x80014c01, // ipc_port_copyout_send
    0x803f5098, // kernel_task
    0x80388ce8, // vtab
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


/* 8.0.1, 8.0.2 (untested, by offsetfinder) */
static addr_t S5L8950X_12A402[] =
{   // n42
    0x802cb381, // OSSerializer_serialize
    0x802cd4a1, // OSSymbol_getMetaClass
    0x8001ce5d, // calend_gettime
    0x800c12f5, // bufattr_cpx
    0x803a2ce0, // clock_ops
    0x800ae86c, // copyin
    0x800c12f6, // bx_lr
    0x803e91f8, // vm_kernel_addrperm
    0x80038b69, // current_task
    0x80014e69, // ipc_port_make_send
    0x80014ef1, // ipc_port_copyout_send
    0x803fb098, // kernel_task
    0x8038ece8, // vtab
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
static addr_t S5L8940X_12A402[] =
{   // n94
    0x802c6309, // OSSerializer_serialize
    0x802c83e9, // OSSymbol_getMetaClass
    0x8001ca81, // calend_gettime
    0x800be319, // bufattr_cpx
    0x8039cce0, // clock_ops
    0x800ac724, // copyin
    0x800be31a, // bx_lr
    0x803e30c8, // vm_kernel_addrperm
    0x80038455, // current_task
    0x80014b79, // ipc_port_make_send
    0x80014c01, // ipc_port_copyout_send
    0x803f5098, // kernel_task
    0x80388ce8, // vtab
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

/* 8.0 (untested, by offsetfinder) */
static addr_t S5L8950X_12A365[] =
{   // n42
    0x802cb381, // OSSerializer_serialize
    0x802cd4a1, // OSSymbol_getMetaClass
    0x8001ce5d, // calend_gettime
    0x800c12f5, // bufattr_cpx
    0x803a2ce0, // clock_ops
    0x800ae86c, // copyin
    0x800c12f6, // bx_lr
    0x803e91f8, // vm_kernel_addrperm
    0x80038b69, // current_task
    0x80014e69, // ipc_port_make_send
    0x80014ef1, // ipc_port_copyout_send
    0x803fb098, // kernel_task
    0x8038ece8, // vtab
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
static addr_t S5L8940X_12A365[] =
{   // n94
    0x802c6309, // OSSerializer_serialize
    0x802c83e9, // OSSymbol_getMetaClass
    0x8001ca81, // calend_gettime
    0x800be319, // bufattr_cpx
    0x8039cce0, // clock_ops
    0x800ac724, // copyin
    0x800be31a, // bx_lr
    0x803e30c8, // vm_kernel_addrperm
    0x80038455, // current_task
    0x80014b79, // ipc_port_make_send
    0x80014c01, // ipc_port_copyout_send
    0x803f5098, // kernel_task
    0x80388ce8, // vtab
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
        uint16_t cpid = 0;
        struct utsname u = {};
        uname(&u);
        
        offsets = NULL;
        
        LOG("kern.version: %s", u.version);
        
        if(     strstr(u.version, "S5L8940X") != NULL) cpid = 0x8940;
        else if(strstr(u.version, "S5L8942X") != NULL) cpid = 0x8942;
        else if(strstr(u.version, "S5L8945X") != NULL) cpid = 0x8945;
        else if(strstr(u.version, "S5L8950X") != NULL) cpid = 0x8950;
        else if(strstr(u.version, "S5L8955X") != NULL) cpid = 0x8955;
        else                                           cpid = 0x0000;
        
        if(cpid)
        {
            if(strstr(u.version, "xnu-2784.40.6~18") != NULL)
            {
                /* 8.4.1: 12H321 */
                if((cpid & 0xFFF0) == 0x8950) offsets = S5L8950X_12H321;
                if((cpid & 0xFFF0) == 0x8940) offsets = S5L8940X_12H321;
            }
            else if(strstr(u.version, "xnu-2784.30.7~30") != NULL)
            {
                /* 8.4: 12H143 */
                if((cpid & 0xFFF0) == 0x8950) offsets = S5L8950X_12H143;
                if((cpid & 0xFFF0) == 0x8940) offsets = S5L8940X_12H143;
            }
            else if(strstr(u.version, "xnu-2784.20.34~2") != NULL)
            {
                /* 8.3: 12F69 */
                /* 8.3: 12F70 */
                if((cpid & 0xFFF0) == 0x8950) offsets = S5L8950X_12F69;
                if((cpid & 0xFFF0) == 0x8940) offsets = S5L8940X_12F69;
            }
            else if(strstr(u.version, "xnu-2783.5.38~5") != NULL)
            {
                /* 8.2: 12D508 */
                if((cpid & 0xFFF0) == 0x8950) offsets = S5L8950X_12D508;
                if((cpid & 0xFFF0) == 0x8940) offsets = S5L8940X_12D508;
            }
            else if(strstr(u.version, "xnu-2783.3.26~3") != NULL)
            {
                /* 8.1.3: 12B466 */
                if((cpid & 0xFFF0) == 0x8950) offsets = S5L8950X_12B466;
                if((cpid & 0xFFF0) == 0x8940) offsets = S5L8940X_12B466;
            }
            else if(strstr(u.version, "xnu-2783.3.22~1") != NULL)
            {
                /* 8.1.1: 12B435 */
                /* 8.1.2: 12B440 */
                if((cpid & 0xFFF0) == 0x8950) offsets = S5L8950X_12B435;
                if((cpid & 0xFFF0) == 0x8940) offsets = S5L8940X_12B435;
            }
            else if(strstr(u.version, "xnu-2783.3.13~4") != NULL)
            {
                /* 8.1: 12B410 */
                /* 8.1: 12B411 */
                if((cpid & 0xFFF0) == 0x8950) offsets = S5L8950X_12B410;
                if((cpid & 0xFFF0) == 0x8940) offsets = S5L8940X_12B410;
            }
            else if(strstr(u.version, "xnu-2783.1.72~23") != NULL)
            {
                /* 8.0.1: 12A402 */
                /* 8.0.2: 12A405 */
                if((cpid & 0xFFF0) == 0x8950) offsets = S5L8950X_12A402;
                if((cpid & 0xFFF0) == 0x8940) offsets = S5L8940X_12A402;
            }
            else if(strstr(u.version, "xnu-2783.1.72~8") != NULL)
            {
                /* 8.0: 12A365 */
                if((cpid & 0xFFF0) == 0x8950) offsets = S5L8950X_12A365;
                if((cpid & 0xFFF0) == 0x8940) offsets = S5L8940X_12A365;
            }
            else
            {
                ERR("unsupported version");
                offsets = NULL;
            }
        }
        
        if(offsets != NULL) init = true;
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


