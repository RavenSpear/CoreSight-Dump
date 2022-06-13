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
        .name = "coresight_dump_dev",
        .fops = &coresight_dump_fops,
};

static const struct of_device_id coresight_dump_match[] = {
	{.compatible = "xlnx,CoreSight-Dump-1.0"},
	{}
};

static struct platform_driver coresight_dump_driver = {
        .probe = coresight_dump_probe,
        .remove = coresight_dump_remove,
        .driver = {
                .owner = THIS_MODULE,
                .name = "coresight_dump",
                .of_match_table	= coresight_dump_match,	
        }
};

static struct coresight_dump_data cs_dp_data;

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
        coresight_dump_fetch_data();
        coresight_dump_print();
        //coresight_dump_print_raw();
        return 0;
}

static void coresight_dump_fetch_data(void){
        int i;

        cs_dp_data.bug_num = readl(axi_data_ptr[BUG_NUM_INDEX]);

        for(i = SUCCEED_DATA_INDEX;i < SUCCEED_DATA_INDEX + SUCCEED_DATA_NUM;i++){
                cs_dp_data.succeed_data[i-SUCCEED_DATA_INDEX] = readl(axi_data_ptr[i]);
        }
        for(i = PROCEED_DATA_INDEX;i < PROCEED_DATA_INDEX + PROCEED_DATA_NUM;i++){
                cs_dp_data.proceed_data[PROCEED_DATA_NUM - i + PROCEED_DATA_INDEX - 1] = readl(axi_data_ptr[i]);
                //printk(KERN_CONT "i: %d, index: %d, proceed data: %x. ",i,PROCEED_DATA_NUM - i + PROCEED_DATA_INDEX - 1,data);
                //cs_dp_data.proceed_data[i-PROCEED_DATA_INDEX] = readl(axi_data_ptr[i]);
        }
        for(i = FRAME_INDEX;i < FRAME_INDEX + FRAME_NUM;i++){
                cs_dp_data.frame[i-FRAME_INDEX] = readl(axi_data_ptr[i]);
        }
}

static int coresight_dump_print(void){
        int i;
        printk("Print CoreSight Raw Data...");
        printk("Bug Type: %x\n",cs_dp_data.bug_num);
        printk(KERN_CONT "Frame: ");
        for(i = 0;i < FRAME_NUM;i++){
                printk(KERN_CONT "%x ",cs_dp_data.frame[i]);
        }
        printk(KERN_CONT "\nDump Data: ");
        for(i = 0;i < PROCEED_DATA_NUM - 1;i++){
                printk(KERN_CONT "%x ",cs_dp_data.proceed_data[i]);
        }
        /*focus the trace_data[31:0] which triggers the dump.*/
        printk(KERN_CONT "[ %x ] ",cs_dp_data.proceed_data[PROCEED_DATA_NUM - 1]);
        for(i = 0;i < SUCCEED_DATA_NUM;i++){
                printk(KERN_CONT "%x ",cs_dp_data.succeed_data[i]);
        }
        printk("Print Done...");
        return 0;
}

static int coresight_dump_print_raw(void){
        int i;
        u32 data;
        printk("Print CoreSight Raw Data undecoded...\n");
        for(i = 0;i < AXI_LITE_REG_NUM;i++){
                data = readl(axi_data_ptr[i]);
                printk(KERN_CONT "%x ",data);
        }
        printk("Print Done...");
        return 0;
}

static irqreturn_t coresight_dump_irq_handler(int irq, void *dev_id){
        printk("irq = %d\n", irq);
        coresight_dump_fetch_data();
        coresight_dump_print();
        //coresight_dump_print_raw();
        return IRQ_HANDLED;
}



module_init(coresight_dump_init);
module_exit(coresight_dump_exit);
MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Fan Wang");
MODULE_DESCRIPTION("AXI-Lite based device for dumping CoreSight raw data.");
MODULE_DEVICE_TABLE(of, coresight_dump_match);