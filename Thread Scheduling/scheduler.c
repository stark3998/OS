/**
 * Tony Givargis
 * Copyright (C), 2023
 * University of California, Irvine
 *
 * CS 238P - Operating Systems
 * scheduler.c
 */

#undef _FORTIFY_SOURCE

#include <unistd.h>
#include <signal.h>
#include <setjmp.h>
#include "system.h"
#include "scheduler.h"

/**
 * Needs:
 *   setjmp()
 *   longjmp()
 */

/* research the above Needed API and design accordingly */

struct thread_space
{
    jmp_buf ctx;
    void *arg_fnc;
    scheduler_fnc_t fnc;
    enum
    {
        STATUS_,
        STATUS_RUNNING,
        STATUS_SLEEPING,
        STATUS_TERMINATED
    } status;
    struct
    {
        void *memory_; /*Put whatever we get from Malloc*/
        void *memory;  /*Memory Address after Memory Align*/
    } stack;
    struct thread_space *link;
};

static struct
{
    struct thread_space *head;
    struct thread_space *curr_thread; /* Update every time we switch */
    jmp_buf ctx;
} state;

void destroy(void)
{
    struct thread_space *temp = state.head;
    struct thread_space *temp2;
    while (temp != NULL)
    {
        temp2 = temp;
        FREE(temp);
        temp = temp2->link;
    }
    state.head = NULL;
    state.curr_thread = NULL;
    /*
        for i in linked_list:
            Traverse the link from *head
            Deallocate -> 1) Stack for Thread, 2) Thread
        state.head = NULL
        state.Thread = NULL
    */
}

struct thread_space *thread_candidate(void)
{
    struct thread_space *temp = state.curr_thread ? state.curr_thread : state.head;
    while (temp != NULL)
    {
        if (temp->status == STATUS_SLEEPING || temp->status == STATUS_)
        {
            return temp;
        }
        temp = temp->link;
    }

    temp = state.head;
    while (temp != state.curr_thread)
    {
        if (temp->status == STATUS_SLEEPING || temp->status == STATUS_)
        {
            return temp;
        }
        temp = temp->link;
    }

    return NULL;
}

void schedule(void)
{
    struct thread_space *thread_c = thread_candidate();
    if (thread_c == NULL)
    {
        return;
    }
    else
    {
        state.curr_thread = thread_c;
    }
    /*
        thread_space thread_c = thread_candidate();
        if thread_c = empty {
            return
        }
        else {
            state.thread = thread_c
            if thread->status != STATUS_INIT {
                thread->status = RUNNING
                longjmp(thread->ctx);
            }
            else {
                Create a stack, allocate it
                call the function instead of longjump
            }
        }
    */
}

void move_stack_ptr(void)
{
    uint64_t rsp;
    rsp = 17;
    __asm__ volatile("mov %[rs], %%rsp \n" : [rs] "+r"(rsp)::);
}

/* rsp=(uint64_t)thread->stack.memory+SZ_STACK;
__asm__ volatile ("mov %[rs], %%rsp \n" : [rs] "+r" (rsp) ::); */

int scheduler_create(scheduler_fnc_t fnc, void *arg)
{
    size_t pg_size = page_size();
    struct thread_space *thread = (struct thread_space *)malloc(sizeof(struct thread_space));
    thread->status = STATUS_;
    thread->stack.memory_ = malloc(2 * pg_size);
    thread->stack.memory = memory_align(thread->stack.memory_, pg_size);
    thread->fnc = fnc;
    thread->arg_fnc = arg;

    thread->link = state.head;
    state.head = thread;

    /* allocate thread obj
        init thread -> fnc,arg; allocate stack
        link it to state head
    */
    return 0;
}

void scheduler_execute(void)
{
    setjmp(state.ctx);
    schedule();
    destroy();
    /*
    checkpoint -> store in state.ctx, Just for our scheduler
    schedule(); -> Does not return anything, Only returns null when nothing is left to run
    destroy();
    */
}

void scheduler_yield(void)
{
    /*
        checkpoint -> store the context of current_thread -> state.thread->ctx
        state.thread->status = SLEEPING
        longjmp(state.ctx) -> this will take us back to scheduler_execute, it will find a new thread to run, run it and bring us back here
    */
}
