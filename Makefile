CFLAGS=-Wall -Werror -g -fsanitize=address
TARGETS=plaidsh ps_test
OBJS=clist.o tlist.o tokenize.o pipeline.o parse.o
HDRS=clist.h tlist.h token.h tokenize.h pipeline.h parse.h
LIBS=-lasan -lm -lreadline 


all: $(TARGETS)

plaidsh: $(OBJS) plaidsh.o
	gcc $(LDFLAGS) $^ $(LIBS) -o $@

ps_test: $(OBJS) ps_test.o
	gcc $(LDFLAGS) $^ $(LIBS) -o $@

%.o: %.c $(HDRS)
	gcc -c $(CFLAGS) $< -o $@

clean:
	rm -f *.o $(TARGETS)
