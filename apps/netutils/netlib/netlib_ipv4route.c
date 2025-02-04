/****************************************************************************
 * apps/netutils/netlib/netlib_ipv4route.c
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

#include <nuttx/config.h>

#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <errno.h>

#include <arpa/inet.h>

#include "netutils/netlib.h"

#if defined(CONFIG_NET_IPv4) && defined(HAVE_ROUTE_PROCFS)

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* Determines the size of an intermediate buffer that must be large enough
 * to handle the longest line generated by this logic.
 */

#define PROCFS_LINELEN 58

/* The form of the entry from the routing table file:
 *
 *            11111111112222222222333333333344444444444555
 *   12345678901234567890123456789012345678901234567890123
 *   SEQ   TARGET          NETMASK         ROUTER
 *   nnnn. xxx.xxx.xxx.xxx xxx.xxx.xxx.xxx xxx.xxx.xxx.xxx
 */

#define TARGET_OFFSET  6
#define NETMASK_OFFSET 22
#define ROUTER_OFFSET  38

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Name: set_nul_terminator
 *
 * Description:
 *   Make sure that the string is NUL terminated.
 *
 ****************************************************************************/

static void set_nul_terminator(FAR char *str)
{
  /* The first non-decimal character that is not '.' terminates the
   * address
   */

  while ((*str >= '0' && *str <= '9') || *str == '.')
    {
      str++;
    }

  *str = '\0';
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: netlib_read_ipv4route
 *
 * Description:
 *   Read the next entry from the IPv4 routing table.
 *
 * Input Parameters:
 *   fd     - The open file descriptor to the procfs' IPv4 routing table.
 *   route  - The location to return that the next routing table entry.
 *
 * Returned Value:
 *   sizeof(struct netlib_ipv4_route_s) is returned on success.  Zero is
 *   returned if the end of file is encountered.  A negated errno value is
 *   returned on any failure.
 *
 ****************************************************************************/

ssize_t netlib_read_ipv4route(FILE *stream,
                              FAR struct netlib_ipv4_route_s *route)
{
  FAR char *prefix;
  FAR char *netmask;
  FAR char *router;
  char line[PROCFS_LINELEN];
  int ret;

  DEBUGASSERT(stream != NULL && route != NULL);

  /* The form of the entry read from the routing table:
   *
   *            11111111112222222222333333333344444444444555
   *   12345678901234567890123456789012345678901234567890123
   *   SEQ   TARGET          NETMASK         ROUTER
   *   nnnn. xxx.xxx.xxx.xxx xxx.xxx.xxx.xxx xxx.xxx.xxx.xxx
   */

  if (fgets(line, PROCFS_LINELEN, stream) == NULL)
    {
      /* End of file (or possibly a read error?) */

      return 0;
    }

  /* Special case the first line of the file */

  if (strncmp(line, "SEQ", 3) == 0)
    {
      /* Skip over the header and read the first real line of data */

      if (fgets(line, PROCFS_LINELEN, stream) == NULL)
        {
          /* End of file (or possibly a read error?) */

          return 0;
        }
    }

  /* Make certain that there is a NUL terminator */

  line[PROCFS_LINELEN - 1] = '\0';

  /* The format of the line we just read is very strict so we should be able
   * dispense with parsing and force things as follows:
   */

  prefix  = &line[TARGET_OFFSET];
  netmask = &line[NETMASK_OFFSET];
  router  = &line[ROUTER_OFFSET];

  /* Break up the strings in the line by adding NUL terminators */

  set_nul_terminator(prefix);
  set_nul_terminator(netmask);
  set_nul_terminator(router);

  /* Return the converted versions of the addresses */

  ret = inet_pton(AF_INET, prefix, &route->prefix);
  if (ret == 1)
    {
      ret = inet_pton(AF_INET, netmask, &route->netmask);
      if (ret == 1)
        {
          ret = inet_pton(AF_INET, router, &route->router);
          if (ret == 1)
            {
              return sizeof(struct netlib_ipv4_route_s);
            }
        }
    }

  return ret < 0 ? ret : -EINVAL;
}

#endif /* CONFIG_NET_IPv4 && HAVE_ROUTE_PROCFS */
