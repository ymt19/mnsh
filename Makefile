CFLAGS=-std=c11 -g -static
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

9cc: $(OBJS)
	$(CC) -o mnsh $(OBJS) $(LDFLAGS)

$(OBJS): mnsh.h

clean:
	rm -f 9cc *.o *~ tmp*

.PHONY: clean