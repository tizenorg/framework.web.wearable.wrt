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
 * @file page_global_context_container.h
 * @author Tomasz Iwanek (t.iwanek@smasung.com)
 * @brief Declares container for global context that holds its references
 */
#ifndef PAGE_GLOBAL_CONTEXT_CONTAINER_H
#define PAGE_GLOBAL_CONTEXT_CONTAINER_H

#include <map>

#include <JavaScriptCore/JSContextRef.h>
#include <WKBundlePage.h>

/**
 * @brief The PageGlobalContextContainer class
 *
 * Container for global contexts that increments reference for holded elements
 */
class PageGlobalContextContainer
{
public:
    typedef std::map<WKBundlePageRef, JSGlobalContextRef> PageGlobalContext;
    typedef PageGlobalContext::iterator iterator;
    typedef PageGlobalContext::const_iterator const_iterator;
    typedef PageGlobalContext::value_type value_type;

    PageGlobalContextContainer();
    ~PageGlobalContextContainer();

    PageGlobalContextContainer(const PageGlobalContextContainer&) = delete;
    PageGlobalContextContainer& operator=(const PageGlobalContextContainer&) = delete;

    void insertContextForPage(WKBundlePageRef page, JSGlobalContextRef context);
    void removeContextForPage(WKBundlePageRef page);
    /**
     * @brief getContextForPage gets context for given page
     * @param page wk page
     * Page should be valid for container.
     * If you don't know if page is present in container use find
     *
     * @return context
     */
    JSGlobalContextRef getContextForPage(WKBundlePageRef page) const;

    const_iterator begin() const;
    const_iterator find(WKBundlePageRef ref) const;
    const_iterator end() const;
private:
    PageGlobalContext m_map;
};

#endif // PAGE_GLOBAL_CONTEXT_CONTAINER_H
