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
 * @file    injected_bundle_uri_handling.h
 * @author  Marcin Kaminski (marcin.ka@samsung.com)
 * @version 1.0
 */
#ifndef INJECTED_BUNDLE_URI_HANDLING_H_
#define INJECTED_BUNDLE_URI_HANDLING_H_

#include <dpl/string.h>
#include <dpl/optional_typedefs.h>
#include <dpl/wrt-dao-ro/common_dao_types.h>

namespace InjectedBundleURIHandling {
bool processURI(const DPL::String& inputURI,
                const DPL::String& tizenId,
                WrtDB::WidgetSecurityModelVersion m_securityModelVersion);
bool processURI(const std::string& inputURI,
                const DPL::String& tizenId,
                WrtDB::WidgetSecurityModelVersion version);
bool processMainResource(const DPL::String& inputURI,
                const DPL::String& tizenId,
                WrtDB::WidgetSecurityModelVersion m_securityModelVersion);
bool processURIForPlugin(const char* url);
DPL::OptionalString localizeURI(const DPL::String& inputURI,
                                const DPL::String& tizenId);
std::string localizeURI(const std::string& inputURI, const std::string& tizenId);
}

#endif // INJECTED_BUNDLE_URI_HANDLING_H_
