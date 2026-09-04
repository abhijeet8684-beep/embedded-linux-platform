#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/spinlock.h>

#include "edu_device.h"
#include "edu_ioctl.h"

int edu_fifo_init(struct edu_device *edu)
{
	if (!edu)
		return -EINVAL;
	edu->head = 0;
	edu->tail = 0;
	edu->count = 0;
	return 0;
}

void edu_fifo_reset(struct edu_device *edu)
{
	unsigned long flags;

	if (!edu)
		return;
	spin_lock_irqsave(&edu->lock, flags);
	edu->head = 0;
	edu->tail = 0;
	edu->count = 0;
	spin_unlock_irqrestore(&edu->lock, flags);
}

int edu_fifo_push(struct edu_device *edu, u32 value)
{
	unsigned long flags;

	if (!edu)
		return -EINVAL;
	if (edu->count >= edu->length)
		return -ENOSPC;

	spin_lock_irqsave(&edu->lock, flags);
	edu->fifo_data[edu->head] = value;
	edu->head = (edu->head + 1) % edu->length;
	edu->count++;
	edu->fifo_level = edu->count;
	spin_unlock_irqrestore(&edu->lock, flags);
	wake_up_interruptible(&edu->wq);
	return 0;
}

u32 edu_fifo_pop(struct edu_device *edu)
{
	unsigned long flags;
	u32 value = 0;

	if (!edu)
		return value;
	spin_lock_irqsave(&edu->lock, flags);
	if (edu->count > 0) {
		value = edu->fifo_data[edu->tail];
		edu->tail = (edu->tail + 1) % edu->length;
		edu->count--;
		edu->fifo_level = edu->count;
	}
	spin_unlock_irqrestore(&edu->lock, flags);
	return value;
}

int edu_fifo_level(struct edu_device *edu)
{
	return edu ? edu->fifo_level : 0;
}
