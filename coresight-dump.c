#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/miscdevice.h>
#include <asm/io.h>

#define AXI_LITE_WIDTH      4
#define AXI_LITE_BASE       0xA0000000
#define AXI_LITE_REG_NUM    16

static void __iomem *axi_data_ptr[AXI_LITE_REG_NUM]; 

static int coresight_dump_open(struct inode *inode, struct file *filp)  
{
    printk("CoreSight Dump dev is open!...\n");
    return 0;
}



static int coresight_dump_print(void){
        int i;
        u32 data;
        printk("Reading CoreSight Raw Data...");
        for(i = 0;i<AXI_LITE_REG_NUM;i++){
                data = readl(axi_data_ptr[i]);
                printk(KERN_CONT "%x ",data);
        }
        printk("Reading Ended...");
        return 0;
}

static ssize_t coresight_dump_read(struct file *file, char __user *buff, size_t count, loff_t *ppos){
        coresight_dump_print();
        return 0;
}

static const struct file_operations coresight_dump_fops = {
        .owner = THIS_MODULE,
        .open = coresight_dump_open,
        .read = coresight_dump_read
};

static struct miscdevice coresight_dump_dev = {
        .minor = MISC_DYNAMIC_MINOR,
        .name = "coresight_dump_dev",
        .fops = &coresight_dump_fops,
};

static int __init coresight_dump_init(void){
        int i;
        for(i = 0;i<AXI_LITE_REG_NUM;i++){
                axi_data_ptr[i] = ioremap(AXI_LITE_BASE + AXI_LITE_WIDTH*i, AXI_LITE_WIDTH);
        }
        if (misc_register(&coresight_dump_dev)) {
                printk("Coresight Dump device register failed...\n");
                return -1;
        }
        printk("CoreSight Dump init.");
        return 0;
}

static void __exit coresight_dump_exit(void){
        int i;
        for(i = 0;i<AXI_LITE_REG_NUM;i++){
                iounmap(axi_data_ptr[i]);
        }
        misc_deregister(&coresight_dump_dev); 
        printk("CoreSight Dump exit.");
}


module_init(coresight_dump_init);
module_exit(coresight_dump_exit);
MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Fan Wang");
MODULE_DESCRIPTION("AXI-Lite based device for dumping CoreSight raw data.");