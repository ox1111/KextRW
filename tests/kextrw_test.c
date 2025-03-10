#include <lib/KextRW.h>

#include <mach-o/loader.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

// Kernel functions
#define PMAP_ENTER_OPTIONS kslide(0xFFFFFE0008869DA8)
#define ML_STATIC_PTOVIRT kslide(0xFFFFFE000888F29C)
#define PANIC kslide(0xFFFFFE0008F99D88)
#define KAUTH_CRED_PROC_REF kslide(0xFFFFFE0008C30EF4)
#define KAUTH_CRED_UNREF kslide(0xFFFFFE0008C32188)

// Kernel constants and variables
#define KERNPROC kslide(0xFFFFFE0007CA6F38)
#define KERNEL_TASK kslide(0xFFFFFE0007CA5DF0)
#define TASK_SIZE kslide(0xFFFFFE000C049520) // proc->task = proc + sizeof(proc)

// Kernel structure offsets
#define off_proc_pid (0x60)
#define off_proc_next (0x0)
#define off_proc_prev (0x8)
#define off_task_map (0x28)
#define off_vm_map_pmap (0x40)

#define koffsetof(type, field) off_##type##_##field

void panic(const char *msg, ...)
{
    char *panicMessage = NULL;
    va_list args;
    va_start(args, msg);
    vasprintf(&panicMessage, msg, args);
    va_end(args);
    uint64_t panicMsgBuf = kalloc(strlen(panicMessage) + 1);
    kwritebuf(panicMsgBuf, (void *)panicMessage, strlen(panicMessage) + 1);
    kcall(PANIC, (uint64_t []){ panicMsgBuf, 0, 0, 0, 0, 0, 0, 0 }, 8);
    kfree(panicMsgBuf, strlen(panicMessage) + 1);
}

static uint64_t kernproc = 0;

static uint64_t off_proc_task = 0;
static uint64_t proc_find(pid_t pid)
{
    if (!kernproc) return 0;
    uint64_t curProc = kernproc;
    while (curProc > STATIC_KERNEL_BASE) {
        pid_t curPid = kread32(curProc + koffsetof(proc, pid));
        if (curPid == pid) break;
        curProc = kreadptr(curProc + koffsetof(proc, prev));
    }
    return curProc;
}

static uint64_t proc_task(uint64_t proc)
{
    if (!off_proc_task) off_proc_task = kread64(TASK_SIZE);
    return proc + koffsetof(proc, task);
}

static uint64_t task_map(uint64_t task)
{
    return task + koffsetof(task, map);
}

static uint64_t task_pmap(uint64_t task)
{
    return kreadptr(task_map(task) + koffsetof(vm_map, pmap));
}

int main(void) {
    if (kextrw_init() == -1) {
        printf("Failed to initialize KextRW\n");
        return 1;
    }

    uint64_t kernelBase = get_kernel_base();

    printf("Kernel base (VA): 0x%llX\n", kernelBase);
    printf("Kernel base (PA): 0x%llX\n", kvtophys(kernelBase));
    printf("Kernel slide: 0x%llX\n", kslide(0));

    printf("kread32(0x%llX) -> 0x%X\n", kernelBase, kread32(kernelBase));

    uint64_t alloc = kalloc(0x100);
    printf("kalloc(0x100) -> 0x%llX\n", alloc);
    kfree(alloc, 0x100);

    uint64_t kbasePA = kvtophys(kernelBase);
    printf("kvtophys(0x%llX) -> 0x%llX\n", kernelBase, kbasePA);
    printf("physread32(0x%llX) -> 0x%X\n", kbasePA, physread32(kbasePA));

    uint64_t va = kcall(ML_STATIC_PTOVIRT, (uint64_t []){ kbasePA,0x41,0x42,0x43,0x44,0x45,0x46,0x47 }, 8);

    printf("ml_static_ptovirt(0x%llX) -> 0x%llX, kcall success!\n", kbasePA, va);

    kernproc = kreadptr(KERNPROC);
    uint64_t kernel_task = proc_task(kernproc);

    uint64_t self_proc = proc_find(getpid());
    uint64_t self_task = proc_task(self_proc);
    uint64_t self_vm_map = task_map(self_task);
    uint64_t self_pmap = task_pmap(self_task);

    printf("kernproc: 0x%llX\n", kernproc);
    printf("kernel_task: 0x%llX\n", kernel_task);
    printf("Our proc: 0x%llX\n", self_proc);
    printf("Our task: 0x%llX\n", self_task);
    printf("Our vm_map: 0x%llX\n", self_vm_map);
    printf("Our pmap: 0x%llX\n", self_pmap);

    kextrw_deinit();
    return 0;
}