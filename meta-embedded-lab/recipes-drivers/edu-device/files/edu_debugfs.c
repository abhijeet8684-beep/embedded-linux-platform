#include <linux/debugfs.h>
#include <linux/fs.h>

#include "edu_device.h"

static int edu_debug_open(struct inode *inode, struct file *file)
{
	file->private_data = inode->i_private;
	return 0;
}

static const struct file_operations edu_debug_fops = {
	.owner = THIS_MODULE,
	.open = edu_debug_open,
};

int edu_debugfs_create(struct edu_device *edu)
{
	if (!edu)
		return -EINVAL;

	edu->debug_root = debugfs_create_dir("edu-device", NULL);
	if (IS_ERR(edu->debug_root))
		return PTR_ERR(edu->debug_root);

	debugfs_create_u32("irq_count", 0444, edu->debug_root, &edu->irq_status);
	debugfs_create_bool("fault_inject", 0644, edu->debug_root, &edu->fault_active);
	debugfs_create_u32("fifo_level", 0444, edu->debug_root, &edu->fifo_level);
	return 0;
}

void edu_debugfs_remove(struct edu_device *edu)
{
	if (!edu)
		return;
	if (edu->debug_root)
		debugfs_remove_recursive(edu->debug_root);
	edu->debug_root = NULL;
	edu->irq_count = NULL;
	edu->fault_inject = NULL;
	edu->fifo_level_entry = NULL;
}
