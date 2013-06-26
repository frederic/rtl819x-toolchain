/* Linuxthreads - a simple clone()-based implementation of Posix        */
/* threads for Linux.                                                   */
/* Copyright (C) 1996 Xavier Leroy (Xavier.Leroy@inria.fr)              */
/*                                                                      */
/* This program is free software; you can redistribute it and/or        */
/* modify it under the terms of the GNU Library General Public License  */
/* as published by the Free Software Foundation; either version 2       */
/* of the License, or (at your option) any later version.               */
/*                                                                      */
/* This program is distributed in the hope that it will be useful,      */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of       */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        */
/* GNU Library General Public License for more details.                 */

/* changed for uClibc */
#define __sched_get_priority_min sched_get_priority_min
#define __sched_get_priority_max sched_get_priority_max

/* Handling of thread attributes */

#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/param.h>
#include "pthread.h"
#include "internals.h"

#include <sys/resource.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdio_ext.h>
#include <stdlib.h>
#include <sys/resource.h>

libpthread_hidden_proto(pthread_attr_destroy)
libpthread_hidden_proto(pthread_attr_init)
libpthread_hidden_proto(pthread_attr_getdetachstate)
libpthread_hidden_proto(pthread_attr_setdetachstate)
libpthread_hidden_proto(pthread_attr_getinheritsched)
libpthread_hidden_proto(pthread_attr_setinheritsched)
libpthread_hidden_proto(pthread_attr_setschedparam)
libpthread_hidden_proto(pthread_attr_getschedparam)
libpthread_hidden_proto(pthread_attr_getschedpolicy)
libpthread_hidden_proto(pthread_attr_setschedpolicy)
libpthread_hidden_proto(pthread_attr_getscope)
libpthread_hidden_proto(pthread_attr_setscope)

/* NOTE: With uClibc I don't think we need this versioning stuff.
 * Therefore, define the function pthread_attr_init() here using
 * a strong symbol. */

//int __pthread_attr_init_2_1(pthread_attr_t *attr)
int pthread_attr_init(pthread_attr_t *attr)
{
  size_t ps = getpagesize ();

  attr->__detachstate = PTHREAD_CREATE_JOINABLE;
  attr->__schedpolicy = SCHED_OTHER;
  attr->__schedparam.sched_priority = 0;
  attr->__inheritsched = PTHREAD_EXPLICIT_SCHED;
  attr->__scope = PTHREAD_SCOPE_SYSTEM;
  attr->__guardsize = ps;
  attr->__stackaddr = NULL;
  attr->__stackaddr_set = 0;
  attr->__stacksize = STACK_SIZE - ps;
  return 0;
}
libpthread_hidden_def(pthread_attr_init)

/* uClibc: leave out this for now. */
#if DO_PTHREAD_VERSIONING_WITH_UCLIBC
#if defined __PIC__ && defined DO_VERSIONING
default_symbol_version (__pthread_attr_init_2_1, pthread_attr_init, GLIBC_2.1);

int __pthread_attr_init_2_0(pthread_attr_t *attr)
{
  attr->__detachstate = PTHREAD_CREATE_JOINABLE;
  attr->__schedpolicy = SCHED_OTHER;
  attr->__schedparam.sched_priority = 0;
  attr->__inheritsched = PTHREAD_EXPLICIT_SCHED;
  attr->__scope = PTHREAD_SCOPE_SYSTEM;
  return 0;
}
symbol_version (__pthread_attr_init_2_0, pthread_attr_init, GLIBC_2.0);
#else
strong_alias (__pthread_attr_init_2_1, pthread_attr_init)
#endif
#endif /* DO_PTHREAD_VERSIONING_WITH_UCLIBC */

int pthread_attr_destroy(pthread_attr_t *attr attribute_unused)
{
  return 0;
}
libpthread_hidden_def(pthread_attr_destroy)


int pthread_attr_setdetachstate(pthread_attr_t *attr, int detachstate)
{
  if (detachstate < PTHREAD_CREATE_JOINABLE ||
      detachstate > PTHREAD_CREATE_DETACHED)
    return EINVAL;
  attr->__detachstate = detachstate;
  return 0;
}
libpthread_hidden_def(pthread_attr_setdetachstate)

int pthread_attr_getdetachstate(const pthread_attr_t *attr, int *detachstate)
{
  *detachstate = attr->__detachstate;
  return 0;
}
libpthread_hidden_def(pthread_attr_getdetachstate)

int pthread_attr_setschedparam(pthread_attr_t *attr,
                               const struct sched_param *param)
{
  int max_prio = __sched_get_priority_max(attr->__schedpolicy);
  int min_prio = __sched_get_priority_min(attr->__schedpolicy);

  if (param->sched_priority < min_prio || param->sched_priority > max_prio)
    return EINVAL;
  memcpy (&attr->__schedparam, param, sizeof (struct sched_param));
  return 0;
}
libpthread_hidden_def(pthread_attr_setschedparam)

int pthread_attr_getschedparam(const pthread_attr_t *attr,
                               struct sched_param *param)
{
  memcpy (param, &attr->__schedparam, sizeof (struct sched_param));
  return 0;
}
libpthread_hidden_def(pthread_attr_getschedparam)

int pthread_attr_setschedpolicy(pthread_attr_t *attr, int policy)
{
  if (policy != SCHED_OTHER && policy != SCHED_FIFO && policy != SCHED_RR)
    return EINVAL;
  attr->__schedpolicy = policy;
  return 0;
}
libpthread_hidden_def(pthread_attr_setschedpolicy)

int pthread_attr_getschedpolicy(const pthread_attr_t *attr, int *policy)
{
  *policy = attr->__schedpolicy;
  return 0;
}
libpthread_hidden_def(pthread_attr_getschedpolicy)

int pthread_attr_setinheritsched(pthread_attr_t *attr, int inherit)
{
  if (inherit != PTHREAD_INHERIT_SCHED && inherit != PTHREAD_EXPLICIT_SCHED)
    return EINVAL;
  attr->__inheritsched = inherit;
  return 0;
}
libpthread_hidden_def(pthread_attr_setinheritsched)

int pthread_attr_getinheritsched(const pthread_attr_t *attr, int *inherit)
{
  *inherit = attr->__inheritsched;
  return 0;
}
libpthread_hidden_def(pthread_attr_getinheritsched)

int pthread_attr_setscope(pthread_attr_t *attr, int scope)
{
  switch (scope) {
  case PTHREAD_SCOPE_SYSTEM:
    attr->__scope = scope;
    return 0;
  case PTHREAD_SCOPE_PROCESS:
    return ENOTSUP;
  default:
    return EINVAL;
  }
}
libpthread_hidden_def(pthread_attr_setscope)

int pthread_attr_getscope(const pthread_attr_t *attr, int *scope)
{
  *scope = attr->__scope;
  return 0;
}
libpthread_hidden_def(pthread_attr_getscope)

int __pthread_attr_setguardsize(pthread_attr_t *attr, size_t guardsize)
{
  size_t ps = getpagesize ();

  /* First round up the guard size.  */
  guardsize = roundup (guardsize, ps);

  /* The guard size must not be larger than the stack itself */
  if (guardsize >= attr->__stacksize) return EINVAL;

  attr->__guardsize = guardsize;

  return 0;
}
weak_alias (__pthread_attr_setguardsize, pthread_attr_setguardsize)

int __pthread_attr_getguardsize(const pthread_attr_t *attr, size_t *guardsize)
{
  *guardsize = attr->__guardsize;
  return 0;
}
weak_alias (__pthread_attr_getguardsize, pthread_attr_getguardsize)

int __pthread_attr_setstackaddr(pthread_attr_t *attr, void *stackaddr)
{
  attr->__stackaddr = stackaddr;
  attr->__stackaddr_set = 1;
  return 0;
}
weak_alias (__pthread_attr_setstackaddr, pthread_attr_setstackaddr)

int __pthread_attr_getstackaddr(const pthread_attr_t *attr, void **stackaddr)
{
  /* XXX This function has a stupid definition.  The standard specifies
     no error value but what is if no stack address was set?  We simply
     return the value we have in the member.  */
  *stackaddr = attr->__stackaddr;
  return 0;
}
weak_alias (__pthread_attr_getstackaddr, pthread_attr_getstackaddr)

int __pthread_attr_setstacksize(pthread_attr_t *attr, size_t stacksize)
{
  /* We don't accept value smaller than PTHREAD_STACK_MIN.  */
  if (stacksize < PTHREAD_STACK_MIN)
    return EINVAL;

  attr->__stacksize = stacksize;
  return 0;
}
weak_alias (__pthread_attr_setstacksize, pthread_attr_setstacksize)

int __pthread_attr_getstacksize(const pthread_attr_t *attr, size_t *stacksize)
{
  *stacksize = attr->__stacksize;
  return 0;
}
weak_alias (__pthread_attr_getstacksize, pthread_attr_getstacksize)

extern int *__libc_stack_end;
void *pthread_getattr_np (pthread_t thread, pthread_attr_t * attr)
{
  static void *stackBase = 0;
  static size_t stackSize = 0;
  int ret = 0;
  /* Stack size limit.  */
  struct rlimit rl;

  /* The safest way to get the top of the stack is to read
     /proc/self/maps and locate the line into which
     __libc_stack_end falls.  */
  FILE *fp = fopen ("/proc/self/maps", "rc");
  if (fp == NULL)
    ret = errno;
  /* We need the limit of the stack in any case.  */
  else if (getrlimit (RLIMIT_STACK, &rl) != 0)
    ret = errno;
  else
    {
      /* We need no locking.  */
      __fsetlocking (fp, FSETLOCKING_BYCALLER);

      /* Until we found an entry (which should always be the case)
         mark the result as a failure.  */
      ret = ENOENT;

      char *line = NULL;
      size_t linelen = 0;
      uintptr_t last_to = 0;

      while (!feof_unlocked (fp))
        {
          if (getdelim (&line, &linelen, '\n', fp) <= 0)
            break;

          uintptr_t from;
          uintptr_t to;
          if (sscanf (line, "%x-%x", &from, &to) != 2)
            continue;
          if (from <= (uintptr_t) __libc_stack_end
              && (uintptr_t) __libc_stack_end < to)
            {
              /* Found the entry.  Now we have the info we need.  */
              attr->__stacksize = rl.rlim_cur;
#ifdef _STACK_GROWS_UP
              /* Don't check to enforce a limit on the __stacksize */
              attr->__stackaddr = (void *) from;
#else
              attr->__stackaddr = (void *) to;

              /* The limit might be too high.  */
              if ((size_t) attr->__stacksize >
                  (size_t) attr->__stackaddr - last_to)
                attr->__stacksize = (size_t) attr->__stackaddr - last_to;
#endif

              /* We succeed and no need to look further.  */
              ret = 0;
              break;
            }
          last_to = to;
        }

      fclose (fp);
      free (line);
    }
#ifndef _STACK_GROWS_UP
  stackBase = (char *) attr->__stackaddr - attr->__stacksize;
#else
  stackBase = attr->__stackaddr;
#endif
  stackSize = attr->__stacksize;
  return (void *) (stackBase + stackSize);
}

int __pthread_attr_getstack (const pthread_attr_t * attr, void **stackaddr,
                         size_t * stacksize)
{
  /* XXX This function has a stupid definition.  The standard specifies
     no error value but what is if no stack address was set?  We simply
     return the value we have in the member.  */
#ifndef _STACK_GROWS_UP
  *stackaddr = (char *) attr->__stackaddr - attr->__stacksize;
#else
  *stackaddr = attr->__stackaddr;
#endif
  *stacksize = attr->__stacksize;
  return 0;
}
weak_alias (__pthread_attr_getstack, pthread_attr_getstack)
