# page_walker
Linux Kernel Driver that converts virtual address of a process to physical address

## TODO
- convert virt address to physical address related to the process
- read out process memory from the converted physical address


## Commands to build and test
```bash
make
sudo insmod page_walker.ko
sudo dmesg | tail
sudo rmmod page_walker
```
