/*
 * Copyright (c) 2012 Samsung Electronics Co., Ltd All Rights Reserved
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */
/**
 * @file    injected_bundle_decryption_support.h
 * @author  Jihoon Chung (jihoon.chung@samsung.com)
 * @version 1.0
 */
#ifndef INJECTED_BUNDLE_DECRYPTION_SUPPORT_H_
#define INJECTED_BUNDLE_DECRYPTION_SUPPORT_H_

#include <memory>
#include <string>
#include <dpl/wrt-dao-ro/common_dao_types.h>

namespace InjectedBundle {
class DecryptionSupportImplementation;

class DecryptionSupport
{
  public:
    DecryptionSupport();
    virtual ~DecryptionSupport();
    void initialize(WrtDB::TizenAppId appId);
    void deinitialize(void);
    bool isNeedDecryption(std::string url);
    std::string decryptResource(std::string url);

  private:
    std::unique_ptr<DecryptionSupportImplementation> m_impl;
};
} // namespace InjectedBundle

#endif // INJECTED_BUNDLE_DECRYPTION_SUPPORT_H_
