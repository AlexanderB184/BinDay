# Define the compiler and flags
CC = gcc
CFLAGS = -O3

# Define the source directories
SRC_DIR = src
TEST_DIR = test

# Define the sources
SOURCES = $(SRC_DIR)/gc.c $(SRC_DIR)/mmap.c $(SRC_DIR)/root_list.c

# Define the default target
all: 

# Target to compile and run the specified test
run: $(TEST_DIR)/$(TEST).c $(SOURCES)
	$(CC) $(CFLAGS) $(TEST_DIR)/$(TEST).c $(SOURCES) -o $(TEST_DIR)/$(TEST).bin
	./$(TEST_DIR)/$(TEST).bin

# Clean up the compiled binaries
clean:
	rm -f $(TEST_DIR)/*.bin

# Help message
help:
	@echo "Usage:"
	@echo "  make run TEST=<testname>  Compile and run the specified test."
	@echo "  make clean                Clean up the compiled binaries."

.PHONY: all run clean help