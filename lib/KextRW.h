#ifndef LIBKEXTRW_H
#define LIBKEXTRW_H

#include <stdio.h>
#include <mach/mach.h>
#include <IOKit/IOKitLib.h>
#include <mach-o/loader.h>

#define PAGE_ALIGN(x) (x & ~PAGE_MASK)

#define STATIC_KERNEL_BASE (0xfffffe0007004000)

uint64_t gKernelBase = 0, gKernelSlide = 0;

// gKernelSlide matches the kernel slide in a panic log,
// but we need to use the KernelCache slide, which is
// always 0x8000 less than the normal kernel slide.
#define kslide(x) (x + gKernelSlide - 0x8000)

io_connect_t gClient = MACH_PORT_NULL;

static inline io_connect_t kextrw_open(void)
{
    io_service_t service = IOServiceGetMatchingService(kIOMainPortDefault, IOServiceMatching("KextRW"));
    if(!MACH_PORT_VALID(service))
    {
        return MACH_PORT_NULL;
    }

    io_connect_t client = MACH_PORT_NULL;
    kern_return_t ret = IOServiceOpen(service, mach_task_self(), 0, &client);
    IOObjectRelease(service);
    if(ret != KERN_SUCCESS)
    {
        return MACH_PORT_NULL;
    }
    return client;
}

static inline kern_return_t kextrw_kread(io_connect_t client, uint64_t from, void *to, uint64_t len)
{
    uint64_t in[] = { from, (uint64_t)to, len };
    return IOConnectCallScalarMethod(client, 0, in, 3, NULL, NULL);
}

static inline kern_return_t kextrw_kwrite(io_connect_t client, void *from, uint64_t to, uint64_t len)
{
    uint64_t in[] = { (uint64_t)from, to, len };
    return IOConnectCallScalarMethod(client, 1, in, 3, NULL, NULL);
}

static inline kern_return_t kextrw_physread(io_connect_t client, uint64_t from, void *to, uint64_t len, uint8_t align)
{
    uint64_t in[] = { from, (uint64_t)to, len, align };
    return IOConnectCallScalarMethod(client, 2, in, 4, NULL, NULL);
}

static inline kern_return_t kextrw_physwrite(io_connect_t client, void *from, uint64_t to, uint64_t len, uint8_t align)
{
    uint64_t in[] = { (uint64_t)from, to, len, align };
    return IOConnectCallScalarMethod(client, 3, in, 4, NULL, NULL);
}

static inline kern_return_t kextrw_get_reset_vector(io_connect_t client, uint64_t *out)
{
    uint32_t outCnt = 1; 
    return IOConnectCallScalarMethod(client, 4, NULL, 0, out, &outCnt);
}

static inline kern_return_t kextrw_kvtophys(io_connect_t client, uint64_t va, uint64_t *out)
{
    uint64_t in = va;
    uint32_t outCnt = 1;
    return IOConnectCallScalarMethod(client, 5, &in, 1, out, &outCnt);
}

static inline kern_return_t kextrw_phystokv(io_connect_t client, uint64_t pa, uint64_t *out)
{
    uint64_t in = pa;
    uint32_t outCnt = 1;
    return IOConnectCallScalarMethod(client, 6, &in, 1, out, &outCnt);
}

static inline kern_return_t kextrw_kcall(io_connect_t client, uint64_t fn, uint64_t *args, uint32_t argsCnt, uint64_t *out)
{
    if (argsCnt > 9) return KERN_INVALID_ARGUMENT;
    uint64_t argsBuf[9] = { 0 };
    argsBuf[0] = fn;
    for (uint32_t i = 0; i < argsCnt; i++)
    {
        if (args[i]) argsBuf[i + 1] = args[i];
    }
    uint32_t outCnt = 1;
    uint64_t rv = 0;
    IOReturn ret = IOConnectCallScalarMethod(client, 7, argsBuf, argsCnt + 1, &rv, &outCnt);
    if (out) *out = rv;
    return ret;
}

static inline kern_return_t kextrw_kalloc(io_connect_t client, uint64_t size, uint64_t *out)
{
    uint64_t in = size;
    uint32_t outCnt = 1;
    return IOConnectCallScalarMethod(client, 8, &in, 1, out, &outCnt);
}

static inline kern_return_t kextrw_kfree(io_connect_t client, uint64_t addr, uint64_t size)
{
    uint64_t in[] = { addr, size };
    return IOConnectCallScalarMethod(client, 9, in, 2, NULL, NULL);
}

void kextrw_close(io_connect_t client)
{
    IOServiceClose(client);
}

int kextrw_init(void)
{
    gClient = kextrw_open();
    return MACH_PORT_VALID(gClient) ? 0 : -1;
}

void kextrw_deinit(void)
{
    if (MACH_PORT_VALID(gClient))
    {
        kextrw_close(gClient);
    }
}

/* Virtual read/write */

uint8_t kread8(uint64_t addr)
{
    uint8_t val = 0;
    kextrw_kread(gClient, addr, &val, sizeof(val));
    return val;
}

uint16_t kread16(uint64_t addr)
{
    uint16_t val = 0;
    kextrw_kread(gClient, addr, &val, sizeof(val));
    return val;
}

uint32_t kread32(uint64_t addr)
{
    uint32_t val = 0;
    kextrw_kread(gClient, addr, &val, sizeof(val));
    return val;
}

uint64_t kread64(uint64_t addr)
{
    uint64_t val = 0;
    kextrw_kread(gClient, addr, &val, sizeof(val));
    return val;
}

int kreadbuf(uint64_t addr, void *buf, size_t len)
{
    return kextrw_kread(gClient, addr, buf, len);
}

void kwrite8(uint64_t addr, uint8_t val)
{
    kextrw_kwrite(gClient, &val, addr, sizeof(val));
}

void kwrite16(uint64_t addr, uint16_t val)
{
    kextrw_kwrite(gClient, &val, addr, sizeof(val));
}

void kwrite32(uint64_t addr, uint32_t val)
{
    kextrw_kwrite(gClient, &val, addr, sizeof(val));
}

void kwrite64(uint64_t addr, uint64_t val)
{
    kextrw_kwrite(gClient, &val, addr, sizeof(val));
}

int kwritebuf(uint64_t addr, void *buf, size_t len)
{
    return kextrw_kwrite(gClient, buf, addr, len);
}

static uint64_t xpaci(uint64_t pointer)
{
	asm("xpaci %[value]\n" : [value] "+r"(pointer));
	return pointer;
}

uint64_t kreadptr(uint64_t addr)
{
    return xpaci(kread64(addr));
}

/* Physical read/write */

uint8_t physread8(uint64_t addr)
{
    uint8_t val = 0;
    kextrw_physread(gClient, addr, &val, sizeof(val), 1);
    return val;
}

uint16_t physread16(uint64_t addr)
{
    uint16_t val = 0;
    kextrw_physread(gClient, addr, &val, sizeof(val), 2);
    return val;
}

uint32_t physread32(uint64_t addr)
{
    uint32_t val = 0;
    kextrw_physread(gClient, addr, &val, sizeof(val), 4);
    return val;
}

uint64_t physread64(uint64_t addr)
{
    uint64_t val = 0;
    kextrw_physread(gClient, addr, &val, sizeof(val), 8);
    return val;
}

int physreadbuf(uint64_t addr, void *buf, size_t len)
{
    return kextrw_physread(gClient, addr, buf, len, 1);
}

void physwrite8(uint64_t addr, uint8_t val)
{
    kextrw_physwrite(gClient, &val, addr, sizeof(val), 1);
}

void physwrite16(uint64_t addr, uint16_t val)
{
    kextrw_physwrite(gClient, &val, addr, sizeof(val), 2);
}

void physwrite32(uint64_t addr, uint32_t val)
{
    kextrw_physwrite(gClient, &val, addr, sizeof(val), 4);
}

void physwrite64(uint64_t addr, uint64_t val)
{
    kextrw_physwrite(gClient, &val, addr, sizeof(val), 8);
}

int physwritebuf(uint64_t addr, void *buf, size_t len)
{
    return kextrw_physwrite(gClient, buf, addr, len, 1);
}

/* Kernel call */

uint64_t kcall(uint64_t fn, uint64_t *args, uint32_t argsCnt)
{
    uint64_t rv = 0;
    kextrw_kcall(gClient, fn, args, argsCnt, &rv);
    return rv;
}

/* Translation */

uint64_t kvtophys(uint64_t va)
{
    uint64_t pa = 0;
    kextrw_kvtophys(gClient, va, &pa);
    return pa;
}

uint64_t phystokv(uint64_t pa)
{
    uint64_t va = 0;
    kextrw_phystokv(gClient, pa, &va);
    return va;
}

/* Allocations */

uint64_t kalloc(uint64_t size)
{
    uint64_t addr = 0;
    kextrw_kalloc(gClient, size, &addr);
    return addr;
}

void kfree(uint64_t addr, uint64_t size)
{
    kextrw_kfree(gClient, addr, size);
}

/* Utilities */

uint64_t get_kernel_base()
{
    uint64_t kernelPage = 0;
    kextrw_get_reset_vector(gClient, &kernelPage);
    if (!kernelPage) return 0;

    uint64_t kernelBase = 0;
    while (!kernelBase) {
        if (kread32(kernelPage) == MH_MAGIC_64
            && kread32(kernelPage + 0xC) == MH_EXECUTE) {
            kernelBase = kernelPage;
            break;
        }
        kernelPage -= PAGE_SIZE;
    }

    gKernelSlide = kernelBase - STATIC_KERNEL_BASE;

    return kernelBase;
}

#endif // LIBKEXTRW_H