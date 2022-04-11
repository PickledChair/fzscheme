SRCS=$(wildcard *.c)
OBJS=$(SRCS: .c=.o)

fzscheme: $(OBJS)
	$(CC) $(CFLAGS) -o fzscheme $(OBJS) $(LDFLAGS)

$(OBJS): fzscheme.h

.PHONY: run
run: fzscheme
	./$<

.PHONY: clean
clean:
	-rm fzscheme
