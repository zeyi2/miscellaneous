# Makefile for Brainfuck interpreter

# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -O2

# Executable name
TARGET = bf

# Source files
SRCS = bf.c

# Object files
OBJS = $(SRCS:.c=.o)

# Install directory
PREFIX = /usr/local
BINDIR = $(PREFIX)/bin

# Default target
all: $(TARGET)

# Link the executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS)

# Compile source files to object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Install the executable
install: $(TARGET)
	@echo "Installing $(TARGET) to $(BINDIR)..."
	install -D -m 755 $(TARGET) $(BINDIR)/$(TARGET)

# Uninstall the executable
uninstall:
	@echo "Uninstalling $(TARGET) from $(BINDIR)..."
	rm -f $(BINDIR)/$(TARGET)

# Clean object files and executable
clean:
	rm -f $(OBJS) $(TARGET)

# Print help message
help:
	@echo "Usage:"
	@echo "  make              Build the bf executable"
	@echo "  make clean        Remove object files and executable"
	@echo "  make install      Install the bf executable to $(BINDIR)"
	@echo "  make uninstall    Uninstall the bf executable from $(BINDIR)"
	@echo "  make help         Display this help message"

.PHONY: all clean install uninstall help
