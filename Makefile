CFLAGS=-Wall -Werror -g -fsanitize=address
TARGETS=plaidsh
LIBS= -lreadline 


all: $(TARGETS)

plaidsh:  clist.h clist.c token.h tokenize.h tokenize.c plaidsh.c
	gcc $(CFLAGS) $^ $(LIBS) -o $@


clean:
	rm -f $(TARGETS)
