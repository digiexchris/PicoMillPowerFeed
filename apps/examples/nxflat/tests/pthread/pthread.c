/****************************************************************************
 * apps/examples/nxflat/tests/pthread/pthread.c
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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

#define CHILD_ARG ((void*)0x12345678)
#define CHILD_RET ((void*)0x87654321)

/****************************************************************************
 * Private Types
 ****************************************************************************/

enum exit_values_e
{
  TESTRESULT_SUCCESS = 0,
  TESTRESULT_PTHREAD_ATTR_INIT_FAIL,
  TESTRESULT_PTHREAD_CREATE_FAIL,
  TESTRESULT_PTHREAD_JOIN_FAIL,
  TESTRESULT_CHILD_ARG_FAIL,
  TESTRESULT_CHILD_RETVAL_FAIL,
};

/****************************************************************************
 * External Functions
 ****************************************************************************/

/****************************************************************************
 * Private Data
 ****************************************************************************/

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/* NOTE: it is necessary for functions that are referred to by function pointers
 * pointer to be declared with global scope (at least for ARM).  Otherwise,
 * a relocation type that is not supported by NXFLAT is generated by GCC.
 */

void *child_start_routine(void *arg)
{
  printf("CHILD: started with arg=%d\n", (int)arg);

  if (arg != CHILD_ARG)
    {
      printf("CHILD: expected arg=%d\n", (int)CHILD_ARG);
      return (void*)TESTRESULT_CHILD_ARG_FAIL;
    }
  sleep(2);

  printf("CHILD: returning %d\n", (int)CHILD_RET);
  pthread_exit(CHILD_RET);
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

int main(int argc, char **argv)
{
  pthread_attr_t attr;
  pthread_t      thread;
  void          *retval;
  int            status;

  puts("PARENT: started\n");

  status = pthread_attr_init(&attr);
  if (status != 0)
    {
      printf("PARENT: pthread_attr_init() returned %d\n", status);
      exit(TESTRESULT_PTHREAD_ATTR_INIT_FAIL);
    }

  printf("PARENT: calling pthread_start with arg=%d\n", (int)CHILD_ARG);
  status = pthread_create(&thread, &attr, child_start_routine, CHILD_ARG);
  if (status != 0)
    {
      printf("PARENT: pthread_create() returned %d\n", status);
      exit(TESTRESULT_PTHREAD_CREATE_FAIL);
    }

  status = pthread_join(thread, &retval);
  if (status != 0)
    {
      printf("PARENT pthread_join() returned %d\n", status);

      exit(TESTRESULT_PTHREAD_JOIN_FAIL);
    }

  printf("PARENT child exitted with %d\n", (int)retval);
  if (retval != CHILD_RET)
    {
      printf("PARENT child thread did not exit with %d\n", (int)CHILD_RET);
      exit(TESTRESULT_CHILD_RETVAL_FAIL);
    }

  puts("PARENT returning success\n");
  return TESTRESULT_SUCCESS;
}
