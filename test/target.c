#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/mman.h>

#define TEST_VALUE 0xDEADBEEFCAFEBABEULL

int main(void)
{
    size_t page_size = sysconf(_SC_PAGESIZE);

    // Allocate one page of anonymous memory
    void *memory = mmap(
        NULL,
        page_size,
        PROT_READ | PROT_WRITE,
        MAP_PRIVATE | MAP_ANONYMOUS,
        -1,
        0
    );

    if (memory == MAP_FAILED) {
        perror("mmap");
        return 1;
    }

    // Write a recognizable 64-bit value
    volatile uint64_t *target = (volatile uint64_t *)memory;
    *target = TEST_VALUE;

    printf("PID:             %d\n", getpid());
    printf("Virtual Address: %p\n", memory);
    printf("Stored Value:    0x%016llx\n",
           (unsigned long long)*target);
    printf("Page Size:       %zu\n", page_size);

    fflush(stdout);

    // Keep the process alive
    printf("\nPress ENTER to exit...\n");
    getchar();

    munmap(memory, page_size);

    return 0;
}
