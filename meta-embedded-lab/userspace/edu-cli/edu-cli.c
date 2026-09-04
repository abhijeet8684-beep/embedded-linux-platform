#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#define EDU_IOC_MAGIC 'E'
#define EDU_IOC_RESET _IO(EDU_IOC_MAGIC, 0x01)
#define EDU_IOC_ENABLE_IRQ _IOW(EDU_IOC_MAGIC, 0x02, int)
#define EDU_IOC_SET_MODE _IOW(EDU_IOC_MAGIC, 0x03, int)
#define EDU_IOC_GET_STATUS _IOR(EDU_IOC_MAGIC, 0x04, int)
#define EDU_IOC_FAULT_INJECT _IOW(EDU_IOC_MAGIC, 0x07, int)

static void print_usage(const char *prog)
{
	fprintf(stderr,
		"Usage: %s [reset|status|write <value>|read|fault <code>|help]\n",
		prog);
}

int main(int argc, char **argv)
{
	const char *path = "/dev/edu0";
	int fd = -1;
	int value = 0;
	int status = 0;

	if (argc < 2) {
		print_usage(argv[0]);
		return 1;
	}

	if (strcmp(argv[1], "help") == 0 || strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0) {
		print_usage(argv[0]);
		return 0;
	}

	fd = open(path, O_RDWR);
	if (fd < 0) {
		fprintf(stderr, "Failed to open %s: %s\n", path, strerror(errno));
		return 2;
	}

	if (strcmp(argv[1], "reset") == 0) {
		if (ioctl(fd, EDU_IOC_RESET, 0) != 0) {
			perror("ioctl(EDU_IOC_RESET)");
			close(fd);
			return 3;
		}
		printf("reset ok\n");
	} else if (strcmp(argv[1], "status") == 0) {
		if (ioctl(fd, EDU_IOC_GET_STATUS, &status) != 0) {
			perror("ioctl(EDU_IOC_GET_STATUS)");
			close(fd);
			return 4;
		}
		printf("status=0x%x\n", status);
	} else if (strcmp(argv[1], "write") == 0) {
		if (argc != 3) {
			print_usage(argv[0]);
			close(fd);
			return 1;
		}
		value = strtol(argv[2], NULL, 0);
		if (write(fd, &value, sizeof(value)) != sizeof(value)) {
			perror("write");
			close(fd);
			return 5;
		}
		printf("write ok: %d\n", value);
	} else if (strcmp(argv[1], "read") == 0) {
		if (read(fd, &value, sizeof(value)) != sizeof(value)) {
			perror("read");
			close(fd);
			return 6;
		}
		printf("read=%d\n", value);
	} else if (strcmp(argv[1], "fault") == 0) {
		if (argc != 3) {
			print_usage(argv[0]);
			close(fd);
			return 1;
		}
		value = strtol(argv[2], NULL, 0);
		if (ioctl(fd, EDU_IOC_FAULT_INJECT, &value) != 0) {
			perror("ioctl(EDU_IOC_FAULT_INJECT)");
			close(fd);
			return 7;
		}
		printf("fault injected: %d\n", value);
	} else if (strcmp(argv[1], "help") == 0 || strcmp(argv[1], "--help") == 0 || strcmp(argv[1], "-h") == 0) {
		print_usage(argv[0]);
	} else {
		print_usage(argv[0]);
		close(fd);
		return 1;
	}

	close(fd);
	return 0;
}
