#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/io.h>
#include <linux/module.h>
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/types.h>
#include <linux/uaccess.h>

#include "edu_device.h"
#include "edu_ioctl.h"

struct edu_device *g_dev;

static const struct of_device_id edu_of_match[] = {
	{ .compatible = "edu,device-v1" },
	{ }
};
MODULE_DEVICE_TABLE(of, edu_of_match);

static int edu_register_chrdev(struct edu_device *edu)
{
	int ret;

	ret = alloc_chrdev_region(&edu->devno, 0, 1, EDU_NAME);
	if (ret)
		return ret;

	cdev_init(&edu->cdev, &edu_fops);
	edu->cdev.owner = THIS_MODULE;
	ret = cdev_add(&edu->cdev, edu->devno, 1);
	if (ret) {
		unregister_chrdev_region(edu->devno, 1);
		return ret;
	}

	edu->class = class_create(EDU_NAME);
	if (IS_ERR(edu->class)) {
		ret = PTR_ERR(edu->class);
		cdev_del(&edu->cdev);
		unregister_chrdev_region(edu->devno, 1);
		return ret;
	}

	edu->dev = device_create(edu->class, NULL, edu->devno, NULL, "edu0");
	if (IS_ERR(edu->dev)) {
		ret = PTR_ERR(edu->dev);
		class_destroy(edu->class);
		cdev_del(&edu->cdev);
		unregister_chrdev_region(edu->devno, 1);
		return ret;
	}

	return 0;
}

static void edu_unregister_chrdev(struct edu_device *edu)
{
	if (edu->dev)
		device_destroy(edu->class, edu->devno);
	if (edu->class)
		class_destroy(edu->class);
	cdev_del(&edu->cdev);
	unregister_chrdev_region(edu->devno, 1);
}

static int edu_probe(struct platform_device *pdev)
{
	struct edu_device *edu;
	int ret;

	edu = devm_kzalloc(&pdev->dev, sizeof(*edu), GFP_KERNEL);
	if (!edu)
		return -ENOMEM;

	platform_set_drvdata(pdev, edu);
	g_dev = edu;
	edu->pdev = pdev;
	edu->dev = &pdev->dev;
	edu->regs = NULL;
	edu->reg_size = 0;
	edu->version = 1;
	edu->control = 0;
	edu->status = 0x1;
	edu->irq_status = 0;
	edu->error_status = 0;
	edu->irq_enabled = true;
	edu->length = EDU_MAX_BUFSZ;
	spin_lock_init(&edu->lock);
	mutex_init(&edu->mutex);
	init_waitqueue_head(&edu->wq);
	INIT_WORK(&edu->irq_work, edu_irq_work);

	ret = edu_mmio_init(edu, pdev);
	if (ret) {
		dev_err(edu->dev, "EDU MMIO init failed: %d\n", ret);
		return ret;
	}

	ret = edu_register_chrdev(edu);
	if (ret)
		return ret;

	ret = edu_fifo_init(edu);
	if (ret) {
		edu_unregister_chrdev(edu);
		return ret;
	}

	ret = edu_sysfs_create(edu);
	if (ret) {
		edu_unregister_chrdev(edu);
		return ret;
	}

	ret = edu_debugfs_create(edu);
	if (ret) {
		edu_sysfs_remove(edu);
		edu_unregister_chrdev(edu);
		return ret;
	}

	ret = edu_irq_request(edu, pdev);
	if (ret) {
		edu_debugfs_remove(edu);
		edu_sysfs_remove(edu);
		edu_unregister_chrdev(edu);
		return ret;
	}

	dev_info(edu->dev, "EDU virtual device registered\n");
	return 0;
}

static int edu_remove(struct platform_device *pdev)
{
	struct edu_device *edu = platform_get_drvdata(pdev);

	edu_irq_free(edu);
	edu_debugfs_remove(edu);
	edu_sysfs_remove(edu);
	edu_unregister_chrdev(edu);
	cancel_work_sync(&edu->irq_work);
	return 0;
}

static struct platform_driver edu_driver = {
	.probe = edu_probe,
	.remove = edu_remove,
	.driver = {
		.name = EDU_NAME,
		.of_match_table = edu_of_match,
		.pm = &edu_pm_ops,
	},
};

module_platform_driver(edu_driver);

MODULE_AUTHOR("Abhijit Bhosale");
MODULE_DESCRIPTION("Virtual EDU device driver with platform/char interfaces");
MODULE_LICENSE("GPL");
MODULE_ALIAS("platform:edu-device");
