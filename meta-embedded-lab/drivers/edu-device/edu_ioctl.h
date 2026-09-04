#ifndef EDU_IOCTL_H
#define EDU_IOCTL_H

#include <linux/ioctl.h>

#define EDU_IOC_MAGIC 'E'

#define EDU_IOC_RESET _IO(EDU_IOC_MAGIC, 0x01)
#define EDU_IOC_ENABLE_IRQ _IOW(EDU_IOC_MAGIC, 0x02, int)
#define EDU_IOC_SET_MODE _IOW(EDU_IOC_MAGIC, 0x03, int)
#define EDU_IOC_GET_STATUS _IOR(EDU_IOC_MAGIC, 0x04, int)
#define EDU_IOC_READ_FIFO _IOWR(EDU_IOC_MAGIC, 0x05, struct edu_fifo_req)
#define EDU_IOC_WRITE_FIFO _IOW(EDU_IOC_MAGIC, 0x06, int)
#define EDU_IOC_FAULT_INJECT _IOW(EDU_IOC_MAGIC, 0x07, int)

struct edu_fifo_req {
	__u32 len;
	__u32 buffer[64];
};

#endif
