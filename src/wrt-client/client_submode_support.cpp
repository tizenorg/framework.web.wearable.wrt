/*
 * Copyright (c) 2013 Samsung Electronics Co., Ltd All Rights Reserved
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
 * @file       client_submode_support.cpp
 * @author     Jihoon Chung (jihoon.chung@samsung.com)
 * @version    1.0
 */

#include "client_submode_support.h"

#include <memory>
#include <sstream>
#include <Ecore.h>
#include <Elementary.h>
#include <bundle.h>

#include <dpl/assert.h>
#include <dpl/log/secure_log.h>
#include <dpl/wrt-dao-ro/widget_dao_read_only.h>

#include <application_data.h>

namespace ClientModule {
namespace {
const unsigned int EMPTY = 0;
const unsigned int INLINE_MODE = 1;
const unsigned int TRANSIENT_WINDOW = 1 << 1;
}

 //Implementation class
class SubmodeSupportImplementation
{
  private:
    bool m_initialized;
    WrtDB::TizenAppId m_appId;
    unsigned int m_mode;

    void setMode(const int mode)
    {
        m_mode |= mode;
    }

    bool getMode(const int mode)
    {
        return m_mode & mode;
    }

    static Eina_Bool destoryCallback(void* data, int /*type*/, void* event)
    {
        _D("called");
        Ecore_X_Window callerId = reinterpret_cast<Ecore_X_Window>(data);

        Assert(event);
        Ecore_X_Event_Window_Hide* ev =
            static_cast<Ecore_X_Event_Window_Hide*>(event);

        if(ev->win == callerId) {
            elm_exit();
        }
        return ECORE_CALLBACK_CANCEL;
    }

  public:
    SubmodeSupportImplementation() :
        m_initialized(false),
        m_mode(EMPTY)
    {
    }

    void initialize(WrtDB::TizenAppId appId)
    {
        _D("called");

        m_appId = appId;
        WrtDB::WidgetDAOReadOnly dao(m_appId);
        WrtDB::WidgetAppControlList widgetApplicationControlList;
        dao.getAppControlList(widgetApplicationControlList);
        FOREACH(it, widgetApplicationControlList) {
            if (it->disposition ==
                WrtDB::WidgetAppControl::Disposition::INLINE)
            {
                _D("disposition");
                setMode(INLINE_MODE);
            }
        }

        m_initialized = true;
    }

    void deinitialize(void)
    {
        _D("called");
        m_initialized = false;
    }

    bool isInlineMode(void)
    {
        return getMode(INLINE_MODE);
    }

    bool isNeedTerminateOnSuspend(void)
    {
        if (isInlineMode()) {
            return !getMode(TRANSIENT_WINDOW);
        }
        return false;
    }

    bool transientWindow(Ecore_X_Window calleeId)
    {
        _D("called");
        if (!m_initialized) {
            _E("not initialized");
            return false;
        }

        bundle* b = ApplicationDataSingleton::Instance().getBundle();
        if (!b) {
            _W("Service data is empty");
            return false;
        }
        const char* callerIdPtr = bundle_get_val(b, "__APP_SVC_K_WIN_ID__");
        if (callerIdPtr) {
            Ecore_X_Window callerId = atoi(callerIdPtr);
            _D("Caller x handle = %u", callerId);
            ecore_x_icccm_transient_for_set(calleeId, callerId);
            ecore_x_window_client_manage(callerId);
            ecore_event_handler_add(ECORE_X_EVENT_WINDOW_DESTROY,
                                    destoryCallback,
                                    reinterpret_cast<void*>(callerId));
            setMode(TRANSIENT_WINDOW);
        } else {
            _W("Service data is empty");
            return false;
        }

        return true;
    }
};

SubmodeSupport::SubmodeSupport() :
    m_impl(new SubmodeSupportImplementation())
{
}

SubmodeSupport::~SubmodeSupport()
{
}

void SubmodeSupport::initialize(WrtDB::TizenAppId appId)
{
    m_impl->initialize(appId);
}

void SubmodeSupport::deinitialize(void)
{
    m_impl->deinitialize();
}

bool SubmodeSupport::isInlineMode(void)
{
    return m_impl->isInlineMode();
}

bool SubmodeSupport::isNeedTerminateOnSuspend(void)
{
    return m_impl->isNeedTerminateOnSuspend();
}

bool SubmodeSupport::transientWindow(Ecore_X_Window calleeId)
{
    return m_impl->transientWindow(calleeId);
}
} // namespace ClientModule
