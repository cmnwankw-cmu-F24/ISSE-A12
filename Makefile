CFLAGS=-Wall -Werror -g -fsanitize=address
TARGETS=plaidsh
LIBS= -lreadline 


all: $(TARGETS)

plaidsh: plaidsh.c
	gcc $(CFLAGS) $^ $(LIBS) -o $@


clean:
	rm -f $(TARGETS)
