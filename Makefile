# TODO: custom flags for tests
CC             = clang
LD             = clang
DIROBJ         = .obj
DIRBIN         = .bin
DIRLIB         = .lib
DIRDEPS        = deps
DIRSRC         = postfix
DIRTESTSRC     = tests
DIRCHECK       = $(DIRDEPS)/check
DIRCHECLIB     = $(DIRCHECK)/src/.libs
CHEKMAKE       = $(DIRCHECK)/Makefile
CCFLAGS       += -g -I$(DIRCHECK)/src/ -I$(DIRCHECK) -I$(DIRSRC)
LDFLAGS       += -L$(DIRLIB) -lcheck
SRC           := $(wildcard $(DIRSRC)/*.c)
OBJ           := $(addprefix $(DIROBJ)/,$(notdir $(SRC:.c=.o)))
TESTSRC       := $(wildcard $(DIRTESTSRC)/*.c)
TESOBJ        := $(addprefix $(DIROBJ)/,$(notdir $(TESTSRC:.c=.o)))
TEST           = $(DIRBIN)/postfixtest
LIBCHECK       = $(DIRLIB)/libcheck.a

default: test

$(DIROBJ): $(DIRBIN)
	@mkdir -p $@

$(DIRBIN):
	@mkdir -p $@

$(DIRLIB):
	@mkdir -p $@

$(CHEKMAKE):
	@cd $(DIRCHECK) && autoreconf --install && ./configure

$(LIBCHECK): $(DIRLIB)
	@cd $(DIRCHECK) && make && make check
	@cp $(DIRCHECLIB)/libcheck.a $@

$(DIROBJ)/%.o: $(DIRSRC)/%.c $(DIROBJ)
	$(CC) $(CCFLAGS) -c $< -o $@

$(DIROBJ)/%.o: $(DIRTESTSRC)/%.c $(DIROBJ) $(DIRCHECLIB)
	$(CC) $(CCFLAGS) -c $< -o $@

$(TEST): $(OBJ) $(TESOBJ) $(LIBCHECK)
	$(LD) $(LDFLAGS) -o $@ $^

test: $(TEST)
	$(TEST)

clean:
	rm -rf $(DIROBJ)
	rm -rf $(DIRBIN)
	# rm -rf $(DIRLIB)
	# cd $(DIRCHECK) && make clean