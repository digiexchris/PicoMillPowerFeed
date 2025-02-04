/****************************************************************************
 * apps/examples/camera/camera_bkgd.h
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

#ifndef __APPS_EXAMPLES_CAMERA_CAMERA_BKGD_H
#define __APPS_EXAMPLES_CAMERA_CAMERA_BKGD_H

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

/****************************************************************************
 * Public Function Prototypes
 ****************************************************************************/

int nximage_initialize(void);                      /* Initialize NX graphics subsystem */
void nximage_draw(FAR void *image, int w, int h);  /* Draw image onto Background window */
void nximage_finalize(void);                       /* Finalize NX graphics subsystem */

#endif /* __APPS_EXAMPLES_CAMERA_CAMERA_BKGD_H */
