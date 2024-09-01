# Compiler
CC = gcc

# Compiler flags


# Source files
SRCS = myshell.c log.c dir.c bg_handler.c info.c input.c proclore.c reveal.c seek.c

# Output executable (default name 'a.out')
TARGET = a.out

# Default target
all: $(TARGET)

# Link all object files into the final executable
$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS)

# Clean up build files
clean:
	rm -f $(TARGET)

# Rebuild the project
rebuild: clean all
