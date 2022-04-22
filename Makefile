# Compiler and Linker
CC = gcc

# The Target Binary Program
CLIENT = client_0
SERVER = server

# Directories, Source, Includes, Objects and Binary
SRCDIR = src
CLIENT_DIR = $(SRCDIR)/client
SERVER_DIR = $(SRCDIR)/server

INCDIR = inc
BUILDDIR  = obj
TARGETDIR = bin

# Flags, Libraries and Includes
CFLAGS = -Wall -Wextra -Werror -Wpedantic -std=gnu99
LIB    = -lm
INC    = -I $(INCDIR)

SOURCES = $(shell find $(SRCDIR) -type f -name *.c)
CLIENT_SRCS = $(shell find $(CLIENT_DIR) -type f -name *.c)
SERVER_SRCS = $(shell find $(SERVER_DIR) -type f -name *.c)

OBJECTS = $(patsubst $(SRCDIR)/%,$(BUILDDIR)/%,$(SOURCES:.c=.o))
CLIENT_OBJS = $(patsubst $(CLIENT_DIR)/%,$(BUILDDIR)/%,$(CLIENT_SRCS:.c=.o))
SERVER_OBJS = $(patsubst $(SERVER_DIR)/%,$(BUILDDIR)/%,$(SERVER_SRCS:.c=.o))


RM = rm
RMFLAGS = -rf

# Defauilt Make
all: directories $(CLIENT) $(SERVER)

# Remake
remake: cleaner all

# Make the Directories
directories:
	@mkdir -p $(TARGETDIR)
	@mkdir -p $(BUILDDIR)

# Clean only Objecst
clean:
	@$(RM) $(RMFLAGS) $(BUILDDIR)
	@ipcrm -a
	@echo "Removed object files and executables..."

# Full Clean, Objects and Binaries
cleaner: clean
	@$(RM) $(RMFLAGS) $(TARGETDIR)

# Link
$(CLIENT): $(OBJECTS) $(CLIENT_OBJS)
	@echo "Making client executable: "$@
	$(CC) -o $(TARGETDIR)/$(CLIENT) $^ $(LIB)

$(SERVER): $(OBJECTS) $(SERVER_OBJS)
	@echo "Making server executable: "$@
	$(CC) -o $(TARGETDIR)/$(SERVER) $^ $(LIB)

# Compile
$(OBJECTS): $(SOURCES)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INC) -c -o $@ $<

$(CLIENT_OBJS): $(SOURCES) $(CLIENT_SRCS)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INC) -c -o $@ $<

$(SERVER_OBJS): $(SOURCES) $(SERVER_SRCS)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INC) -c -o $@ $<


# Non-File Targets
.PHONY: all remake clean cleaner directories
