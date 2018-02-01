DIROBJ         = obj
DIRBIN         = bin
DIRLIB         = lib
DIRDEPS        = deps
DIRSRC         = postfix
DIRTESTSRC     = tests
DIRCHECK       = $(DIRDEPS)/check
DIRCHECLIB     = $(DIRCHECK)/src/.libs
CHEKMAKE       = $(DIRCHECK)/Makefile
CFLAGS        += -g -I$(DIRCHECK)/src/ -I$(DIRCHECK) -I$(DIRSRC)
LDFLAGS       += -L$(DIRLIB)
SRC           := $(wildcard $(DIRSRC)/*.c)
OBJ           := $(addprefix $(DIROBJ)/,$(notdir $(SRC:.c=.o)))
TESTSRC       := $(wildcard $(DIRTESTSRC)/*.c)
TESTOBJ        := $(addprefix $(DIROBJ)/,$(notdir $(TESTSRC:.c=.o)))
TEST           = $(DIRBIN)/postfixtest
LIBCHECK       = $(DIRLIB)/libcheck.a
LIBPOSTFIX     = $(DIRLIB)/libpostfix.a

default: $(LIBPOSTFIX)
	

$(DIROBJ): $(DIRBIN)
	@mkdir -p $@

$(DIRBIN):
	@mkdir -p $@

$(DIRLIB):
	@mkdir -p $@

$(CHEKMAKE):
	@cd $(DIRCHECK) && autoreconf --install && ./configure

$(LIBCHECK): $(DIRLIB) $(DIRCHECK)
	@cd $(DIRCHECK) && make
	@cp $(DIRCHECLIB)/libcheck.a $@

$(LIBPOSTFIX): $(OBJ) $(DIRLIB)
	$(AR) -rsc $@ $(OBJ)

$(DIROBJ)/%.o: $(DIRSRC)/%.c $(DIROBJ)
	$(CC) $(CFLAGS) -c $< -o $@

$(DIROBJ)/%.o: $(DIRTESTSRC)/%.c $(DIROBJ)
	$(CC) $(CFLAGS) -c $< -o $@

$(TEST): $(TESTOBJ) $(LIBCHECK) $(LIBPOSTFIX)
	$(LD) $(LDFLAGS) -lcheck -lpostfix -o $@ $(TESTOBJ)

test: $(TEST)
	$(TEST)

clean:
	rm -rf $(DIROBJ)
	rm -rf $(DIRBIN)
	rm -rf $(DIRLIB)
	# cd $(DIRCHECK) && make clean