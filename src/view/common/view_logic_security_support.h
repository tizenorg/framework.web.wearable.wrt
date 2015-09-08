/*
 * Copyright (c) 2011 Samsung Electronics Co., Ltd All Rights Reserved
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
 * @file    view_logic_security_support.h
 * @author  Pawel Sikorski (p.sikorski@samsung.com)
 * @brief   Header file of SecuritySupport API used by ViewLogic
 */

#ifndef VIEW_LOGIC_SECURITY_SUPPORT_H_
#define VIEW_LOGIC_SECURITY_SUPPORT_H_

#include <string>
#include <dpl/string.h>

//Forward declaration
class WidgetModel;

namespace ViewModule {
namespace SecuritySupport {
char const * const PARAM_URL = "param:url";
bool geolocationACECheck(const DPL::String& tizenId, bool highAccuracy);
} // namespace SecuritySupport
} // namespace ViewModule

#endif /* VIEW_LOGIC_SECURITY_SUPPORT_H_ */
