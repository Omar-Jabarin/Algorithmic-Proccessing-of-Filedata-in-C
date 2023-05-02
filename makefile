CC = gcc
OBJS = IsraeliQueue.o HackerEnrollment.o HackEnrollment.o
EXEC = prog
DEBUG_FLAG = # now empty, assign -g for debug
COMP_FLAG = -std=c99 -Wall -Werror

$(EXEC) : $(OBJS)
	$(CC) $(DEBUG_FLAG) $(OBJS) -o $@
HackEnrollment.o: HackEnrollment.c HackerEnrollment.h
	$(CC) -c $(DEBUG_FLAG) $(COMP_FLAG) $*.c
HackerEnrollment.o: HackerEnrollment.c IsraeliQueue.h
	$(CC) -c $(DEBUG_FLAG) $(COMP_FLAG) $*.c
IsraeliQueue.o: IsraeliQueue.c IsraeliQueue.h
	$(CC) -c $(DEBUG_FLAG) $(COMP_FLAG) $*.c
clean:
	rm -f $(OBJS) $(EXEC)