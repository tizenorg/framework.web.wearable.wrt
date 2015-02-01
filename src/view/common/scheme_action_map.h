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
/*
 * @file       scheme_action_map.h
 * @author     Krzysztof Jackiewicz (k.jackiewicz@samsung.com)
 * @version    1.0
 */

#ifndef SCHEME_ACTION_MAP_H_
#define SCHEME_ACTION_MAP_H_

#include "scheme_action_map_type.h"

namespace ViewModule {
namespace SchemeActionMap {
bool HandleUri(const char* uri,
               NavigationContext context);
};
} /* namespace ViewModule */
#endif /* SCHEME_ACTION_MAP_H_ */
