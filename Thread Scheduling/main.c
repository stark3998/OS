/**
 * Tony Givargis
 * Copyright (C), 2023
 * University of California, Irvine
 *
 * CS 238P - Operating Systems
 * main.c
 */

#include "system.h"
#include "scheduler.h"

static void
_thread_(void *arg)
{
	const char *name;
	int i;

	name = (const char *)arg;
	for (i=0; i<100; ++i) {
		printf("%s %d\n", name, i);
		us_sleep(20000);
		scheduler_yield();
	}
}

int
main(int argc, char *argv[])
{
	UNUSED(argc);
	UNUSED(argv);

	if (scheduler_create(_thread_, "hello") ||
	    scheduler_create(_thread_, "world") ||
	    scheduler_create(_thread_, "love") ||
	    scheduler_create(_thread_, "this") ||
	    scheduler_create(_thread_, "course!")) {
		TRACE(0);
		return -1;
	}
	scheduler_execute();
	return 0;
}

/*
	We are gonna do Context Switching for Concurrency, Start fn F() and stop it to Call G(), then stop G() to call F(). Use POSIX pthread()
	Theres a register called Program Counter for every Fn
	Store the Program Counter for current fn and change the counter to PC of G()
	Store the content of all registers, Adjust Stack Pointer to point to G()
	C offers setjump() and longjump(), They can do the register switching job
	{
		jmp_buf env;
		setjump(env): setjump has a return value like fork. Takes a memory, Reads all the registers, and stores it in the memory buffer env
		lonjump(env): longjump takes the env variable and brings us back to the snapshot env.
		...
		...
		if(setjump(env)) {
			print(Back from Future)
		}
		else {
			Do Something
		}
		...
		...
		longjump(env) /Throws it back to setjump
	}

	We will implement pthread in a form of co-routines. We would have to call yield()
	For Extra Creds -> Setup a Timer to switch context, instead of yield()
	Co-operative switching is basically programs calling yield(), it doesnt automatically switch based on OS Timer Clock
	To select threads while yielding, we could use round robin, or priority or something like that. affinity flag specifies the physical cores that it would run on
	Assign a new Memory Space for each thread !
	struct thread_space {
		inp_buf ctx;
		int arg_fnc;
		enum {
			STATUS_,
			STATUS_RUNNING;
			STATUS_SLEEPING;
			STATUS_TERMINATED;
		} status;
		struct {
			void *memory_; /Put whatever we get from Malloc
			void *memory; /Memory Address after Memory Align
		} stack;
		struct thread_space *link;
	}

	static struct {
		struct thread *head;
		struct thread *curr_thread; /Update every time we switch
		jmp_buf ctx;
	} state;

	void destroy(void) {
		for i in linked_list:
			Traverse the link from *head
			Deallocate -> 1) Stack for Thread, 2) Thread
		state.head = NULL
		state.Thread = NULL
	}
	struct *thread thread_candidate(void) {
		for i in linked list:
			Traverse the list
			return next_thread (The onw whos either sleeping or initiated)
		
		traverse the whole list in circular fashion, If no thread is left, Call destroy and Exit
	}

	schedule() {
		identify who to run next ?

		return NULL if nothing is running
		Go Round Robin from the current thread;
	}
*/
