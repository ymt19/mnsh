CFLAGS=-std=c11 -g -static
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

mnsh: $(OBJS)
	$(CC) -o mnsh $(OBJS) $(LDFLAGS)

$(OBJS): mnsh.h

clean:
	rm -f mnsh *.o *~ tmp*

.PHONY: clean