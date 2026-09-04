#include <linux/device.h>
#include <linux/sysfs.h>

#include "edu_device.h"

static ssize_t version_show(struct device *dev,
			   struct device_attribute *attr, char *buf)
{
	struct edu_device *edu = dev_get_drvdata(dev);
	return sysfs_emit(buf, "%u\n", edu ? edu->version : 0);
}

static ssize_t status_show(struct device *dev,
			  struct device_attribute *attr, char *buf)
{
	struct edu_device *edu = dev_get_drvdata(dev);
	return sysfs_emit(buf, "%u\n", edu ? edu->status : 0);
}

static ssize_t fifo_level_show(struct device *dev,
				 struct device_attribute *attr, char *buf)
{
	struct edu_device *edu = dev_get_drvdata(dev);
	return sysfs_emit(buf, "%u\n", edu ? edu->fifo_level : 0);
}

static ssize_t irq_enable_show(struct device *dev,
				 struct device_attribute *attr, char *buf)
{
	struct edu_device *edu = dev_get_drvdata(dev);
	return sysfs_emit(buf, "%d\n", edu && edu->irq_enabled ? 1 : 0);
}

static ssize_t error_status_show(struct device *dev,
				 struct device_attribute *attr, char *buf)
{
	struct edu_device *edu = dev_get_drvdata(dev);
	return sysfs_emit(buf, "%u\n", edu ? edu->error_status : 0);
}

static DEVICE_ATTR_RO(version);
static DEVICE_ATTR_RO(status);
static DEVICE_ATTR_RO(fifo_level);
static DEVICE_ATTR_RO(irq_enable);
static DEVICE_ATTR_RO(error_status);

int edu_sysfs_create(struct edu_device *edu)
{
	int ret;

	if (!edu || !edu->dev)
		return -EINVAL;

	ret = device_create_file(edu->dev, &dev_attr_version);
	if (ret)
		return ret;
	ret = device_create_file(edu->dev, &dev_attr_status);
	if (ret)
		return ret;
	ret = device_create_file(edu->dev, &dev_attr_fifo_level);
	if (ret)
		return ret;
	ret = device_create_file(edu->dev, &dev_attr_irq_enable);
	if (ret)
		return ret;
	ret = device_create_file(edu->dev, &dev_attr_error_status);
	if (ret)
		return ret;
	return 0;
}

void edu_sysfs_remove(struct edu_device *edu)
{
	if (!edu || !edu->dev)
		return;
	device_remove_file(edu->dev, &dev_attr_version);
	device_remove_file(edu->dev, &dev_attr_status);
	device_remove_file(edu->dev, &dev_attr_fifo_level);
	device_remove_file(edu->dev, &dev_attr_irq_enable);
	device_remove_file(edu->dev, &dev_attr_error_status);
}
