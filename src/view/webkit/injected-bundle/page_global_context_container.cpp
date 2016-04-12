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
/**
 * @file page_global_context_container.cpp
 * @author Tomasz Iwanek (t.iwanek@smasung.com)
 * @brief Declares container for global context that holds its references
 */
#include "page_global_context_container.h"

#include <dpl/foreach.h>

PageGlobalContextContainer::PageGlobalContextContainer()
{
}

PageGlobalContextContainer::~PageGlobalContextContainer()
{
    FOREACH(iter, m_map)
    {
        JSGlobalContextRelease(iter->second);
    }
}

void PageGlobalContextContainer::insertContextForPage(WKBundlePageRef page, JSGlobalContextRef context)
{
    PageGlobalContext::iterator iter = m_map.find(page);
    if(iter != m_map.end())
    {
        JSGlobalContextRelease(m_map[page]);
    }
    JSGlobalContextRetain(context);
    m_map[page] = context;
}

void PageGlobalContextContainer::removeContextForPage(WKBundlePageRef page)
{
    PageGlobalContext::iterator iter = m_map.find(page);
    if(iter != m_map.end())
    {
        JSGlobalContextRelease(m_map[page]);
        m_map.erase(iter);
    }
}

JSGlobalContextRef PageGlobalContextContainer::getContextForPage(WKBundlePageRef page) const
{
    return m_map.find(page)->second;
}

PageGlobalContextContainer::const_iterator PageGlobalContextContainer::begin() const
{
    return m_map.begin();
}
PageGlobalContextContainer::const_iterator PageGlobalContextContainer::find(WKBundlePageRef ref) const
{
    return m_map.find(ref);
}
PageGlobalContextContainer::const_iterator PageGlobalContextContainer::end() const
{
    return m_map.end();
}
