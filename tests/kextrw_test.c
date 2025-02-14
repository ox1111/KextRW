#include <lib/KextRW.h>

#include <mach-o/loader.h>
#include <stdio.h>

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
            printf("Segment:\n");
            struct segment_command_64 *seg = (struct segment_command_64 *)lc;
            printf("\tSegment name: %s\n", seg->segname);
            printf("\tVM address: 0x%llX\n", seg->vmaddr);
            printf("\tVM size: 0x%llX\n", seg->vmsize);
            printf("\tFile offset: 0x%llX\n", seg->fileoff);
            printf("\tFile size: 0x%llX\n", seg->filesize);
            printf("\tMax protection: 0x%X\n", seg->maxprot);
            printf("\tInitial protection: 0x%X\n", seg->initprot);
            printf("\tNumber of sections: 0x%X\n", seg->nsects);
            printf("\tFlags: 0x%X\n", seg->flags);

            struct section_64 *sect = (struct section_64 *)(seg + 1);
            for (uint32_t i = 0; i < seg->nsects; i++) {
                printf("Section:\n");
                printf("\tName: %s,%s\n", sect->segname, sect->sectname);
                printf("\tVM address: 0x%llX\n", sect->addr);
                printf("\tVM size: 0x%llX\n", sect->size);
                printf("\tFile offset: 0x%X\n", sect->offset);
                printf("\tAlignment: 0x%X\n", sect->align);
                printf("\tFlags: 0x%X\n", sect->flags);
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


    kextrw_deinit();
    return 0;
}