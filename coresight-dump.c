#include <linux/module.h>
#include <linux/init.h>
#include <asm/io.h>
#include <linux/of.h>

#include "coresight-dump.h"

static const struct file_operations coresight_dump_fops = {
        .owner = THIS_MODULE,
        .open = coresight_dump_open,
        .read = coresight_dump_read
};
static struct miscdevice coresight_dump_dev = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = "coresight_dump_dev",  /* 指明/dev/目录下的设备节点名 */
    .fops = &coresight_dump_fops,
};

static const struct of_device_id coresight_dump_match[] = {
	{.compatible = "xlnx,DMA-AXI-LiTE-1.0"},
	{}
};
MODULE_DEVICE_TABLE(of, coresight_dump_match);
static struct platform_driver coresight_dump_driver = {
        .probe = coresight_dump_probe,
        .remove = coresight_dump_remove,
        .driver = {
                .owner = THIS_MODULE,
                .name = "coresight_dump",
                .of_match_table	= coresight_dump_match,	
        }
};


static void __iomem *axi_data_ptr[AXI_LITE_REG_NUM]; 

static int __init coresight_dump_init(void){
        int i;
        for(i = 0;i<AXI_LITE_REG_NUM;i++){
                axi_data_ptr[i] = ioremap(AXI_LITE_BASE + AXI_LITE_WIDTH*i, AXI_LITE_WIDTH);
        }
	if (platform_driver_register(&coresight_dump_driver)) {
		pr_info("Error registering platform driver\n");
		return -1;
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
        platform_driver_unregister(&coresight_dump_driver);
        misc_deregister(&coresight_dump_dev); 
        printk("CoreSight Dump exit.");
}

static int coresight_dump_probe(struct platform_device *pdev){
        int err;
        pl_ps_irq = platform_get_irq(pdev,0);
        if (pl_ps_irq <= 0)
		return -ENXIO;
        err = request_irq(pl_ps_irq,
			coresight_dump_irq_handler,
			IRQF_TRIGGER_RISING | IRQF_ONESHOT,
			coresight_dump_dev.name, NULL);				   
	if (err) {
		printk(KERN_ALERT "irq_probe irq error=%d\n", err);
	}
	else
	{
		printk("irq = %d gained.\n", pl_ps_irq);
	}
        return 0;
}

static int coresight_dump_remove(struct platform_device *pdev){
        free_irq(pl_ps_irq, NULL);
	printk("irq = %d freed.\n", pl_ps_irq);
 
	return 0;
}

static int coresight_dump_open(struct inode *inode, struct file *filp)  
{
        printk("CoreSight Dump dev is open!...\n");
        return 0;
}

static ssize_t coresight_dump_read(struct file *file, char __user *buff, size_t count, loff_t *ppos){
        coresight_dump_print();
        return 0;
}

static int coresight_dump_print(void){
        int i;
        u32 data;
        printk("Reading CoreSight Raw Data...\n\r");
        for(i = 0;i<AXI_LITE_REG_NUM;i++){
                data = readl(axi_data_ptr[i]);
                printk(KERN_CONT "%x ",data);
        }
        printk("Reading Ended...");
        return 0;
}

static irqreturn_t coresight_dump_irq_handler(int irq, void *dev_id){
        printk("irq = %d\n", irq);
        coresight_dump_print();
        return IRQ_HANDLED;
}



module_init(coresight_dump_init);
module_exit(coresight_dump_exit);
MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Fan Wang");
MODULE_DESCRIPTION("AXI-Lite based device for dumping CoreSight raw data.");