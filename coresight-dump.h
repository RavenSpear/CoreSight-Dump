#include <linux/fs.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/miscdevice.h>
#include <linux/interrupt.h>

#define AXI_LITE_WIDTH      4
#define AXI_LITE_BASE       0xA0000000
#define AXI_LITE_REG_NUM    64
#define PROCEED_DATA_INDEX  32
#define PROCEED_DATA_NUM    27
#define SUCCEED_DATA_INDEX  0
#define SUCCEED_DATA_NUM    32
#define BUG_NUM_INDEX       63
#define FRAME_INDEX         59
#define FRAME_NUM           4

struct coresight_dump_data{
        u32 bug_num;
        u32 frame[FRAME_NUM];
        u32 proceed_data[PROCEED_DATA_NUM];
        u32 succeed_data[SUCCEED_DATA_NUM];
};
static void __iomem *axi_data_ptr[AXI_LITE_REG_NUM]; 
static unsigned int pl_ps_irq;
static int __init coresight_dump_init(void);
static void __exit coresight_dump_exit(void);
static int coresight_dump_probe(struct platform_device *pdev);
static int coresight_dump_remove(struct platform_device *pdev);
static int coresight_dump_open(struct inode *, struct file *);
static ssize_t coresight_dump_read(struct file *, char __user *, size_t, loff_t *);
static void coresight_dump_fetch_data(void);
static int coresight_dump_print(void);
static int coresight_dump_print_raw(void);
static irqreturn_t coresight_dump_irq_handler(int, void *);
