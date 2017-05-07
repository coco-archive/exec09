prefix ?= /usr/local
#prefix ?= /opt/gcc6809

CC = $(prefix)/bin/m6809-sim-none-gcc

SRC = $(wildcard *.c)
S19 = $(sort $(SRC:.c=.s19))
LST = $(addsuffix .lst,$(SRC))

CFLAGS ?= -Wall -O2 -g -Wa,-av,=$<.lst

%.s19: %.c
	$(CC) $(CFLAGS) $(CPPFLAGS) $< -o $@


all: $(S19)

clean:
	rm -f $(S19) $(LST)

tests: $(S19)
	@for exe in *.s19 ; \
	do \
		printf "\nStarting %s...\n" $$exe ; \
		$(prefix)/bin/m6809-run $$exe ; \
		printf "%s returned with %s\n\n" $$exe $$? ; \
	done
