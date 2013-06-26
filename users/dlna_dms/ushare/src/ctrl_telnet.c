/* ctrltelnet.c - Telnet controler
 * Copyright (C) 2005-2007 Sven Almgren <sven@tras.se>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#define STR(x) _STR(x)
#define _STR(x) __STR(x)
#define __STR(x) #x

#include "../config.h"
#include "ctrl_telnet.h"
#include "minmax.h"
#include "trace.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h> /* For select */
#include <sys/time.h>
#include <unistd.h> /* For pipe */
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdarg.h>

#if (defined(____DISABLE_MUTEX) || 0)
#define pthread_mutex_lock(x)   printf(">>>> Locking   " __FILE__ ":" STR(__LINE__) " \t" #x "\n");
#define pthread_mutex_unlock(x) printf("<<<< Unlocking " __FILE__ ":" STR(__LINE__) " \t" #x "\n");
#endif

/**
 * @brief Structure holding data between the staring rutine and the thread
 */
typedef struct telnet_thread_data_t
{
  pthread_t thread;

  /* Litening socket */
  int listener;

  /* Socket used to terminate loop:
     0 is reading and 1 is sending, kill by sending to 1 */
  int killer[2];

  /* Our socket address */
  struct sockaddr_in local_address;

  /* Shared data buffer that can be used by others... */
  char shared_buffer[CTRL_TELNET_SHARED_BUFFER_SIZE];

  ctrl_telnet_client *clients;
} telnet_thread_data;

/**
 * @brief Struct for registerd commands
 */
typedef struct telnet_function_list_t
{
  /* Function name, or keyword, if you like */
  char *name;
  char *description;
  ctrl_telnet_command_ptr function;

  struct telnet_function_list_t *next;
} telnet_function_list;

/* Static yes used to set socketoptions */
static int yes = 1;
static telnet_thread_data ttd;
static telnet_function_list* functions = NULL;
static pthread_mutex_t functions_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t startstop_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t shared_lock = PTHREAD_MUTEX_INITIALIZER;
static int started = 0;

/* Threadfunction, core in telnet controler */
/**
 * @brief Thread function
 *
 * @param data Not used, leave as NULL
 */
static void *ctrl_telnet_thread (void *data);

/**
 * @brief Adds a new client to our list of new ones
 *
 * @param client to add
 */
static void ctrl_telnet_client_add (ctrl_telnet_client *client);

/**
 * @brief Removes "client" from our list of clients
 */
static void ctrl_telnet_client_remove (ctrl_telnet_client *client);

/**
 * @brief Updates an fd_set to contain the current set of clients
 *
 * @return max fd found in list
 */
static int ctrl_telnet_fix_fdset (fd_set* readable);

static void ctrl_telnet_tokenize (char *raw, int *argc, char ***argv);

static int ctrl_telnet_client_recv (ctrl_telnet_client *client);
static int ctrl_telnet_client_execute (ctrl_telnet_client *client);
static int ctrl_telnet_client_execute_line (ctrl_telnet_client *client,
                                            char *line);
static int ctrl_telnet_client_execute_line_safe (ctrl_telnet_client *client,
                                                 char *line);
static void ctrl_telnet_register_internals();

/**
 * @brief Starts a Telnet bound control interface
 *
 * @return 0 on success, -1 on error
 */
int
ctrl_telnet_start (int port)
{
  /* Start by making us threadsafe... */
  pthread_mutex_lock (&startstop_lock);

  /* Create listener socket */
  ttd.listener = socket (PF_INET, SOCK_STREAM, 0);
  if (ttd.listener == -1)
  {
    perror ("socket");
    pthread_mutex_unlock (&startstop_lock);
    return -1;
  }

  /* Clears us from "address already in use" errors */
  if (setsockopt (ttd.listener, SOL_SOCKET, SO_REUSEADDR,
                  &yes, sizeof (int)) == -1)
    perror ("setsockopt");

  ttd.local_address.sin_family = AF_INET;
  ttd.local_address.sin_addr.s_addr = INADDR_ANY;
  ttd.local_address.sin_port = htons (port);
  memset (&ttd.local_address.sin_zero, '\0',
          sizeof (ttd.local_address.sin_zero));

  if (bind (ttd.listener, (struct sockaddr *) &ttd.local_address,
            sizeof (ttd.local_address)) == -1)
  {
    perror ("bind");
    pthread_mutex_unlock (&startstop_lock);
    return -1;
  }

  if (listen (ttd.listener, CTRL_TELNET_BACKLOG) == -1)
  {
    perror ("listen");
    pthread_mutex_unlock (&startstop_lock);
    return -1;
  }

  print_log (ULOG_NORMAL, "Listening on telnet port %u\n", port);

  /* Create killer pipes */
  if (pipe (ttd.killer))
  {
    perror ("Failed to create killer pipe");
    pthread_mutex_unlock (&startstop_lock);
    return -1; /* FIXME. Kill all sockets... not critical,, but still */
  }

  if (pthread_create (&ttd.thread, NULL, ctrl_telnet_thread, NULL))
  {
    /* FIXME: Killall sockets... */
    perror ("Failed to create thread");
    pthread_mutex_unlock (&startstop_lock);
    return -1;
  }

  started = 1;
  ctrl_telnet_register_internals ();
  pthread_mutex_unlock (&startstop_lock);

  return 0;
}

/**
 * @brief Stops all telnet bound control interfaces
 */
void
ctrl_telnet_stop (void)
{
  pthread_mutex_lock (&startstop_lock);

  if (!started)
  {
    pthread_mutex_unlock (&startstop_lock);
    return;
  }

  /* yes is int, which is bigger then char, so this should be safe */
  write (ttd.killer[1], &yes, sizeof (char));

  pthread_mutex_unlock (&startstop_lock);
  pthread_join (ttd.thread, NULL);
}

/**
 * @brief Telnet thread function
 */
static void *
ctrl_telnet_thread (void *a __attribute__ ((unused)))
{
  /* fd_set with readable clients */
  fd_set fd_readable;

  /* Pointer to a client object */
  ctrl_telnet_client *client;

  int fd_max;

  while (1)
  {
    /* Get fds */
    fd_max = ctrl_telnet_fix_fdset (&fd_readable);

    if (select (fd_max + 1, &fd_readable, NULL, NULL, NULL) == -1)
    {
      perror ("select");
      /* FIXME: Close sockets */
      return NULL;
    }

    /* Check killer */
    if (FD_ISSET (ttd.killer[0], &fd_readable))
    {
      /* FIXME: TODO: Shut down sockets...  */

      /* Close listener and killer */
      close (ttd.listener);
      close (ttd.killer[0]);
      close (ttd.killer[1]);

      /* Check which fds that had anyhting to say... */
      client = ttd.clients;

      /* Say goodby to clients */
      while (client)
      {
        ctrl_telnet_client *current = client;
        ctrl_telnet_client_send (current,
                                 "\nServer is going down, Bye bye\n");
        client = client->next;
        ctrl_telnet_client_remove (current);
      }

      pthread_mutex_lock (&functions_lock);

      while (functions)
      {
        telnet_function_list *head = functions;
        functions = functions->next;

        free (head->name);
        if (head->description)
          free (head->description);

        free (head);
      }

      pthread_mutex_unlock (&functions_lock);

      return NULL;
    }

    /* Check for new connection */
    if (FD_ISSET (ttd.listener, &fd_readable))
    {
      socklen_t sl_addr;

      /* Create client object */
      client = malloc (sizeof (ctrl_telnet_client));

      if (!client)
      {
        perror ("Failed to create new client");
        return NULL;
      }

      memset (client, '\0', sizeof (ctrl_telnet_client));
      sl_addr = sizeof (client->remote_address);

      client->socket = accept (ttd.listener,
                               (struct sockaddr *) &client->remote_address,
                               &sl_addr);
      if (client->socket == -1)
      {
        perror ("accept");
        free (client);
      }
      else
      {
        ctrl_telnet_client_add (client);
        ctrl_telnet_client_execute_line_safe (client, "banner");
        ctrl_telnet_client_sendf (client, "For a list of registered commands type \"help\"\n");
        ctrl_telnet_client_send (client, "\n> ");
      }
    }

    /* Check which fds that had anyhting to say... */
    client = ttd.clients;

    /* Run through all clients and check if there's data avalible
       with FD_ISSET(current->socket) */
    while (client)
    {
      ctrl_telnet_client *current = client;
      client = client->next;

      if (FD_ISSET (current->socket, &fd_readable))
      {
        if (ctrl_telnet_client_recv (current) <= 0)
        {
          ctrl_telnet_client_remove (current);
          continue;
        }

        if (current->ready)
        {
          ctrl_telnet_client_execute (current);

          if (!current->exiting)
            ctrl_telnet_client_send (current, "\n> ");
          else
            ctrl_telnet_client_remove (current);
        }
      }
    }
  }
}

/**
 * @brief Adds a new client to our list of new ones
 *
 * @note This funtion is only called from a single thread,
 *       as such it won't need to be threadsafe
 * @param client to add
 */
static void
ctrl_telnet_client_add (ctrl_telnet_client *client)
{
  client->next = ttd.clients;
  ttd.clients = client;
}

/**
 * @brief Removes "client" from our list of clients
 *
 * @note This funtion is only called from a single thread,
 *       as such it won't need to be threadsafe
 * @param client to remove
 */
static void
ctrl_telnet_client_remove (ctrl_telnet_client *client)
{
  ctrl_telnet_client *tmp;

  /* Start by dealing with our head */
  if (client == ttd.clients)
    ttd.clients = client->next;
  else
  {
    for (tmp = ttd.clients; tmp->next; tmp = tmp->next)
    {
      if (tmp->next == client)
      {
        tmp->next = tmp->next->next;
        break;
      }
    }
  }

  close (client->socket);

  free (client);
}

/**
 * @brief Clears readable fd_set and adds every client to it,
 *        returns max fd found
 *
 * @param readable fd_set to update
 * @return Biggest fd
 */
static int
ctrl_telnet_fix_fdset (fd_set *readable)
{
  int maxfd;
  ctrl_telnet_client *client;

  maxfd = MAX (ttd.killer[0], ttd.listener);

  FD_ZERO (readable);
  FD_SET (ttd.listener, readable);
  FD_SET (ttd.killer[0], readable);

  client = ttd.clients;

  while (client)
  {
    if (client->socket > maxfd)
      maxfd = client->socket;

    FD_SET (client->socket, readable);

    client = client->next;
  }

  return maxfd;
}

static int
ctrl_telnet_client_recv (ctrl_telnet_client *client)
{
  int i;
  int nbytes;
  int buffer_free = CTRL_CLIENT_RECV_BUFFER_SIZE - client->buffer_recv_current - 1;

  nbytes = recv (client->socket,
                 client->buffer_recv + client->buffer_recv_current,
                 buffer_free, 0);
  if (nbytes <= 0)
  {
    close (client->socket);
    return nbytes;
  }

  client->buffer_recv_current += nbytes;
  client->buffer_recv[client->buffer_recv_current] = '\0';

  for (i = 0; i < client->buffer_recv_current; i++)
    if (client->buffer_recv[i] == '\n')
      client->ready = 1;

  return nbytes;
}

int
ctrl_telnet_client_send (const ctrl_telnet_client *client, const char *string)
{
  const char* cc = string;
  int len = strlen (cc);
  int sent = 0;
  int senttotal = 0;

  while ((cc - string) < len)
  {
    /* Use nonblocking just as a precation...
       and a failed write won't _really_ kill us */
    sent = send (client->socket, string, len - (cc - string), MSG_DONTWAIT);
    
    /* This will mark the socket as dead... just to be safe..
       and its only a telnet interface... reconnect and do it again */
    if (sent == -1)
      return -1;

    senttotal += sent;
    cc += sent;
  }

  return senttotal;
}

int
ctrl_telnet_client_sendf (const ctrl_telnet_client *client,
                          const char *format, ...)
{
  int retval;
  va_list ap;
  int len;

  pthread_mutex_lock (&shared_lock);

  va_start (ap, format);
  len = vsnprintf (ttd.shared_buffer,
                   CTRL_TELNET_SHARED_BUFFER_SIZE, format, ap);
  va_end (ap);

  /* Check if the message fitted inside the buffer, if not,
     either exit or adjust len to be buffersize, I choose exit for now */
  if (len >= CTRL_TELNET_SHARED_BUFFER_SIZE)
  {
    pthread_mutex_unlock (&shared_lock);
    /* FIXME: Return error or send what we've got? */
    return -1; /* Buffer was to small */
  }
  
  /* TODO: Might be good to have the option to specify str length so
     send doesn't have to recompute it... */
  retval = ctrl_telnet_client_send (client, ttd.shared_buffer);

  pthread_mutex_unlock (&shared_lock);

  return retval;
}

int
ctrl_telnet_client_sendsf (const ctrl_telnet_client *client,
                           char *buffer, int buffersize,
                           const char *format, ...)
{
  va_list ap;
  int len;

  va_start (ap, format);
  len = vsnprintf (buffer, buffersize, format, ap);
  va_end (ap);

  /* Check if the message fitted inside the buffer, if not,
     either exit or adjust len to be buffersize, I choose exit for now */
  if (len >= buffersize)
    return -1; /* Buffer was to small */

  /* TODO: Might be good to have the option to specify str length
     so send doesn't have to recompute it... */
  return ctrl_telnet_client_send (client, buffer);
}

/* FIXME: Ulgy non optimised version */
static int
ctrl_telnet_client_execute (ctrl_telnet_client *client)
{
  int i = 0;

  /* Check buffer for complete lines and execute them,,, */
  for (i = 0; i < client->buffer_recv_current; i++)
  {
    if (client->buffer_recv[i] == '\n' || client->buffer_recv[i] == '\r')
    {
      /* Replace newline with null (or \r) */
      client->buffer_recv[i] = '\0';

      /* Send line to execution */
      ctrl_telnet_client_execute_line_safe (client, client->buffer_recv);

      /* Check if next is either newline or CR, strip that too, if needed */
      if ((i + 1 < CTRL_CLIENT_RECV_BUFFER_SIZE) &&
          (client->buffer_recv[i+1]=='\n' || client->buffer_recv[i+1]=='\r'))
        client->buffer_recv[++i] = '\0';

      /* Remove processed line */
      memmove (client->buffer_recv, client->buffer_recv + i,
               client->buffer_recv_current - 1);
      client->buffer_recv_current -= (i + 1);
      i = -1;
    }
  }

  return 0; /* No syntax error checking yet */
}

static int
ctrl_telnet_client_execute_line_safe (ctrl_telnet_client *client, char *line)
{
  int retval;

  pthread_mutex_lock (&functions_lock);
  retval = ctrl_telnet_client_execute_line (client, line);
  pthread_mutex_unlock (&functions_lock);

  return retval;
}

static int
ctrl_telnet_client_execute_line (ctrl_telnet_client *client, char *line)
{
  int argc = 0;
  char **argv = NULL;
  telnet_function_list *node;
  char *line2 = strdup (line); /* To make it safer */
  ctrl_telnet_tokenize (line2, &argc, &argv);

  node = functions;

  if (*argv[0] == '\0')
  {
    free (argv);
    free (line2);
    return 0;
  }

  while (node)
  {
    if (!strcmp (node->name, argv[0]))
    {
      node->function (client, argc, argv);
      break;
    }

    node = node->next;
  }

  if (!node)
    ctrl_telnet_client_sendf (client, "%s: Command not found\n", argv[0]);

  free (argv);
  free (line2);

  return strlen (line);
}

void
ctrl_telnet_register (const char *funcname,
                      ctrl_telnet_command_ptr funcptr,
                      const char *description)
{
  telnet_function_list *function;

  function = malloc (sizeof (telnet_function_list));
  function->name = strdup (funcname); /* Mayby use strndup...? */
  function->description = description ? strdup (description) : NULL;
  function->function = funcptr;

  pthread_mutex_lock (&functions_lock);
  function->next = functions;
  functions = function;
  pthread_mutex_unlock (&functions_lock);
}

/* Warning: This WILL edit the input string... use strdup or something
   if needed, also remember to free() argv as the first array is dynamic */
/* If *argv != NULL it'll first be free()ed... or realloc,
   make sure to clear *argv to null on initialization */
static void
ctrl_telnet_tokenize (char *raw, int *argc, char ***argv)
{
  int i;
  int has_backslash = 0;
  int has_quote = 0;
  char *pc = raw;

  if (!raw || !argc || !argv)
  {
    perror ("NULL in " __FILE__ " at line " STR (__LINE__));
    return;
  }

  /* (1/3) First run is just to count our arguments... */
  *argc = (raw[0] == '\0') ? 0 : 1;

  pc = raw;
  while (*pc)
  {
    switch (*pc)
    {
    case '\\':
      if (!has_backslash)
        has_backslash = 2; /* FULHACK */
      break;
    case ' ':
      if (!has_backslash && !has_quote)
        (*argc)++;
      break;
    case '"':
      if (!has_backslash)
        has_quote = !has_quote;
      
      break;
    }

    /* When we get a BS we set it to two, this makes it one,
       next run it will still be 1, then one after that is zero... FULHACK */
    if (has_backslash)
      has_backslash--;

    pc++;
  }

  /* Create argv */
  *argv = malloc (sizeof (char **) * ((*argc) + 1));

  /* (2/3) Parse throu one more time, this time filling argv (Pass 2 / 3) */
  i = 0;
  pc = raw;
  has_backslash = 0;
  has_quote = 0;
  (*argv)[0] = raw;

  while (*pc)
  {
    switch (*pc)
    {
    case '\\':
      if (!has_backslash)
        has_backslash = 2; /* FULHACK */
      break;
    case ' ':
      if (!has_backslash && !has_quote)
      {
        *pc = '\0';
        (*argv)[++i] = pc+1;
        pc++;
        continue;
      }
      break;
    case '"':
      if (!has_backslash)
        has_quote = !has_quote;
      break;
    }

    /* When we get a BS we set it to two, this makes it one,
       next run it will still be 1, then one after that is zero... FULHACK */
    if (has_backslash)
      has_backslash--;

    pc++;
  }

  /* Make last element (argc) point to null... */
  (*argv)[++i] = NULL;

  /* (3/3) Parse arguments to remove escapings and such */
  for (i = 0; (*argv)[i]; i++)
  {
    /* Set up environment */
    pc = (*argv)[i];
    has_backslash = 0;
    has_quote = 0;

    /* Remove leading and ending quotes, if existing */
    if (*pc == '"')
    {
      int len = strlen (pc);

      if (len > 0 && pc[len - 1] == '"')
        pc[len - 1] = '\0';
      memmove (pc, pc + 1, len);
    }

    /* Remove any special characters */
    while (*pc)
    {
      switch (*pc)
      {
      case '\\':
        if (!has_backslash)
        {
          has_backslash = 2; /* FULHACK */
          break;
        }
        /* Else: fall through */
      case ' ':
      case '"':
        if (has_backslash)
        {
          pc--;
          memmove (pc, pc + 1, strlen (pc)); /* FIXME: Not cheap */
        }
        break;
      }

      /* When we get a BS we set it to two, this makes it one,
         next run it will still be 1, then one after that is zero... */
      if (has_backslash)
        has_backslash--;
      
      pc++;
    }
  }
}

static void
help (ctrl_telnet_client *client, int argc, char **argv)
{
  int hidden = 0;
  ctrl_telnet_client_execute_line (client, "banner");
  
  if (argc < 2)
  {
    ctrl_telnet_client_send (client, "\n");
    ctrl_telnet_client_send (client, "Usage: help TOPIC\n");
    ctrl_telnet_client_send (client, "Valid topics are\n");
    ctrl_telnet_client_send
      (client, "  commands - For a list of registed commands\n");
    ctrl_telnet_client_send
      (client, "  syntax - For a description of the interface syntax\n");
    return;
  }
  else
  {
    if (!strcmp ("commands", argv[1]))
    {
      telnet_function_list *node;

      node = functions;
      ctrl_telnet_client_send
        (client, "Registered command (command - description)\n");
      ctrl_telnet_client_send
        (client, "=======================================================\n");

      while (node)
      {
        /* Make functions without descriptions invisible */
        if (node->description)
          ctrl_telnet_client_sendf (client, "  %s - %s\n",
                                    node->name, node->description);
        else
          hidden++;

        node = node->next;
      }

      if (hidden)
        ctrl_telnet_client_sendf
          (client, "There's also %i hidden functions\n", hidden);
    } /* commands */
    else if (!strcmp ("syntax", argv[1]))
    {
      ctrl_telnet_client_send
        (client, "Syntax is easy: command parameters\n");
      ctrl_telnet_client_send
        (client, "  Each new word is a new argument, unless the space is precided\n");
      ctrl_telnet_client_send
        (client, "  a backslash (\\), or if a set of words are surrounded by quotes\n");
      ctrl_telnet_client_send
        (client, "  (\"). To get a litteral quote you can escape it as \\\".\n");
      ctrl_telnet_client_send (client, "\n");
      ctrl_telnet_client_send (client, "STUB\n");
    }
    else
      ctrl_telnet_client_send (client, "Unknown topic\n");
  }
}

static void
banner (ctrl_telnet_client *client,
        int argc __attribute__ ((unused)),
        char **argv __attribute__ ((unused)))
{
  ctrl_telnet_client_sendf (client, "%s (%s) (Built %s)\n",
                            PACKAGE_NAME, VERSION, __DATE__);
}

static void
echo (ctrl_telnet_client *client, int argc, char **argv)
{
  int i;
  
  for (i = 1; i < argc; i++)
    ctrl_telnet_client_sendf (client, "%s%s", (i > 1 ? " " : ""), argv[i]);
  ctrl_telnet_client_send (client, "\n");
}

static void
echod (ctrl_telnet_client *client, int argc, char **argv)
{
  int i;

  ctrl_telnet_client_sendf (client, "Argc: %i\n", argc);

  for (i = 0; i < argc; i++)
    ctrl_telnet_client_sendf (client, "%i: '%s'\n", i, argv[i]);
}

static void
ctrl_telnet_exit (ctrl_telnet_client *client,
                  int argc __attribute__ ((unused)),
                  char **argv __attribute__ ((unused)))
{
  client->exiting = 1;
  ctrl_telnet_client_send (client, "Bye bye\n");
}

static void
ctrl_telnet_register_internals (void)
{
  ctrl_telnet_register ("echo", echo, "Echos all arguments");
  ctrl_telnet_register ("echod", echod, "Echos all arguments but with each argument on a new line... DEBUG");
  ctrl_telnet_register ("help", help, "Display help");
  ctrl_telnet_register ("banner", banner, NULL);
  ctrl_telnet_register ("exit", ctrl_telnet_exit, "Exits this interface (Or CTRL+D then Enter)");
  /* CTRL+D... But it has to be fallowd by a new line */
  ctrl_telnet_register ("\4", ctrl_telnet_exit, NULL);
}
