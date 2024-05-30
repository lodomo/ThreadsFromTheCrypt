###############################################################################
#
# Author: Lorenzo Moon	
# Instructor: R. Jesse Chaney
# Assignment: Lab 4 - Threads from the Crypt 
# Description: Makefile for Threads from the Crypt. 
# Targets: all (default)
#          clean (Aliases: clena, claen, cls, cl)
#          tar : create a tarball of all source files and Makefile
#          git : auto git add and commit
#          debug : compile with debug flags
#          test	: run tests against master viktar program
#
###############################################################################

# Variables
PROGRAM = thread_hash 
CFLAGS = -Wall -Wextra -Wshadow -Wunreachable-code -Wredundant-decls \
		 -Wmissing-declarations -Wold-style-definition -Wmissing-prototypes \
		 -Wdeclaration-after-statement -Wno-return-local-addr \
		 -Wunsafe-loop-optimizations -Wuninitialized -Werror \
		 -Wno-unused-parameter
DFLAGS = -DNOISY_DEBUG -g
LDFLAGS = -lz
CC = gcc
TAR_FILE = ${LOGNAME}_Lab4.tar.gz
TEST_FILE = test-threads.bash

.PHONY: test

all: $(PROGRAM) 

$(PROGRAM): $(PROGRAM).o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(PROGRAM).o: $(PROGRAM).c $(PROGRAM).h 
	$(CC) $(CFLAGS) -c $<

# I am bad at typing clean.
clean clena claen cls cl:
	rm -f $(PROGRAM) $(PROGRAM).o *~ \#*
	rm -f *.viktar *.bin *.out *.err *.txt *.jerr *.jout *.serr *.sout
	

tar:
	tar -cvaf ${TAR_FILE} *.c [Mm]akefile

git:
	git add . 
	git commit -m "Lazy commit via Make"
	git push

test:
	./$(TEST_FILE)
	

debug: $(PROGRAM).c $(PROGRAM).h 
	$(CC) $(CFLAGS) $(DFLAGS) -o $(PROGRAM) $(PROGRAM).c $(LDFLAGS)
