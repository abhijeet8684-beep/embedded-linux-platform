CC ?= gcc
CFLAGS ?= -O2 -Wall -Wextra -std=c11

TARGET = edu-cli

all: $(TARGET)

$(TARGET): edu-cli.c
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(TARGET)
