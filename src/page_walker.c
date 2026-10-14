#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/ioctl.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include <linux/pid.h>
#include <linux/sched.h>
#include <linux/rcupdate.h>



MODULE_LICENSE("GPL");
MODULE_AUTHOR("jmasec");
MODULE_DESCRIPTION("Page Walker");
MODULE_VERSION("0.1");

#define PAGE_WALK _IOWR('p', 0, struct pagewlk_args)

dev_t dev = 0;
static struct class *dev_class;
static struct cdev pw_cdev;

struct pagewlk_args{
	unsigned long virt_addr;
	unsigned long phys_addr;
	int pid;
};

struct pagewlk_offsets{
	int64_t pgd_offset;
	int64_t pud_offset;
	int64_t pmd_offset;
	int64_t pt_offset;
	int64_t phys_offset;
};

static int __init pw_init(void);
static void __exit pw_exit(void);
static int pw_open(struct inode *inode, struct file *file);
static int pw_release(struct inode *inode, struct file *file);
static long pw_ioctl(struct file *file, unsigned int cmd, unsigned long arg); 
static int page_walker(struct pagewlk_args *pw_args);
static void get_page_table_offsets(struct pagewlk_offsets *pw_offsets, unsigned long virt_addr);
static struct task_struct* get_task_by_pid(int pid);

static const struct file_operations fops = {
	.owner          = THIS_MODULE,
	.open           = pw_open,
	.release        = pw_release,
	.unlocked_ioctl = pw_ioctl,
};


static int  pw_open(struct inode *inode, struct file *file){
	printk(KERN_INFO "Device File Opened...\n");
	return 0;
}

static int pw_release(struct inode *inode, struct file *file){
	printk(KERN_INFO "Device File Closed...\n");
	return 0;
}

static void get_page_table_offsets(struct pagewlk_offsets *pw_offsets, unsigned long virt_addr){
	pw_offsets->pgd_offset = (virt_addr >> 39);
	pw_offsets->pud_offset = (virt_addr >> 30);
	pw_offsets->pmd_offset = (virt_addr >> 21);
	pw_offsets->pt_offset = (virt_addr >> 12);
	pw_offsets->phys_offset = virt_addr & 0xFFF;
}

static struct task_struct* get_task_by_pid(int pid){
	struct task_struct *task = NULL;
	struct pid *pid_struct = find_vpid(pid);
	if(pid_struct){
		task = pid_task(pid_struct, PIDTYPE_PID);

		if(task){
			get_task_struct(task);
		} 
	}
	return task;
}

static int page_walker(struct pagewlk_args *pw_args){
	struct pagewlk_offsets pw_offsets;
	
	get_page_table_offsets(&pw_offsets, pw_args->virt_addr);
	struct task_struct *task = get_task_by_pid(pw_args->pid);

	struct mm_struct *mm = task->mm;

	// pgd
	unsigned long val = pgd_val(*(mm->pgd + pw_offsets.pgd_offset));
	pud_t *pud = val & 0x000FFFFFFFFFF000;	

	return 0; 
}

static long pw_ioctl(struct file *file, unsigned int cmd, unsigned long  arg){
	// - case for the cmd, only have one right now
	// - define the struct for args and copy the user passed one into ours
	
	struct pagewlk_args pw_args;

	switch(cmd){
		case PAGE_WALK:
		       if(copy_from_user(&pw_args, (struct pagewlk_args *)arg, sizeof(struct pagewlk_args))){
				return -EFAULT; 
			}
			pr_info("Virt Address: %lu and PID: %d\n", pw_args.virt_addr, pw_args.pid);
			page_walker(&pw_args);
			if(copy_to_user((struct pagewalk_args *)arg, &pw_args, sizeof(struct pagewlk_args))){
				return -EFAULT; 
			}
			break;
		default:
			return -ENOTTY;	
	}

	return 0;
}



static int __init pw_init(void)
{
	pr_info("Page Walker loaded into the kernel.\n");
	
	// allocate major number 
	if((alloc_chrdev_region(&dev, 0, 1, "pw_dev")) < 0){
		printk(KERN_INFO "Cannot allocate major number\n");
		return -1;
	}

	// init cdevv struct
	cdev_init(&pw_cdev, &fops);

	// add char dev to the system
	if((cdev_add(&pw_cdev, dev, 1)  < 0)){
		printk(KERN_INFO "Cannot add the device to the system\n");
		goto r_class;		
	}


	// creating struct class
	if((dev_class = class_create("chr_class")) == NULL){
		printk(KERN_INFO "Cannot create the struct class\n");
		goto r_class;
	}

	// creating the device
	if((device_create(dev_class, NULL, dev, NULL, "chr_dev")) == NULL){
		printk(KERN_INFO "Cannot create the Device\n");
		goto r_device;
	}

	printk(KERN_INFO "Device Driver Insert...Done\n");

	return 0; // Non-zero return means init failed; module won't load

r_device:
	class_destroy(dev_class);

r_class:
	unregister_chrdev_region(dev, 1);
	cdev_del(&pw_cdev);
	return -1;
}

static void __exit pw_exit(void)
{
	device_destroy(dev_class, dev);
	class_destroy(dev_class);
	cdev_del(&pw_cdev);
	unregister_chrdev_region(dev, 1);
	printk(KERN_INFO "Device Driver Remove...Done\n");
}

module_init(pw_init);
module_exit(pw_exit);

