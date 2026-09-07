#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/platform_device.h>

#include "edu_device.h"

static irqreturn_t edu_irq_handler(int irq, void *dev_id)
{
	struct edu_device *edu = dev_id;

	if (!edu)
		return IRQ_NONE;

	if (!edu->irq_enabled)
		return IRQ_HANDLED;

	edu->irq_status = 1;
	edu->status |= 0x2;
	wake_up_interruptible(&edu->wq);
	schedule_work(&edu->irq_work);
	return IRQ_HANDLED;
}

void edu_irq_work(struct work_struct *work)
{
	struct edu_device *edu = container_of(work, struct edu_device, irq_work);

	if (!edu)
		return;

	edu->status |= 0x4;
	edu->fifo_level = edu_fifo_level(edu);
}

int edu_irq_request(struct edu_device *edu, struct platform_device *pdev)
{
	int irq;

	if (!edu || !pdev)
		return -EINVAL;

	irq = platform_get_irq(pdev, 0);
	if (irq < 0)
		return irq;

	if (devm_request_irq(&pdev->dev, irq, edu_irq_handler, IRQF_SHARED,
			     EDU_NAME, edu))
		return -EBUSY;

	return 0;
}

void edu_irq_free(struct edu_device *edu)
{
	if (!edu || !edu->pdev)
		return;

	/* The IRQ is registered with devm_request_irq() and is released by the
	 * device framework when the platform device is removed.
	 */
}
