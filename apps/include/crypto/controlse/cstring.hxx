//***************************************************************************
// apps/include/crypto/controlse/cstring.hxx
//
// SPDX-License-Identifier: Apache-2.0
// SPDX-FileCopyrightText: 2024 NXP
//
// Licensed to the Apache Software Foundation (ASF) under one or more
// contributor license agreements.  See the NOTICE file distributed with
// this work for additional information regarding copyright ownership.  The
// ASF licenses this file to you under the Apache License, Version 2.0 (the
// "License"); you may not use this file except in compliance with the
// License.  You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
// WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
// License for the specific language governing permissions and limitations
// under the License.
//
//**************************************************************************

#pragma once

//***************************************************************************
// Included Files
//***************************************************************************

#include "crypto/controlse/isecure_element_object.hxx"
#include <stddef.h>

namespace Controlse
{

//***************************************************************************
// Class definitions
//***************************************************************************

class CString : public ISecureElementObject
{
public:
  CString(const ISecureElement &se, uint32_t keystore_id);
  CString(char *string, size_t size);
  CString(const CString &p1);
  ~CString();

  CString &operator=(const CString &other);
  bool operator==(CString &a) const;
  bool operator!=(CString &a) const;

  // return value is string otherwise nullptr
  // NOTE: need to delete[] return value
  char *c_str(void) const;

  bool IsLoaded() const;
  bool StoreOnSecureElement(const ISecureElement &se,
                            uint32_t keystore_id) const;
  bool LoadFromSecureElement(const ISecureElement &se, uint32_t keystore_id);

private:
  size_t m_size = 0;
  char *m_string = nullptr;
};
} // namespace Controlse
