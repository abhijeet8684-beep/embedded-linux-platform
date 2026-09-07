#ifndef EDU_DEVICE_H
#define EDU_DEVICE_H

#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/io.h>
#include <linux/pm.h>
#include <linux/mutex.h>
#include <linux/platform_device.h>
#include <linux/spinlock.h>
#include <linux/wait.h>
#include <linux/workqueue.h>

#define EDU_NAME "edu-device"
#define EDU_MAX_BUFSZ 256
#define EDU_IRQ_LINE 12

struct edu_device {
	struct device *dev;
	struct platform_device *pdev;
	struct cdev cdev;
	dev_t devno;
	struct class *class;
	spinlock_t lock;
	struct mutex mutex;
	wait_queue_head_t wq;
	struct work_struct irq_work;
	struct dentry *debug_root;
	struct dentry *irq_count;
	struct dentry *fault_inject;
	struct dentry *fifo_level_entry;
	void __iomem *regs;
	resource_size_t reg_size;
	bool irq_enabled;
	bool fault_active;
	u32 version;
	u32 control;
	u32 status;
	u32 irq_status;
	u32 error_status;
	u32 fifo_level;
	u32 fifo_data[EDU_MAX_BUFSZ];
	u32 head;
	u32 tail;
	u32 count;
	u32 length;
};

extern struct edu_device *g_dev;
extern const struct file_operations edu_fops;
extern const struct dev_pm_ops edu_pm_ops;

int edu_fifo_init(struct edu_device *edu);
void edu_fifo_reset(struct edu_device *edu);
int edu_fifo_push(struct edu_device *edu, u32 value);
u32 edu_fifo_pop(struct edu_device *edu);
int edu_fifo_level(struct edu_device *edu);

int edu_mmio_init(struct edu_device *edu, struct platform_device *pdev);
u32 edu_reg_read32(struct edu_device *edu, u32 offset);
void edu_reg_write32(struct edu_device *edu, u32 offset, u32 value);

void edu_irq_work(struct work_struct *work);
int edu_irq_request(struct edu_device *edu, struct platform_device *pdev);
void edu_irq_free(struct edu_device *edu);

int edu_sysfs_create(struct edu_device *edu);
void edu_sysfs_remove(struct edu_device *edu);
int edu_debugfs_create(struct edu_device *edu);
void edu_debugfs_remove(struct edu_device *edu);

void edu_fault_inject(struct edu_device *edu, int fault);
void edu_recover(struct edu_device *edu);
int edu_pm_suspend(struct device *dev);
int edu_pm_resume(struct device *dev);

#endif
