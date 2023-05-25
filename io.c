// siguza's cl0ver
// this file was also used kok3shi9

#include <stddef.h>
#include <stdint.h>

#include <mach/kern_return.h>
#include <mach/mach_error.h>
#include <mach/mach_host.h>
#include <mach/mach_traps.h>
#include <mach/message.h>
#include <mach/port.h>

#include <IOKit/IOKitLib.h>

/* !__LP64__ */
#include <IOKit/iokitmig.h>

extern kern_return_t io_service_open_extended
(
 mach_port_t service,
 task_t owningTask,
 uint32_t connect_type,
 NDR_record_t ndr,
 io_buf_ptr_t properties,
 mach_msg_type_number_t propertiesCnt,
 kern_return_t *result,
 mach_port_t *connection
 );
/* !__LP64__ */

#include <common.h>
#include <plog.h>
#include <io.h>

/* Helper functions */

static mach_port_t get_io_master_port(void)
{
    static mach_port_t master = MACH_PORT_NULL;
    if(master == MACH_PORT_NULL)
    {
        DEVLOG("Getting IO master port...");
        kern_return_t ret = host_get_io_master(mach_host_self(), &master);
        if(ret != KERN_SUCCESS || !MACH_PORT_VALID(master))
        {
            ERR("Failed to get IO master port (port = 0x%08x, ret = %u: %s)", master, ret, mach_error_string(ret));
            return MACH_PORT_NULL;
        }
    }
    return master;
}

io_service_t _io_get_service(void)
{
    static io_service_t service = MACH_PORT_NULL;
    if(service == MACH_PORT_NULL)
    {
        DEVLOG("Getting IO service handle...");
        service = IOServiceGetMatchingService(get_io_master_port(), IOServiceMatching("AppleMobileFileIntegrity"));
        if(!MACH_PORT_VALID(service))
        {
            ERR("Failed to get IO service handle (port = 0x%08x)", service);
            return MACH_PORT_NULL;
        }
    }
    return service;
}

/* Building blocks */
io_connect_t _io_spawn_client(void *dict, size_t dictlen)
{
    DEVLOG("Spawning user client / Parsing dictionary...");
    io_connect_t client = MACH_PORT_NULL;
    kern_return_t err;
    kern_return_t ret = io_service_open_extended(_io_get_service(), mach_task_self(), 0, NDR_record, dict, dictlen, &err, &client);
    if(ret != KERN_SUCCESS || err != KERN_SUCCESS || !MACH_PORT_VALID(client))
    {
        //ERR("Failed to parse dictionary (client = 0x%08x, ret = %u: %s, err = %u: %s)", client, ret, mach_error_string(ret), err, mach_error_string(err));
    }
    return client;
}

io_iterator_t _io_iterator(void)
{
    DEVLOG("Creating dict iterator...");
    io_iterator_t it = 0;
    kern_return_t ret = IORegistryEntryCreateIterator(_io_get_service(), "IOService", kIORegistryIterateRecursively, &it);
    if(ret != KERN_SUCCESS)
    {
        //ERR("Failed to create iterator (ret = %u: %s)", ret, mach_error_string(ret));
    }
    return it;
}

io_object_t _io_next(io_iterator_t it)
{
    DEVLOG("Getting next element from iterator...");
    io_object_t o = IOIteratorNext(it);
    if(o == 0)
    {
        ERR("Failed to get next iterator element");
        return -1;
    }
    return o;
}

void _io_get(io_object_t o, const char *key, void *buf, uint32_t *buflen)
{
    DEVLOG("Retrieving bytes...");
    kern_return_t ret = IORegistryEntryGetProperty(o, key, buf, buflen);
    if(ret != KERN_SUCCESS)
    {
        ERR("Failed to get bytes (ret = %u: %s)", ret, mach_error_string(ret));
        return;
    }
}

void _io_find(const char *key, void *buf, uint32_t *buflen)
{
    io_iterator_t it = _io_iterator();
    {
        io_object_t o;
        bool found = false;
        while(!found && (o = _io_next(it)) != 0)
        {
            if(IORegistryEntryGetProperty(o, key, buf, buflen) == KERN_SUCCESS)
            {
                found = true;
            }
            IOObjectRelease(o);
        }
        if(!found)
        {
            ERR("Failed to find property: %s", key);
            return;
        }
    }
    
    {
        IOObjectRelease(it);
    }
}

void _io_release_client(io_connect_t client)
{
    DEVLOG("Releasing user client...");
    kern_return_t ret = IOServiceClose(client);
    if(ret != KERN_SUCCESS)
    {
        //ERROR("Failed to release user client (ret = %u: %s)", ret, mach_error_string(ret));
    }
}

/* All-in-one routines */

void dict_get_bytes(void *dict, size_t dictlen, const char *key, void *buf, uint32_t *buflen)
{
    TIMER_START(timer);
    io_connect_t client = _io_spawn_client(dict, dictlen);
    {
        _io_find(key, buf, buflen);
    }
    
    {
        _io_release_client(client);
    }
    // Async cleanup
    TIMER_SLEEP_UNTIL(timer, 50e6); // 50ms
}

void dict_parse(void *dict, size_t dictlen)
{
    TIMER_START(timer);
    _io_release_client(_io_spawn_client(dict, dictlen));
    // Async cleanup
    TIMER_SLEEP_UNTIL(timer, 50e6); // 50ms
}

static io_service_t _io_spawn_service(void *dict, size_t dictlen)
{
    DEVLOG("Spawning user client / Parsing dictionary...");
    io_connect_t client = MACH_PORT_NULL;
    io_service_t service = _io_get_service();
    kern_return_t err;
    kern_return_t ret = io_service_open_extended(service, mach_task_self(), 0, NDR_record, dict, dictlen, &err, &client);
    if(ret != KERN_SUCCESS || err != KERN_SUCCESS || !MACH_PORT_VALID(client))
    {
        //ERR("Failed to parse dictionary (client = 0x%08x, ret = %u: %s, err = %u: %s)", client, ret, mach_error_string(ret), err, mach_error_string(err));
    }
    return service;
}

io_service_t dict_parse_service(void *dict, size_t dictlen)
{
    TIMER_START(timer);
    //_io_release_client(_io_spawn_service(dict, dictlen));
    io_service_t service = _io_spawn_service(dict, dictlen);
    // Async cleanup
    TIMER_SLEEP_UNTIL(timer, 50e6); // 50ms
    return service;
}
