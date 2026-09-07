#include <linux/kernel.h>
#include <linux/platform_device.h>

#include "edu_device.h"

static void edu_mmio_validate(struct edu_device *edu, u32 offset, size_t size)
{
	if (!edu || !edu->regs || !edu->reg_size)
		return;

	if (offset + size > edu->reg_size)
		pr_warn("EDU: MMIO access out of range (offset=%u size=%zu reg_size=%llu)\n",
			offset, size, (unsigned long long)edu->reg_size);
}

int edu_mmio_init(struct edu_device *edu, struct platform_device *pdev)
{
	struct resource *mem;

	if (!edu || !pdev)
		return -EINVAL;

	mem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!mem)
		return -ENODEV;

	if (resource_size(mem) < 0x1000)
		return -EINVAL;

	edu->regs = devm_ioremap_resource(&pdev->dev, mem);
	if (IS_ERR(edu->regs))
		return PTR_ERR(edu->regs);

	edu->reg_size = resource_size(mem);
	return 0;
}

u32 edu_reg_read32(struct edu_device *edu, u32 offset)
{
	if (!edu || !edu->regs)
		return 0;

	edu_mmio_validate(edu, offset, sizeof(u32));
	return readl(edu->regs + offset);
}

void edu_reg_write32(struct edu_device *edu, u32 offset, u32 value)
{
	if (!edu || !edu->regs)
		return;

	edu_mmio_validate(edu, offset, sizeof(u32));
	writel(value, edu->regs + offset);
}

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
