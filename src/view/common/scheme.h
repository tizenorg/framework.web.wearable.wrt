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
 * @file       scheme.h
 * @author     Krzysztof Jackiewicz (k.jackiewicz@samsung.com)
 * @version    1.0
 */

#ifndef SCHEME_H_
#define SCHEME_H_

#include <string>

#include <dpl/platform.h>

namespace ViewModule {
class Scheme
{
  public:
    enum Type {
        INVALID = -1,
        FILE = 0,
        SMS,
        SMSTO,
        MMSTO,
        MAILTO,
        DATA,
        TEL,
        HTTP,
        HTTPS,
        WIDGET,
#if ENABLE(APP_SCHEME)
        APP,
#endif
        RTSP,
        ABOUT,

        COUNT
    };

    explicit Scheme(const std::string& name);
    virtual ~Scheme() {}

    std::string GetName() const
    {
        return m_name;
    }
    Type GetType() const
    {
        return m_type;
    }

    static std::string GetName (Type type);
    static std::string GetName (size_t type)
    {
        return GetName(static_cast<Type>(type));
    }
    static Type GetType(const std::string& name);

  private:
    std::string m_name;
    Type m_type;
};
} /* namespace ViewModule */
#endif /* SCHEME_H_ */
