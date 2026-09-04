#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/poll.h>
#include <linux/slab.h>
#include <linux/uaccess.h>

#include "edu_device.h"
#include "edu_ioctl.h"

static int edu_open(struct inode *inode, struct file *file)
{
	struct edu_device *edu = g_dev;
	if (!edu)
		return -ENODEV;
	file->private_data = edu;
	return 0;
}

static int edu_release(struct inode *inode, struct file *file)
{
	file->private_data = NULL;
	return 0;
}

static ssize_t edu_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
	struct edu_device *edu = file->private_data;
	u32 value;
	int ret;

	if (!edu)
		return -ENODEV;

	if (mutex_lock_interruptible(&edu->mutex))
		return -EINTR;

	if (edu->count == 0) {
		mutex_unlock(&edu->mutex);
		return 0;
	}

	value = edu_fifo_pop(edu);
	ret = copy_to_user(buf, &value, sizeof(value));
	if (ret)
		ret = -EFAULT;
	else
		ret = sizeof(value);

	mutex_unlock(&edu->mutex);
	return ret;
}

static ssize_t edu_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
	struct edu_device *edu = file->private_data;
	u32 value;
	int ret;

	if (!edu)
		return -ENODEV;

	if (count < sizeof(value))
		return -EINVAL;

	if (copy_from_user(&value, buf, sizeof(value)))
		return -EFAULT;

	if (mutex_lock_interruptible(&edu->mutex))
		return -EINTR;

	ret = edu_fifo_push(edu, value);
	mutex_unlock(&edu->mutex);
	return ret < 0 ? ret : sizeof(value);
}

static __poll_t edu_poll(struct file *file, poll_table *wait)
{
	struct edu_device *edu = file->private_data;
	__poll_t mask = 0;

	if (!edu)
		return POLLERR;

	poll_wait(file, &edu->wq, wait);
	if (edu->count > 0)
		mask |= POLLIN | POLLRDNORM;
	if (edu->irq_enabled)
		mask |= POLLPRI;

	return mask;
}

static long edu_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	struct edu_device *edu = file->private_data;
	int enable;
	int status;

	if (!edu)
		return -ENODEV;

	switch (cmd) {
	case EDU_IOC_RESET:
		edu_fifo_reset(edu);
		edu->status = 0x1;
		edu->error_status = 0;
		return 0;
	case EDU_IOC_ENABLE_IRQ:
		if (copy_from_user(&enable, (void __user *)arg, sizeof(enable)))
			return -EFAULT;
		edu->irq_enabled = !!enable;
		return 0;
	case EDU_IOC_SET_MODE:
		if (copy_from_user(&enable, (void __user *)arg, sizeof(enable)))
			return -EFAULT;
		edu->control = (u32)enable;
		return 0;
	case EDU_IOC_GET_STATUS:
		status = edu->status | (edu->error_status << 16);
		if (copy_to_user((void __user *)arg, &status, sizeof(status)))
			return -EFAULT;
		return 0;
	default:
		return -ENOTTY;
	}
}

const struct file_operations edu_fops = {
	.owner = THIS_MODULE,
	.open = edu_open,
	.release = edu_release,
	.read = edu_read,
	.write = edu_write,
	.poll = edu_poll,
	.unlocked_ioctl = edu_ioctl,
};
