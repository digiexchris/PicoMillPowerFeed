/****************************************************************************
 * apps/examples/elf/tests/signal/signal.c
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.  The
 * ASF licenses this file to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance with the
 * License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
 * License for the specific language governing permissions and limitations
 * under the License.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <sys/types.h>

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#ifndef USEC_PER_MSEC
#  define USEC_PER_MSEC 1000L
#endif

#ifndef MSEC_PER_SEC
#  define MSEC_PER_SEC  1000L
#endif

#ifndef USEC_PER_SEC
#  define USEC_PER_SEC  (USEC_PER_MSEC * MSEC_PER_SEC)
#endif

#define SHORT_DELAY     (USEC_PER_SEC / 3)

/****************************************************************************
 * Private Data
 ****************************************************************************/

static int sigusr1_rcvd = 0;
static int sigusr2_rcvd = 0;

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Name: siguser_action
 ****************************************************************************/

/* NOTE: it is necessary for functions that are referred to by function pointers
 *  pointer to be declared with global scope (at least for ARM).  Otherwise,
 * a relocation type that is not supported by ELF is generated by GCC.
 */

void siguser_action(int signo, siginfo_t *siginfo, void *arg)
{
  printf("siguser_action: Received signo=%d siginfo=%p arg=%p\n",
         signo, siginfo, arg);

  if (signo == SIGUSR1)
    {
      printf("  SIGUSR1 received\n");
      sigusr2_rcvd = 1;
    }
  else if (signo == SIGUSR2)
    {
      printf("  SIGUSR2 received\n");
      sigusr1_rcvd = 2;
    }
  else
    {
      printf("  ERROR: Unexpected signal\n");
    }

  if (siginfo)
    {
      printf("siginfo:\n");
      printf("  si_signo  = %d\n",  siginfo->si_signo);
      printf("  si_code   = %d\n",  siginfo->si_code);
      printf("  si_errno  = %d\n",  siginfo->si_errno);
      printf("  si_value  = %d\n",  siginfo->si_value.sival_int);
    }
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: main
 ****************************************************************************/

int main(int argc, char **argv)
{
  struct sigaction act;
  struct sigaction oact1;
  struct sigaction oact2;
  pid_t mypid = getpid();
  union sigval sigval;
  int status;

  printf("Setting up signal handlers from pid=%d\n", mypid);

  /* Set up so that siguser_action will respond to SIGUSR1 */

  memset(&act, 0, sizeof(struct sigaction));
  act.sa_sigaction = siguser_action;
  act.sa_flags     = SA_SIGINFO;

  sigemptyset(&act.sa_mask);

  status = sigaction(SIGUSR1, &act, &oact1);
  if (status != 0)
    {
      fprintf(stderr, "Failed to install SIGUSR1 handler, errno=%d\n",
              errno);
      exit(2);
    }

  printf("Old SIGUSR1 sighandler at %p\n", oact1.sa_handler);
  printf("New SIGUSR1 sighandler at %p\n", siguser_action);

  /* Set up so that siguser_action will respond to SIGUSR2 */

  status = sigaction(SIGUSR2, &act, &oact2);
  if (status != 0)
    {
      fprintf(stderr, "Failed to install SIGUSR2 handler, errno=%d\n",
              errno);
      exit(2);
    }

  printf("Old SIGUSR2 sighandler at %p\n", oact2.sa_handler);
  printf("New SIGUSR2 sighandler at %p\n", siguser_action);
  printf("Raising SIGUSR1 from pid=%d\n",  mypid);

  fflush(stdout); usleep(SHORT_DELAY);

  /* Send SIGUSR1 to ourselves via kill() */

  printf("Kill-ing SIGUSR1 from pid=%d\n", mypid);
  status = kill(mypid, SIGUSR1);
  if (status != 0)
    {
      fprintf(stderr, "Failed to kill SIGUSR1, errno=%d\n", errno);
      exit(3);
    }

  usleep(SHORT_DELAY);
  printf("SIGUSR1 raised from pid=%d\n", mypid);

  /* Verify that we received SIGUSR1 */

  if (sigusr1_rcvd == 0)
    {
      fprintf(stderr, "SIGUSR1 not received\n");
      exit(4);
    }

  sigusr1_rcvd = 0;

  /* Send SIGUSR2 to ourselves */

  printf("sigqueue-ing SIGUSR2 from pid=%d\n", mypid);
  fflush(stdout); usleep(SHORT_DELAY);

  /* Send SIGUSR2 to ourselves via sigqueue() */

  sigval.sival_int = 87;
  status = sigqueue(mypid, SIGUSR2, sigval);
  if (status != 0)
    {
      fprintf(stderr, "Failed to queue SIGUSR2, errno=%d\n", errno);
      exit(5);
    }

  usleep(SHORT_DELAY);
  printf("SIGUSR2 queued from pid=%d, sigval=87\n", mypid);

  /* Verify that SIGUSR2 was received */

  if (sigusr2_rcvd == 0)
    {
      fprintf(stderr, "SIGUSR2 not received\n");
      exit(6);
    }

  sigusr2_rcvd = 0;

  /* Remove the siguser_action handler and replace the SIGUSR2
   * handler with sigusr2_sighandler.
   */

  printf("Resetting SIGUSR2 signal handler from pid=%d\n", mypid);

  status = sigaction(SIGUSR2, &oact2, &act);
  if (status != 0)
    {
      fprintf(stderr, "Failed to install SIGUSR1 handler, errno=%d\n",
              errno);
      exit(2);
    }

  printf("Old SIGUSR1 sighandler at %p\n", act.sa_handler);
  printf("New SIGUSR1 sighandler at %p\n", oact1.sa_handler);

  /* Verify that the handler that was removed was siguser_action */

  if ((void*)act.sa_handler != (void*)siguser_action)
    {
      fprintf(stderr,
              "Old SIGUSR2 signal handler (%p) is not siguser_action (%p)\n",
              act.sa_handler, siguser_action);
      exit(8);
    }

  /* Send SIGUSR2 to ourselves via kill() */

  printf("Killing SIGUSR2 from pid=%d\n", mypid);
  fflush(stdout); usleep(SHORT_DELAY);

  status = kill(mypid, SIGUSR2);
  if (status != 0)
    {
      fprintf(stderr, "Failed to kill SIGUSR2, errno=%d\n", errno);
      exit(9);
    }

  usleep(SHORT_DELAY);
  printf("SIGUSR2 killed from pid=%d\n", mypid);

  /* Verify that SIGUSR2 was received */

  if (sigusr2_rcvd == 0)
    {
      fprintf(stderr, "SIGUSR2 not received\n");
      exit(10);
    }

  sigusr2_rcvd = 0;

  return 0;
}
