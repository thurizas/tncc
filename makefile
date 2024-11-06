CC=gcc
CCFLAGS=-g -std=c18 -pedantic -Wall -Wextra

LK=gcc
LKFLAGS=

PROJ=tncc

OBJS=tncc.o token.o lexer.o parser.o vector.o buffer.o util.o

# may need to install the packages
# libasan, libasan-debuginfo, and libasan-static
ifeq ($(mode),checked)
	CCFLAGS += -fsanitize=address
	LKFLAGS += -fsanitize=address -static-libasan
endif

$(PROJ) : $(OBJS)
	$(LK) $(LKFLAGS) $(OBJS) -o $(PROJ)

all : clean $(PROJ)

tncc.o : tncc.c
	$(CC) -c $(CCFLAGS) tncc.c -o tncc.o

token.o : token.c token.h
	$(CC) -c $(CCFLAGS) token.c -o token.o

lexer.o : lexer.c lexer.h
	$(CC) -c $(CCFLAGS) lexer.c -o lexer.o

parser.o : parser.c parser.h
	$(CC) -c $(CCFLAGS) parser.c -o parser.o

vector.o : vector.c vector.h
	$(CC) -c $(CCFLAGS) vector.c -o vector.o

buffer.o : buffer.c buffer.h
	$(CC) -c $(CCFLAGS) buffer.c -o buffer.o

util.o : util.c util.h
	$(CC) -c $(CCFLAGS) util.c -o util.o

vector_test : vector_test.c vector.o
	$(CC) $(CCFLAGS) vector.o vector_test.c -o vector_test

buffer_test : buffer_test.c buffer.o
	$(CC) $(CCFLAGS) buffer.o buffer_test.c -o buffer_test
clean :
	rm -f *.o
	rm -f *.*~
	rm -f $(PROJ)

