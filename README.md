# page_walker
Linux Kernel Driver that converts virtual address of a process to physical address

## TODO
- Add some error checking for incorrect pages 
- Add support for 5 page level checking p4d
- Test on a running process that is not the controlled test process (use gdb to get the value manually to check test)

## Commands to build and test
In test directory, it has a test target file that you can run and copy a controlled value within a known heap value. It will print out its PID and virtual address that the controlled value is at, it then hangs on a getchar to keep the process running
```bash
gcc target.c -o target
```

Also in the test directory is the pgwlk_usercall file, this is the ioctl() call to the driver that takes the PID and virtual addresses as arguments to send to the driver  
```bash
gcc pgwlk_usercall.c -o usercall
```

```bash
make
sudo insmod page_walker.ko
sudo dmesg | tail
sudo rmmod page_walker
```
