CFLAGS   = -Wall -std=gnu99
INCLUDES = -I .
OBJDIR   = obj

TESTER_SRCS = err_exit.c defines.c main.c
TESTER_OBJS = $(addprefix $(OBJDIR)/, $(TESTER_SRCS:.c=.o))

all: $(OBJDIR) tester

tester: $(TESTER_OBJS)
	@echo "Creazione dell'eseguibile: "$@
	@$(CC) $^ -o $@  -lm


$(OBJDIR):
	@mkdir -p $(OBJDIR)

$(OBJDIR)/%.o: %.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ -c $<

clean:
	@rm -vf ${TESTER_OBJS}
	@rm -vf tester
	@rm -rf ${OBJDIR}
	@ipcrm -a
	@echo "Rimozione dei file oggetto ed eseguibili..."

.PHONY: run clean
