###############################################################################
#
# Author: Lorenzo Moon	
# Instructor: R. Jesse Chaney
# Assignment: Lab 4 - Threads from the Crypt 
# Description: Makefile for Threads from the Crypt. 
# Targets: all (default)
# 		   thread_hash     : the executable
# 		   thread_hash.o   : the object file
# 		   clean           : remove all generated files
# 		   tar             : create a tarball of the files
# 		   git             : add, commit, and push to git
#  	       test            : run the test script
#  	       debug           : compile with debug flags
#  	       memcheck        : run valgrind on the program
#
###############################################################################

PROGRAM = thread_hash
CC = gcc
CFLAGS = -Wall -Wextra -Wshadow -Wunreachable-code \
		 -Wredundant-decls -Wmissing-declarations \
	     -Wold-style-definition -Wmissing-prototypes \
		 -Wdeclaration-after-statement -Wno-return-local-addr \
		 -Wunsafe-loop-optimizations -Wuninitialized -Werror \
		 -Wno-unused-parameter -Wno-string-compare -Wno-stringop-overflow \
		 -Wno-stringop-overread -Wno-stringop-truncation
LDFLAGS = -lcrypt
DFLAGS = -g -DNOISY_DEBUG
TAR_FILE = ${LOGNAME}_Lab4.tar.gz
TEST_FILE = test-thread_hash.bash

all: $(PROGRAM)

$(PROGRAM): $(PROGRAM).o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(PROGRAM).o: $(PROGRAM).c $(PROGRAM).h 
	$(CC) $(CFLAGS) -c $<

# I am bad at typing clean.
clean clena claen cls cl:
	rm -f $(PROGRAM) $(PROGRAM).o *~ \#*
	rm -f *.output *.error

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

# TODO SET THIS UP FOR THE 10 WORDS 
memcheck: $(PROGRAM)
	valgrind --track-origins=yes --leak-check=full ./$(PROGRAM) -i ./Master/passwords10.txt -d ./Master/plain10.txt
