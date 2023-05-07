CC = gcc
OBJS = IsraeliQueue.o HackerEnrollment.o HackEnrollment.o
EXEC = prog
DEBUG_FLAG = # now empty, assign -g for debug
COMP_FLAG = -std=c99 -Wall -Werror

$(EXEC) : $(OBJS)
	$(CC) $(DEBUG_FLAG) $(OBJS) -o $@
valgrind : debug $(OBJS)
	valgrind --leak-check=full ./HackEnrollment students.txt courses.txt hackers.txt queues.txt myout.txt
debug : $(OBJS)
	gcc -o HackEnrollment -g -std=c99 -Wall -Werror -pedantic-errors -DNDEBUG -lm *.c
HackEnrollment.o: HackEnrollment.c HackerEnrollment.h
	$(CC) -c $(DEBUG_FLAG) $(COMP_FLAG) $*.c
HackerEnrollment.o: HackerEnrollment.c IsraeliQueue.h
	$(CC) -c $(DEBUG_FLAG) $(COMP_FLAG) $*.c
IsraeliQueue.o: IsraeliQueue.c IsraeliQueue.h
	$(CC) -c $(DEBUG_FLAG) $(COMP_FLAG) $*.c
clean:
	rm -f $(OBJS) $(EXEC)