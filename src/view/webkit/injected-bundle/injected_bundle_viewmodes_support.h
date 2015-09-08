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
 * @file    injected_bundle_viewmodes_support.h
 * @author  Jihoon Chung (jihoon.chung@samsung.com)
 * @version 1.0
 */
#ifndef INJECTED_BUNDLE_VIEWMODES_SUPPORT_H_
#define INJECTED_BUNDLE_VIEWMODES_SUPPORT_H_

#include <memory>
#include <dpl/wrt-dao-ro/common_dao_types.h>

#include <WKBundlePage.h>

namespace InjectedBundle {
class ViewmodesSupportImplementation;

class ViewmodesSupport
{
  public:
    ViewmodesSupport(WrtDB::TizenAppId appId);
    virtual ~ViewmodesSupport();
    void initialize(WKBundlePageRef page);
    void deinitialize(WKBundlePageRef page);
    void enterViewmodesAllPages(const std::string& mode);
    void exitViewmodesAllPages(void);
  private:
    std::unique_ptr<ViewmodesSupportImplementation> m_impl;
};
} // namespace InjectedBundle

#endif // INJECTED_BUNDLE_VIEWMODES_SUPPORT_H_