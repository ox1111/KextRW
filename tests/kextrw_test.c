#include <lib/KextRW.h>

#include <mach-o/loader.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

#define PMAP_ENTER_OPTIONS kslide(0xFFFFFE0008869DA8)
#define ML_STATIC_PTOVIRT kslide(0xFFFFFE000888F29C)
#define PANIC kslide(0xFFFFFE0008F99D88)
#define KAUTH_CRED_PROC_REF kslide(0xFFFFFE0008C30EF4)
#define KAUTH_CRED_UNREF kslide(0xFFFFFE0008C32188)

#define KERNPROC kslide(0xFFFFFE0007CA6F38)
#define KERNEL_TASK kslide(0xFFFFFE0007CA5DF0)
#define TASK_SIZE kslide(0xFFFFFE000C049520)

#define off_proc_pid (0x60)
#define off_proc_next (0x0)
#define off_proc_prev (0x8)
#define off_task_map (0x28)
#define off_vm_map_pmap (0x40)

// USE THE DEFINITIONS! For example:
// koffsetof(proc, pid) = off_proc_pid
#define koffsetof(type, field) off_##type##_##field

void panic(const char *msg, ...) {
    char *panicMessage = NULL;
    va_list args;
    va_start(args, msg);
    vasprintf(&panicMessage, msg, args);
    va_end(args);

    /*
    uint32_t panic_instr[] = {
        0xDAC143E6, 0x910023E1,
        0x52800002,
    };
    uint64_t panic = 0;
    for (uint64_t addr = start; addr < end; addr += 4) {
        void *buf = malloc(sizeof(panic_instr));
        kreadbuf(addr, buf, sizeof(panic_instr));
        if (!memcmp(buf, panic_instr, sizeof(panic_instr))) {
            printf("Found panic @ 0x%llX\n", addr - 0x1C);
            printf("Expected: 0x%llX\n", kslide(0xFFFFFE0008F99D88));
            panic = addr - 0x1C;
            break;
        }
    }
    */
    // #define PANIC_MSG "alfiecg_dev calling panic() - kcall success!"
    // uint64_t panicMsgBuf = kalloc(strlen(PANIC_MSG) + 1);
    // kwritebuf(panicMsgBuf, PANIC_MSG, strlen(PANIC_MSG) + 1);
    // printf("Panic message: %s\n", PANIC_MSG);
    // sleep(2);
    // kcall(panic, (uint64_t []){ panicMsgBuf, 0, 0, 0, 0, 0, 0, 0 }, 8);
    // kfree(panicMsgBuf, strlen(PANIC_MSG) + 1);

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

    uint64_t alloc = kalloc(0x1000);
    printf("kalloc(0x1000) -> 0x%llX\n", alloc);
    kfree(alloc, 0x1000);

    struct mach_header_64 header = { 0 };
    kreadbuf(kernelBase, &header, sizeof(header));

    void *buffer = malloc(header.sizeofcmds);
    kreadbuf(kernelBase + sizeof(struct mach_header_64), buffer, header.sizeofcmds);

    struct load_command *lc = buffer;

    uint64_t text_exec_base = 0, text_exec_size = 0;
    uint64_t cstring_base = 0, cstring_size = 0;
    while ((uint64_t)lc < (uint64_t)(buffer + header.sizeofcmds)) {
        if (lc->cmd == LC_SEGMENT_64) {
            struct segment_command_64 *seg = (struct segment_command_64 *)lc;

            struct section_64 *sect = (struct section_64 *)(seg + 1);
            for (uint32_t i = 0; i < seg->nsects; i++) {
                sect++;
                if (!strcmp(sect->sectname, "__cstring")
                && !strcmp(sect->segname, "__TEXT")) {
                    cstring_base = sect->addr;
                    cstring_size = sect->size;
                }
                if (!strcmp(sect->segname, "__TEXT_EXEC")
                && !strcmp(sect->sectname, "__text")) {
                    text_exec_base = sect->addr;
                    text_exec_size = sect->size;
                }
            }
        }
        lc = (struct load_command *)((uint8_t *)lc + lc->cmdsize);
    }
    free(buffer);

    printf("__TEXT_EXEC,__text: 0x%llX - 0x%llX (0x%llX)\n", text_exec_base, text_exec_base + text_exec_size, text_exec_size);
    printf("__TEXT,__cstring: 0x%llX - 0x%llX (0x%llX)\n", cstring_base, cstring_base + cstring_size, cstring_size);

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

    // uint64_t self_ucred = kcall(KAUTH_CRED_PROC_REF, (uint64_t []){ self_proc, 0, 0, 0, 0, 0, 0, 0 }, 8);
    // This panics and I'm not sure why (zone_require_ro failed: address not in a ro zone)
    // if (self_ucred) {
    //     kcall(KAUTH_CRED_UNREF, (uint64_t []){ self_ucred, 0, 0, 0, 0, 0, 0, 0 }, 8);
    // }

    printf("kernproc: 0x%llX\n", kernproc);
    printf("kernel_task: 0x%llX\n", kernel_task);
    printf("Our proc: 0x%llX\n", self_proc);
    printf("Our task: 0x%llX\n", self_task);
    printf("Our vm_map: 0x%llX\n", self_vm_map);
    printf("Our pmap: 0x%llX\n", self_pmap);
    // printf("Our ucred: 0x%llX\n", self_ucred);

    kextrw_deinit();
    return 0;
}