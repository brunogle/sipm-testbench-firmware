# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra

# Directories
OBJDIR = obj
BINDIR = bin

# Shared source files
COMMON_SRCS = dac.c fpga.c vdac_cal.c ads1247.c gpio.c

# Executables
TARGETS = $(BINDIR)/setvsipm $(BINDIR)/sampleadc $(BINDIR)/readsense $(BINDIR)/ivcurve

# Sources for each target
SETVSIPM_SRCS = $(COMMON_SRCS) setvsipm.c
SAMPLEADC_SRCS = $(COMMON_SRCS) sampleadc.c
READSENSE_SRCS = $(COMMON_SRCS) readsense.c
IVCURVE_SRCS = $(COMMON_SRCS) ivcurve.c

# Object files
SETVSIPM_OBJS = $(addprefix $(OBJDIR)/,$(SETVSIPM_SRCS:.c=.o))
SAMPLEADC_OBJS = $(addprefix $(OBJDIR)/,$(SAMPLEADC_SRCS:.c=.o))
READSENSE_OBJS = $(addprefix $(OBJDIR)/,$(READSENSE_SRCS:.c=.o))
IVCURVE_OBJS = $(addprefix $(OBJDIR)/,$(IVCURVE_SRCS:.c=.o))

# Header dependencies
DEPS = addr.h dac.h fpga.h vdac_cal.h ads1247.h gpio.h

# Default rule
all: $(BINDIR) $(OBJDIR) $(TARGETS)

# Linking rules
$(BINDIR)/setvsipm: $(SETVSIPM_OBJS)
	$(CC) $(CFLAGS) -o $@ $^

$(BINDIR)/sampleadc: $(SAMPLEADC_OBJS)
	$(CC) $(CFLAGS) -o $@ $^

$(BINDIR)/readsense: $(READSENSE_OBJS)
	$(CC) $(CFLAGS) -o $@ $^

$(BINDIR)/ivcurve: $(IVCURVE_OBJS)
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

# Optional rebuild
rebuild: clean all
