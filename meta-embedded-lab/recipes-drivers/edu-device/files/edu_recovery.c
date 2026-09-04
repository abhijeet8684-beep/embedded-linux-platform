#include <linux/kernel.h>

#include "edu_device.h"

void edu_fault_inject(struct edu_device *edu, int fault)
{
	if (!edu)
		return;
	edu->fault_active = !!fault;
	edu->error_status = fault;
	if (fault)
		edu->status |= 0x8;
}

void edu_recover(struct edu_device *edu)
{
	if (!edu)
		return;
	edu->fault_active = false;
	edu->error_status = 0;
	edu->status &= ~0x8;
	edu_fifo_reset(edu);
}
