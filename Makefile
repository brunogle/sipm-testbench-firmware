# # Compiler and flags
# CC = gcc
# CFLAGS = -Wall -Wextra

# # Directories
# OBJDIR = obj
# BINDIR = bin

# # Shared source files
# COMMON_SRCS = src/dac8562.c src/fpga.c src/ads1247.c src/bias.c

# # Executables
# TARGETS = $(BINDIR)/setvsipm #$(BINDIR)/sampleadc $(BINDIR)/readsense $(BINDIR)/ivcurve

# # Sources for each target
# SETVSIPM_SRCS = $(COMMON_SRCS) setvsipm/setvsipm.c
# SAMPLEADC_SRCS = $(COMMON_SRCS) sampleadc.c
# READSENSE_SRCS = $(COMMON_SRCS) readsense.c
# IVCURVE_SRCS = $(COMMON_SRCS) ivcurve.c

# # Object files
# SETVSIPM_OBJS = $(addprefix $(OBJDIR)/,$(SETVSIPM_SRCS:.c=.o))
# SAMPLEADC_OBJS = $(addprefix $(OBJDIR)/,$(SAMPLEADC_SRCS:.c=.o))
# READSENSE_OBJS = $(addprefix $(OBJDIR)/,$(READSENSE_SRCS:.c=.o))
# IVCURVE_OBJS = $(addprefix $(OBJDIR)/,$(IVCURVE_SRCS:.c=.o))

# # Header dependencies
# DEPS = addr.h dac8562.h fpga.h ads1247.h bias.h

# # Default rule
# all: $(BINDIR) $(OBJDIR) $(TARGETS)

# # Linking rules
# $(BINDIR)/setvsipm: $(SETVSIPM_OBJS)
# 	$(CC) $(CFLAGS) -o $@ $^

# $(BINDIR)/sampleadc: $(SAMPLEADC_OBJS)
# 	$(CC) $(CFLAGS) -o $@ $^

# $(BINDIR)/readsense: $(READSENSE_OBJS)
# 	$(CC) $(CFLAGS) -o $@ $^

# $(BINDIR)/ivcurve: $(IVCURVE_OBJS)
# 	$(CC) $(CFLAGS) -o $@ $^


# # Compilation rule
# $(OBJDIR)/%.o: %.c $(DEPS)
# 	$(CC) $(CFLAGS) -c $< -o $@

# # Create directories if they don't exist
# $(BINDIR) $(OBJDIR):
# 	mkdir -p $@

# # Clean rule
# clean:
# 	rm -rf $(OBJDIR) $(BINDIR)

# # Optional rebuild
# rebuild: clean all
# ==== Compiler settings ====
CC      := gcc
CFLAGS  := -Wall -Wextra -Iinc
LDFLAGS :=

# ==== Directories ====
SRC_DIR := src
APP_DIR := apps
INC_DIR := inc
OBJ_DIR := obj
BIN_DIR := bin

# ==== Top-level apps (contain main()) ====
TOP_APPS := setvsipm ivcurve readsense sampleadc gethist      # <-- only those with main()

# ==== Sources ====
COMMON_SRC := $(wildcard $(SRC_DIR)/*.c)
COMMON_OBJ := $(COMMON_SRC:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

APP_SRC := $(wildcard $(APP_DIR)/*.c)
APP_OBJ := $(APP_SRC:$(APP_DIR)/%.c=$(OBJ_DIR)/%.o)

# Helper objs = all app objs except top-level ones
HELPER_OBJ := $(filter-out $(TOP_APPS:%=$(OBJ_DIR)/%.o), $(APP_OBJ))

# ==== Final executables ====
APPS := $(TOP_APPS:%=$(BIN_DIR)/%)

# ==== Default target ====
all: $(APPS)

# ==== Link each app ====
$(BIN_DIR)/%: $(COMMON_OBJ) $(HELPER_OBJ) $(OBJ_DIR)/%.o | $(BIN_DIR)
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)
	@echo "Built $@"

# ==== Compile rules ====
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR) $(DEPS)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: $(APP_DIR)/%.c | $(OBJ_DIR) $(DEPS)
	$(CC) $(CFLAGS) -c $< -o $@

# ==== Ensure output dirs exist ====
$(OBJ_DIR) $(BIN_DIR):
	mkdir -p $@

# ==== Cleanup ====
clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

.PHONY: all clean

# ==== Keep object files between builds ====
.SECONDARY:
