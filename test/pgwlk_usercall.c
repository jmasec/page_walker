#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#define DEVICE_PATH "/dev/chr_dev"

// Must match the ioctl command defined in your driver!
#define PAGE_WALK _IOWR('p', 0, struct pagewlk_args)

struct pagewlk_args {
    unsigned long virt_addr;
    unsigned long phys_addr;
    int pid;
};

int main(int argc, char *argv[])
{
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <pid> <virtual_address>\n", argv[0]);
        return 1;
    }

    struct pagewlk_args args = {0};

    args.pid = atoi(argv[1]);
    args.virt_addr = strtoul(argv[2], NULL, 0);

    int fd = open(DEVICE_PATH, O_RDWR);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    printf("PID: %d\n", args.pid);
    printf("Virtual Address: 0x%lx\n", args.virt_addr);

    if (ioctl(fd, PAGE_WALK, &args) == -1) {
        perror("ioctl");
        close(fd);
        return 1;
    }

    printf("Physical Address: 0x%lx\n", args.phys_addr);

    close(fd);
    return 0;
}
