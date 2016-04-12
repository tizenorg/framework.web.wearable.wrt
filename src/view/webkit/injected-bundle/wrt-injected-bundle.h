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
 * @file wrt-injected-bundle.h
 * @author Jihoon Chung (jihoon.chung@samsung.com)
 * @brief declare injected bundle
 */
#ifndef WRT_SRC_VIEW_WEBKIT_WRT_INJECTED_BUNDLE_H_
#define WRT_SRC_VIEW_WEBKIT_WRT_INJECTED_BUNDLE_H_

#include <map>
#include <memory>
#include <set>
#include <list>
#include <string>
#include <WKBundle.h>
#include <WKPageLoadTypes.h>
#include <WKBundlePage.h>
#include <dpl/platform.h>
#include <dpl/string.h>
#include <dpl/wrt-dao-ro/widget_dao_read_only.h>
#include <dpl/wrt-dao-ro/common_dao_types.h>
#include "page_global_context_container.h"

extern "C" {
WK_EXPORT void WKBundleInitialize(WKBundleRef bundle, WKTypeRef);
}

namespace InjectedBundle {
class DecryptionSupport;
class ViewmodesSupport;
}

class Bundle
{
  public:
    Bundle(WKBundleRef bundle);
    ~Bundle();

    Bundle(const Bundle&) = delete;
    Bundle& operator=(const Bundle&) = delete;

    // WKBundleClient callback
    static void didCreatePageCallback(
        WKBundleRef bundle,
        WKBundlePageRef page,
        const void* clientInfo);
    static void willDestroyPageCallback(
        WKBundleRef bundle,
        WKBundlePageRef page,
        const void* clientInfo);
    static void didReceiveMessageCallback(
        WKBundleRef,
        WKStringRef messageName,
        WKTypeRef messageBody,
        const void *clientInfo);

  private:
    WKBundleRef m_bundle;

    typedef std::list<WKBundlePageRef> PagesList;
    PagesList m_pagesList;
    PageGlobalContextContainer m_pageGlobalContext;
    DPL::String m_widgetTizenId;
    double m_scale;
    std::string m_encodedBundle;
    std::string m_theme;
    std::set<JSGlobalContextRef> m_loadedContext;
    JSGlobalContextRef m_willRemoveContext;
    WrtDB::WidgetSecurityModelVersion m_securityModelVersion;
    bool m_initialized;

    std::unique_ptr<InjectedBundle::DecryptionSupport> m_decryptionSupport;
    std::unique_ptr<InjectedBundle::ViewmodesSupport> m_viewmodesSupport;

    // WKBundlePageResourceLoadClient callback
    static WKURLRequestRef willSendRequestForFrameCallback(
        WKBundlePageRef,
        WKBundleFrameRef,
        uint64_t resourceIdentifier,
        WKURLRequestRef request,
        WKURLResponseRef,
        const void *clientInfo);
    static void didFinishLoadForResourceCallback(
        WKBundlePageRef page,
        WKBundleFrameRef frame,
        uint64_t resourceIdentifier,
        const void* clientInfo);

    //WKBundlePageGlobalObjectIsAvailableForFrameCallback callback
    static void globalObjectIsAvailableForFrameCallback(
        WKBundlePageRef page,
        WKBundleFrameRef frame,
        WKBundleScriptWorldRef script,
        const void* clientInfo);

    //WKBundlePageWillDisconnectDOMWindowExtensionFromGlobalObjectCallback callback
    static void willDisconnectDOMWindowExtensionFromGlobalObjectCallback(
        WKBundlePageRef page,
        WKBundleDOMWindowExtensionRef extension,
        const void* clientInfo);

    //WKBundlePageDidReconnectDOMWindowExtensionToGlobalObjectCallback callback
    static void didReconnectDOMWindowExtensionToGlobalObjectCallback(
        WKBundlePageRef page,
        WKBundleDOMWindowExtensionRef extension,
        const void* clientInfo);

    //WKBundlePageWillDestroyGlobalObjectForDOMWindowExtensionCallback callback
    static void willDestroyGlobalObjectForDOMWindowExtensionCallback(
        WKBundlePageRef page,
        WKBundleDOMWindowExtensionRef extension,
        const void* clientInfo);

    // WKBundlePageDecidePolicyForNavigationActionCallback
    static WKBundlePagePolicyAction decidePolicyForNavigationActionCallback(
        WKBundlePageRef page,
        WKBundleFrameRef frame,
        WKBundleNavigationActionRef navigationAction,
        WKURLRequestRef request,
        WKTypeRef* userData,
        const void* clientInfo);

    // WKBundlePageDecidePolicyForNewWindowActionCallback
    static WKBundlePagePolicyAction decidePolicyForNewWindowActionCallback(
        WKBundlePageRef page,
        WKBundleFrameRef frame,
        WKBundleNavigationActionRef navigationAction,
        WKURLRequestRef request,
        WKStringRef frameName,
        WKTypeRef* userData,
        const void* clientInfo);

    // WKBundlePageDecidePolicyForResponseCallback
    static WKBundlePagePolicyAction decidePolicyForResponseCallback(
        WKBundlePageRef page,
        WKBundleFrameRef frame,
        WKURLResponseRef response,
        WKURLRequestRef request,
        WKTypeRef* userData,
        const void* clientInfo);

    // WKBundleClient
    void didCreatePage(WKBundlePageRef page);
    void willDestroyPage(WKBundlePageRef page);
    void didReceiveMessage(
        WKStringRef messageName,
        WKTypeRef messageBody);

    // WKBundlePageResourceLoadClient
    WKURLRequestRef willSendRequestForFrame(WKURLRequestRef request);
    WKBundlePagePolicyAction decidePolicyForAction(
        bool isNewWindow,
        WKBundlePageRef page,
        WKBundleFrameRef frame,
        WKBundleNavigationActionRef navigationAction,
        WKURLRequestRef request,
        WKTypeRef* userData);

    // basic
    inline static std::string toString(WKStringRef str);
    inline static std::string toString(WKURLRef url);
    inline static std::string toString(WKURLRequestRef req);
    inline static std::string toString(WKErrorRef err);
    static std::string getScheme(std::string uri);

    bool isEncryptedResource(std::string Url, int &size);
    std::string DecryptResource(std::string resource, int size);

    void fixWKMessageArgs(std::string & argScale,
                          std::string & argEncodedBundle,
                          std::string & argTheme);

#if ENABLE(CORS_WHITELISTING)
    void bypassCORSforWARPAccessList(WrtDB::WidgetDAOReadOnly &dao);
#endif
};

#endif // WRT_SRC_VIEW_WEBKIT_WRT_INJECTED_BUNDLE_H_
