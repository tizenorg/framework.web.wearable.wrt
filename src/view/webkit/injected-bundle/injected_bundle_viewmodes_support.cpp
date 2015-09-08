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
/*
 * @file    injected_bundle_viewmodes_support.cpp
 * @author  Jihoon Chung (jihoon.chung@samsung.com)
 * @version 1.0
 */

#include "injected_bundle_viewmodes_support.h"

#include <memory>
#include <map>
#include <set>
#include <string>

#include <dpl/log/secure_log.h>
#include <dpl/assert.h>
#include <dpl/string.h>
#include <dpl/wrt-dao-ro/widget_dao_read_only.h>

#include <WKString.h>
#include <WKBundlePage.h>
#include <WKBundlePagePrivate.h>

namespace InjectedBundle {
namespace {
const std::string VIEWMODE_TYPE_MAXIMIZED = "maximized";
const std::string VIEWMODE_TYPE_FULLSCREEN = "fullscreen";
const std::string VIEWMODE_TYPE_WINDOWED = "windowed";

typedef std::set<std::string> SupportViewmodesSet;
SupportViewmodesSet g_supportViewmodes = {VIEWMODE_TYPE_MAXIMIZED,
                                          VIEWMODE_TYPE_FULLSCREEN,
                                          VIEWMODE_TYPE_WINDOWED};
}

//Implementation class
class ViewmodesSupportImplementation
{
  private:
    typedef std::map<WKBundlePageRef, std::string> ViewmodesMap;
    typedef ViewmodesMap::iterator ViewmodesIt;

    bool m_initialized;

    WrtDB::TizenAppId m_appId;
    WrtDB::WindowModeList m_modeList;
    ViewmodesMap m_initialViewmodeMap;
    ViewmodesMap m_currentViewmodeMap;

    bool isExisted(WKBundlePageRef page)
    {
        ViewmodesIt viewmodeIt = m_initialViewmodeMap.find(page);
        if (viewmodeIt == m_initialViewmodeMap.end()) {
            return false;
        }
        return true;
    }

    std::string getChangedViewmode(void)
    {
        if (!m_currentViewmodeMap.empty()) {
            ViewmodesIt curIt = m_currentViewmodeMap.begin();
            ViewmodesIt initIt = m_initialViewmodeMap.begin();
            if (curIt->second != initIt->second) {
              return curIt->second;
            }
        }
        return std::string();
    }

    bool isSupportViewmode(const std::string& mode)
    {
        if (g_supportViewmodes.find(mode) == g_supportViewmodes.end()) {
            return false;
        }
        return true;
    }

  public:
    ViewmodesSupportImplementation(WrtDB::TizenAppId appId) :
        m_initialized(false),
        m_appId(appId)
    {
        WrtDB::WidgetDAOReadOnly dao(m_appId);
        m_modeList = dao.getWindowModes();

        m_initialized = true;
    }

    void initialize(WKBundlePageRef page)
    {
        _D("initialize");
        if (!m_initialized) {
            Assert(false);
        }

        if (isExisted(page)) {
            _W("This page is already initialized");
            return;
        }

        // set initial viewmode from manifest
        std::string initViewmode = VIEWMODE_TYPE_MAXIMIZED;
        FOREACH(it, m_modeList) {
            std::string mode = DPL::ToUTF8String(*it);
            if (g_supportViewmodes.find(mode) != g_supportViewmodes.end()) {
                initViewmode = mode;
            }
        }
        m_initialViewmodeMap[page] = initViewmode;

        // In case of current viewmode of chrome is changed,
        // set to changed viewmode
        std::string currentViewmode = getChangedViewmode();
        if (currentViewmode.empty()) {
            currentViewmode = initViewmode;
        }
        m_currentViewmodeMap[page] = currentViewmode;

        WKBundlePageSetViewMode(page,
                                WKStringCreateWithUTF8CString(
                                currentViewmode.c_str()));
    }

    void deinitialize(WKBundlePageRef page)
    {
        _D("deinitialize");
        if (!m_initialized) {
            Assert(false);
        }
        m_initialViewmodeMap.erase(page);
        m_currentViewmodeMap.erase(page);
    }

    void setViewmodes(WKBundlePageRef page, const std::string& mode)
    {
        if (!m_initialized) {
            Assert(false);
        }

        m_currentViewmodeMap[page] = mode;
        WKBundlePageSetViewMode(page,
                                WKStringCreateWithUTF8CString(
                                    mode.c_str()));
    }

    void enterViewmodesAllPages(const std::string& mode)
    {
        _D("setViewmodesAllPages");
        if (!m_initialized) {
            Assert(false);
        }
        if (!isSupportViewmode(mode)) {
            _W("Wrong viewmode : %s", mode.c_str());
            return;
        }

        FOREACH(it, m_currentViewmodeMap) {
            setViewmodes(it->first, mode);
        }
    }

    void exitViewmodes(WKBundlePageRef page)
    {
        if (!m_initialized) {
            Assert(false);
        }

        std::string mode = m_initialViewmodeMap[page];
        m_currentViewmodeMap[page] = mode;
        WKBundlePageSetViewMode(page,
                                WKStringCreateWithUTF8CString(
                                    mode.c_str()));
    }

    void exitViewmodesAllPages(void)
    {
        _D("exitViewmodesAllPages");
        if (!m_initialized) {
            Assert(false);
        }

        FOREACH(it, m_currentViewmodeMap) {
            exitViewmodes(it->first);
        }
    }
};

ViewmodesSupport::ViewmodesSupport(WrtDB::TizenAppId appId) :
    m_impl(new ViewmodesSupportImplementation(appId))
{
}

ViewmodesSupport::~ViewmodesSupport()
{
}

void ViewmodesSupport::initialize(WKBundlePageRef page)
{
    m_impl->initialize(page);
}

void ViewmodesSupport::deinitialize(WKBundlePageRef page)
{
    m_impl->deinitialize(page);
}

void ViewmodesSupport::enterViewmodesAllPages(const std::string& mode)
{
    m_impl->enterViewmodesAllPages(mode);
}

void ViewmodesSupport::exitViewmodesAllPages(void)
{
    m_impl->exitViewmodesAllPages();
}
}  // namespace InjectedBundle
