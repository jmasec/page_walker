# page_walker
Linux Kernel Driver that converts virtual address of a process to physical address

## Commands to build and test
```bash
make
sudo insmod driver.ko
sudo dmesg | tail
sudo rmmod driver
```
