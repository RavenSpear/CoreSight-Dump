#include <linux/fs.h>
#include <linux/device.h>
#include <linux/miscdevice.h>

#define AXI_LITE_WIDTH      4
#define AXI_LITE_BASE       0xA0000000
#define AXI_LITE_REG_NUM    16

static int __init coresight_dump_init(void);
static void __exit coresight_dump_exit(void);
static int coresight_dump_open(struct inode *, struct file *);
static ssize_t coresight_dump_read(struct file *, char __user *, size_t, loff_t *);
static int coresight_dump_print(void);
// static irqreturn_t coresight_dump_irq_handler();