/****************************************************************************
 * apps/system/nxcodec/nxcodec.h
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

#ifndef __APPS_SYSTEM_NXCODEC_NXCODEC_H
#define __APPS_SYSTEM_NXCODEC_NXCODEC_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include "nxcodec_context.h"

/****************************************************************************
 * Public Types
 ****************************************************************************/

typedef struct nxcodec_s
{
  char              devname[PATH_MAX];
  int               fd;
  nxcodec_context_t capture;
  nxcodec_context_t output;
} nxcodec_t;

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

int nxcodec_init(FAR nxcodec_t *codec);
int nxcodec_start(FAR nxcodec_t *codec);
int nxcodec_stop(FAR nxcodec_t *codec);
int nxcodec_uninit(FAR nxcodec_t *codec);

#endif /* __APPS_SYSTEM_NXCODEC_NXCODEC_H */
