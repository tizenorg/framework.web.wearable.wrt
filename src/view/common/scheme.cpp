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
 * @file       scheme.cpp
 * @author     Krzysztof Jackiewicz (k.jackiewicz@samsung.com)
 * @version    1.0
 */

#include "scheme.h"
#include <map>
#include <dpl/log/log.h>
#include <dpl/platform.h>

namespace ViewModule {
namespace {
const char * const type2name[Scheme::COUNT] = {
    "file",
    "sms",
    "smsto",
    "mmsto",
    "mailto",
    "data",
    "tel",
    "http",
    "https",
    "widget",
#if ENABLE(APP_SCHEME)
    "app",
#endif
    "rtsp",
    "about"
};

typedef std::map<std::string, Scheme::Type> SchemeMap;

SchemeMap PopulateMap()
{
    LogDebug("Populating scheme map...");
    SchemeMap map;
    for (size_t st = Scheme::FILE; st < Scheme::COUNT; ++st) {
        LogDebug(" * " << type2name[st] << "->" << st);
        map[type2name[st]] = static_cast<Scheme::Type>(st);
    }
    return map;
}

const SchemeMap name2type = PopulateMap();
} // namespace

Scheme::Scheme(const std::string& name) : m_name(name), m_type(INVALID)
{
    m_type = GetType(name);
}

std::string Scheme::GetName (Type type)
{
    Assert(type >= FILE && type < COUNT);
    return type2name[type];
}

Scheme::Type Scheme::GetType(const std::string& name)
{
    auto it = name2type.find(name);
    if (it == name2type.end()) {
        LogError("Invalid scheme: " << name);
        return INVALID;
    }
    return it->second;
}
} /* namespace ViewModule */
