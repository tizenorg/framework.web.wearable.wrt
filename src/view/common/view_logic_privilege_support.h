/*
 * Copyright (c) 2014 Samsung Electronics Co., Ltd All Rights Reserved
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
 * @file    view_logic_privilege_support.h
 * @author  Jihoon Chung (jihoon.chung@samsung.com)
 */

#ifndef VIEW_LOGIC_PRIVILEGE_SUPPORT_H_
#define VIEW_LOGIC_PRIVILEGE_SUPPORT_H_

#include <memory>

#include <dpl/optional_typedefs.h>

//Forward declaration
class WidgetModel;

namespace ViewModule {
//Forward declaration
class PrivilegeSupportImplementation;

class PrivilegeSupport
{
  public:
    enum class Privilege {
        CAMERA,       // http://tizen.org/privilege/camera
        LOCATION,     // http://tizen.org/privilege/location
        MEDIACAPTURE, // http://tizen.org/privilege/mediacapture
        RECORDER      // http://tizen.org/privilege/recorder
    };

    PrivilegeSupport(WidgetModel* model);
    virtual ~PrivilegeSupport();

    DPL::OptionalBool getPrivilegeStatus(Privilege pri);

  private:
    std::unique_ptr<ViewModule::PrivilegeSupportImplementation> m_impl;
};

} // namespace ViewModule

#endif // VIEW_LOGIC_PRIVILEGE_SUPPORT_H_
