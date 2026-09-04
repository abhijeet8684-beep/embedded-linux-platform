#include <linux/pm.h>

#include "edu_device.h"

int edu_pm_suspend(struct device *dev)
{
	struct edu_device *edu = dev_get_drvdata(dev);
	if (!edu)
		return -ENODEV;
	edu->status |= 0x10;
	return 0;
}

int edu_pm_resume(struct device *dev)
{
	struct edu_device *edu = dev_get_drvdata(dev);
	if (!edu)
		return -ENODEV;
	edu->status &= ~0x10;
	return 0;
}

const struct dev_pm_ops edu_pm_ops = {
	.suspend = edu_pm_suspend,
	.resume = edu_pm_resume,
};
