/*
 * Tony Givargis
 * Copyright (C), 2023
 * University of California, Irvine
 *
 * CS 238P - Operating Systems
 * jitc.c
 */

#include "jitc.h"
#include "system.h"
#include <dlfcn.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

/**
 * Needs:
 *   fork()
 *   execv()
 *   waitpid()
 *   WIFEXITED()
 *   WEXITSTATUS()
 *   dlopen()
 *   dlclose()
 *   dlsym()
 */

/* research the above Needed API and design accordingly */

struct jitc
{
  void *handle;
};

struct jitc *shared_object;

int jitc_compile(const char *input, const char *output)
{
  int pid = fork();
  char *argv[] = {"/usr/bin/gcc", "-O3", "-fpic", "-shared", "-Werror", "-Wextra", "-o", "", "", NULL};
  argv[7] = (char *)output;
  argv[8] = (char *)input;

  if (!pid)
  {
    execv("/usr/bin/gcc", argv);
    exit(0);
  }
  else
  {
    int child_status = 0;
    waitpid(pid, &child_status, 0);
    if (WIFEXITED(child_status))
    {
      return WEXITSTATUS(child_status);
    }
    return child_status;
  }
}

struct jitc *jitc_open(const char *path_name)
{
  shared_object = malloc(sizeof(struct jitc));
  shared_object->handle = dlopen(path_name, RTLD_LAZY | RTLD_LOCAL);
  return shared_object;
}

void jitc_close(struct jitc *jitc)
{
  dlclose(jitc->handle);
  FREE(shared_object);
}

long jitc_lookup(struct jitc *jitc, const char *symbol)
{
  return (long)dlsym(jitc->handle, symbol);
}
