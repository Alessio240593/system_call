# Compiler and Linker
CC = gcc

# The Target Binary Program
CLIENT = client_0
SERVER = server

# Directories, Source, Includes, Objects and Binary
SRCDIR = src
CLIENT_DIR = $(SRCDIR)/client
SERVER_DIR = $(SRCDIR)/server

INCDIR    = inc
OBJDIR    = obj
TARGETDIR = bin

# Flags, Libraries and Includes
#CFLAGS = -ansi -pedantic -Wall -Wextra -Werror
CFLAGS = -std=gnu99 -Wall -Wextra -Werror -Wpedantic
LIB    = -lm
INC    = -I $(INCDIR)

SOURCES     = $(shell find $(SRCDIR) -maxdepth 1 -type f -name *.c)
CLIENT_SRCS = $(shell find $(CLIENT_DIR) -maxdepth 1 -type f -name *.c)
SERVER_SRCS = $(shell find $(SERVER_DIR) -maxdepth 1 -type f -name *.c)

OBJECTS = $(patsubst $(SRCDIR)/%,$(OBJDIR)/%,$(SOURCES:.c=.o))
CLIENT_OBJS = $(patsubst $(CLIENT_DIR)/%,$(OBJDIR)/%,$(CLIENT_SRCS:.c=.o))
SERVER_OBJS = $(patsubst $(SERVER_DIR)/%,$(OBJDIR)/%,$(SERVER_SRCS:.c=.o))


RM = rm
RMFLAGS = -rf

# Defauilt Make
all: directories $(CLIENT) $(SERVER)
client: directories $(CLIENT)
server: directories $(SERVER)

# Remake
remake: cleaner all

# Make the Directories
directories:
	@mkdir -p $(TARGETDIR)
	@mkdir -p $(OBJDIR)

# Clean only Objecst
clean:
	@$(RM) $(RMFLAGS) $(OBJDIR)
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
# $(OBJECTS): $(SOURCES)
$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INC) -o $@ -c $<

$(CLIENT_OBJS): $(SOURCES) $(CLIENT_SRCS)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INC) -o $@ -c $<

$(SERVER_OBJS): $(SOURCES) $(SERVER_SRCS)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INC) -o $@ -c $<


# Non-File Targets
.PHONY: all remake clean cleaner directories
