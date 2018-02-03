CFLAGS         += -Ideps -I. -Wall -pedantic
DIRDEPS         = deps
SRC             = $(wildcard postfix/*.c) $(wildcard tests/*.c)
DEPS            = $(wildcard $(DIRDEPS)/*/*.c)
OBJS            = $(DEPS:.c=.o) $(SRC:.c=.o)
DIRCMOCKERY     = $(DIRDEPS)/cmockery/
CMOCKERYCONFIGH = $(DIRCMOCKERY)/config.h
TEST            = runtests

default: $(OBJS)

$(CMOCKERYCONFIGH):
	cd $(DIRCMOCKERY) && chmod u+x configure && ./configure

%.o: %.c
	$(CC) $< -c -o $@ $(CFLAGS)

$(TEST): $(CMOCKERYCONFIGH) $(OBJS)
	$(CC) -o $@ $(OBJS) $(LDFLAGS)

test: $(TEST)
	@./$(TEST)

clean:
	$(foreach c, $(OBJS), rm -f $(c))
	rm -f $(TEST)
