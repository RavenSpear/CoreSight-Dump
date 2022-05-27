#include <linux/init.h>
#include <linux/module.h>

MODULE_LICENSE("Dual BSD/GPL")
MODULE_AUTHOR("Fan Wang")
MODULE_DISCRIPTION("AXI-Lite based device for dumping CoreSight raw data.")

int axi_lite_init(void){
        printk("");
        return 0;
}

void axi_lite_exit(void){
        printk("CoreSight Dump exit.");
}
module_init()
module_exit()