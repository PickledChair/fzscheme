SRCS=$(wildcard *.c)
OBJS=$(SRCS: .c=.o)

fzscheme: $(OBJS)

$(OBJS): fzscheme.h

.PHONY: run
run: fzscheme
	./$<

.PHONY: clean
clean:
	-rm *.o
	-rm fzscheme
