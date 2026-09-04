#include <linux/kernel.h>

#include "edu_device.h"

void edu_hw_reset(struct edu_device *edu)
{
	if (!edu)
		return;
	edu->control = 0;
	edu->status = 0x1;
	edu->irq_status = 0;
	edu->error_status = 0;
	edu_fifo_reset(edu);
}

u32 edu_hw_read_version(struct edu_device *edu)
{
	return edu ? edu->version : 0;
}

void edu_hw_set_fault(struct edu_device *edu, int fault)
{
	if (!edu)
		return;
	edu->fault_active = !!fault;
	edu->error_status = fault;
}
