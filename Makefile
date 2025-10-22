# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra

# Directories
OBJDIR = obj
BINDIR = bin

# Target executable
TARGET = $(BINDIR)/setvsipm

# Source and object files
SRCS = dac.c fpga.c vdac_cal.c setvsipm.c
OBJS = $(addprefix $(OBJDIR)/,$(SRCS:.c=.o))

# Header dependencies
DEPS = addr.h dac.h fpga.h vdac_cal.h

# Default rule
all: $(BINDIR) $(OBJDIR) $(TARGET)

# Linking
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# Compilation rule
$(OBJDIR)/%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c $< -o $@

# Create directories if they don't exist
$(BINDIR) $(OBJDIR):
	mkdir -p $@

# Clean rule
clean:
	rm -rf $(OBJDIR) $(BINDIR)

# Optional: reb
