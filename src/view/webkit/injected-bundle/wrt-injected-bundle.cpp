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
 * @file    wrt-injected-bundle.cpp
 * @author  Lukasz Wrzosek (l.wrzosek@samsung.com)
 * @brief   Implementation file for injected bundle
 */
#include "wrt-injected-bundle.h"

#include <WKBundle.h>
#include <WKBundleInitialize.h>
#include <WKBundlePage.h>
#include <WKBundleFrame.h>
#include <WKURLRequest.h>
#include <WKString.h>
#include <WKType.h>
#include <WKURL.h>
#include <WKError.h>
#include <WKURLResponseTizen.h>
#include <WKBundlePagePrivate.h>
#include <WKBundlePrivate.h>

#include <string>
#include <cstdio>
#include <sstream>
#include <set>
#include <memory>

#include <dpl/foreach.h>
#include <dpl/assert.h>
#include <dpl/wrt-dao-ro/WrtDatabase.h>
#include <dpl/localization/localization_utils.h>
#include <dpl/string.h>
#include <dpl/wrt-dao-ro/global_config.h>
#include <dpl/wrt-dao-ro/widget_dao_read_only.h>
#include <dpl/wrt-dao-ro/common_dao_types.h>
#include <dpl/localization/LanguageTagsProvider.h>
#include <dpl/event/main_event_dispatcher.h>
#include <dpl/platform.h>
#include <dpl/log/secure_log.h>
#include <wrt_plugin_module.h>
#include <profiling_util.h>

#include <appcore-efl.h>

#include <message_support.h>

#include <scheme.h>
#include <scheme_action_map_type.h>
#include <scheme_action_map_data.h>

#include <js_overlay_types.h>
#include <dispatch_event_support.h>
#include <plugins-ipc-message/ipc_message_support.h>

#include <sys/prctl.h>
#include <sys/resource.h>
#include <privilege-control.h>
#include <smack_labeling_support.h>
#include <vconf.h>

// URI localization on WebProcess side
#include "injected_bundle_uri_handling.h"
#include "injected_bundle_decryption_support.h"
#include "injected_bundle_viewmodes_support.h"

#include <resourced.h>

namespace {
const char SCHEME_HTTP[] = "http";
const char SCHEME_HTTPS[] = "https";
const char SCHEME_FILE[] = "file";
const char SCHEME_FILE_SLASH[] = "file://";
const char SCHEME_BOX_SLASH[] = "box://";
const char BLANK_PAGE_URL[] = "about:blank";
const char SRC_DOC_PAGE_URL[] = "about:srcdoc";
const char HTML_MIME[] = "text/html";
const char PHP_MIME[] = "application/x-php";
const char WRT_WILL_SEND_REQUEST_LOG_ENABLE[] = "WRT_WILL_SEND_REQUEST_LOG_ENABLE";
const std::size_t FILE_BUF_MAX_SIZE = 1024; // bytes
const std::size_t PLAIN_CHUNK_SIZE = 1008; // bytes
const unsigned int UID_ROOT = 0;
const unsigned int DEFAULT_PRIORITY = 0;
const char PRIVILEGE_APP_TYPE[] = "wgt";
#if ENABLE(CORS_WHITELISTING)
const char * const warpAllowProtocolsForWildcard[] = { "http", "https" };
#endif
}

Bundle::Bundle(WKBundleRef bundle) :
    m_bundle(bundle),
    m_scale(0),
    m_encodedBundle(""),
    m_theme(""),
    m_willRemoveContext(NULL),
    m_securityModelVersion(
        WrtDB::WidgetSecurityModelVersion::WIDGET_SECURITY_MODEL_V1),
    m_initialized(false),
    m_decryptionSupport(new InjectedBundle::DecryptionSupport())
{
    Try {
        LOG_PROFILE_START("Bundle attachToThread");
        WrtDB::WrtDatabase::attachToThreadRO();
        LOG_PROFILE_STOP("Bundle attachToThread");
    } Catch (DPL::DB::SqlConnection::Exception::Base) {
        _E("## Db attach was failed! Terminate WebProcess by force. ##");
        exit(-1);
    }
}

Bundle::~Bundle()
{
    WrtDB::WrtDatabase::detachFromThread();

    if (!m_pagesList.empty()) {
        _E("There are not closed pages!");
    }
    WKRelease(m_bundle);
}

void Bundle::didCreatePageCallback(
    WKBundleRef /*bundle*/,
    WKBundlePageRef page,
    const void* clientInfo)
{
    LOG_PROFILE_START("didCreatePageCallback");
    Bundle* This = static_cast<Bundle*>(const_cast<void*>(clientInfo));
    This->didCreatePage(page);
    LOG_PROFILE_STOP("didCreatePageCallback");
}

void Bundle::didReceiveMessageCallback(
    WKBundleRef /*bundle*/,
    WKStringRef messageName,
    WKTypeRef messageBody,
    const void *clientInfo)
{
    Bundle* bundle = static_cast<Bundle*>(const_cast<void*>(clientInfo));
    bundle->didReceiveMessage(messageName, messageBody);
}

void Bundle::willDestroyPageCallback(
    WKBundleRef /*bundle*/,
    WKBundlePageRef page,
    const void* clientInfo)
{
    Bundle* This = static_cast<Bundle*>(const_cast<void*>(clientInfo));
    This->willDestroyPage(page);
}

void Bundle::didCreatePage(WKBundlePageRef page)
{
    if (!m_initialized)
    {
        _E("## Injected-bundle was not initialized! Terminate WebProcess by force. ##");
        exit(-1);
    }

    auto mainFrame = WKBundlePageGetMainFrame(page);
    auto context = WKBundleFrameGetJavaScriptContext(mainFrame);
    m_pagesList.push_back(page);
    m_pageGlobalContext.insertContextForPage(page, context);
    _D("created Page : %p created JSContext : %p", page, context);
    m_viewmodesSupport->initialize(page);

    WKBundlePageResourceLoadClient resourceLoadClient = {
        kWKBundlePageResourceLoadClientCurrentVersion,  /* version */
        this, /* clientinfo */
        0, /* didInitiateLoadForResource */
        willSendRequestForFrameCallback, /* willSendRequestForFrame */
        0, /* didReceiveResponseForResource */
        0, /* didReceiveContentLengthForResource */
        0, /* didFinishLoadForResource */
        0, /* didFailLoadForResource */
        0, /* shouldCacheResponse */
        0, /* shouldUseCredentialStorage */
    };
    WKBundlePageSetResourceLoadClient(page, &resourceLoadClient);

    WKBundlePageLoaderClient loaderClient = {
        kWKBundlePageLoaderClientCurrentVersion,
        this, /* clientinfo */
        didStartProvisionalLoadForFrameCallback, /* didStartProvisionalLoadForFrame */
        0, /* didReceiveServerRedirectForProvisionalLoadForFrame */
        0, /* didFailProvisionalLoadWithErrorForFrame */
        didCommitLoadForFrameCallback, /* didCommitLoadForFrame */
        0, /* didFinishDocumentLoadForFrame */
        0, /* didFinishLoadForFrame */
        0, /* didFailLoadWithErrorForFrame */
        0, /* didSameDocumentNavigationForFrame */
        0, /* didReceiveTitleForFrame */
        0, /* didFirstLayoutForFrame */
        0, /* didFirstVisuallyNonEmptyLayoutForFrame */
        didRemoveFrameFromHierarchyCallback, /* didRemoveFrameFromHierarchy */
        0, /* didDisplayInsecureContentForFrame */
        0, /* didRunInsecureContentForFrame */
        0, /* didClearWindowObjectForFrame */
        0, /* didCancelClientRedirectForFrame */
        0, /* willPerformClientRedirectForFrame */
        0, /* didHandleOnloadEventsForFrame */
        0, /* didLayoutForFrame */
        0, /* didNewFirstVisuallyNonEmptyLayout */
        0, /* didDetectXSSForFrame */
        0, /* shouldGoToBackForwardListItem */
        0, /* globalObjectIsAvailableForFrame */
        0, /* willDisconnectDOMWindowExtensionFromGlobalObject */
        0, /* didReconnectDOMWindowExtensionToGlobalObject */
        0, /* willDestroyGlobalObjectForDOMWindowExtension */
        0, /* didFinishProgress */
        0, /* shouldForceUniversalAccessFromLocalURL */
        0, /* didReceiveIntentForFrame */
        0, /* registerIntentServiceForFrame */
    };
    WKBundlePageSetPageLoaderClient(page, &loaderClient);


    WKBundlePagePolicyClient policyClient = {
        kWKBundlePagePolicyClientCurrentVersion, /* version */
        this,                                    /* clientInfo */
        decidePolicyForNavigationActionCallback, /* decidePolicyForNavigationAction */
        decidePolicyForNewWindowActionCallback,  /* decidePolicyForNavigationAction */
        decidePolicyForResponseCallback,         /* decidePolicyForResponse */
        0,                                       /* unableToImplementPolicy */
    };
    WKBundlePageSetPolicyClient(page, &policyClient);
}

void Bundle::willDestroyPage(WKBundlePageRef page)
{
    _D("Destroyed page : %p", page);

    auto context = m_pageGlobalContext.getContextForPage(page);
    m_pagesList.remove(page);
    m_pageGlobalContext.removeContextForPage(page);
    m_pageContext[page].erase(context);
    m_viewmodesSupport->deinitialize(page);

    PluginModule::unloadFrame(context);
    PluginModule::stop(context);
}

void Bundle::fixWKMessageArgs(std::string & argScale,
                              std::string & argEncodedBundle,
                              std::string & argTheme)
{
    if (argScale != "null" && argScale[0] == '_') {
        argScale.erase(0, 1);

        std::stringstream ssScale(argScale);
        ssScale >> m_scale;
    }

    if (argEncodedBundle != "null" && argEncodedBundle[0] == '_') {
        argEncodedBundle.erase(0, 1);

        m_encodedBundle = argEncodedBundle;
    }

    if (argTheme != "null" && argTheme[0] == '_') {
        argTheme.erase(0, 1);

        m_theme = argTheme;
    }
}

#if ENABLE(CORS_WHITELISTING)
void Bundle::bypassCORSforWARPAccessList(WrtDB::WidgetDAOReadOnly & dao)
{
    // bypassing CORS using origin whitelist
    WrtDB::WidgetAccessInfoList WAList;
    dao.getWidgetAccessInfo(WAList);
    FOREACH(it, WAList)
    {
        const WrtDB::WidgetAccessInfo & access = *it;
        WKURLRef url = WKURLCreateWithUTF8CString(DPL::ToUTF8String(access.strIRI).c_str());

#if ENABLE(APP_SCHEME)
        std::string source = std::string("app://") + DPL::ToUTF8String(m_widgetTizenId) + "/";
#else
        std::string source = DPL::ToUTF8String(dao.getFullPath());
#endif

        _D("WARP to WK whitelist position: %s for %s subDomains: %d",
           source.c_str(),
           DPL::ToUTF8String(access.strIRI).c_str(),
           access.bSubDomains);

        WKStringRef wkSource = WKStringCreateWithUTF8CString(source.c_str());
        WKStringRef wkHost;
        WKStringRef wkProtocol;
        if(access.strIRI == L"*")
        {
            //wildcard force to explicitly say which protocol is used
            // passed wkHost if empty means wildcard -> allow everything but protocol has to be set.
            for(unsigned i = 0; i < sizeof(warpAllowProtocolsForWildcard) / sizeof(char*); i++)
            {
                wkHost = WKStringCreateWithUTF8CString("");
                wkProtocol = WKStringCreateWithUTF8CString(warpAllowProtocolsForWildcard[i]);
                WKBundleAddOriginAccessWhitelistEntry(m_bundle,
                    wkSource, wkProtocol, wkHost, access.bSubDomains);
            }
        }
        else
        {
            wkHost = WKURLCopyHostName(url);
            wkProtocol = WKURLCopyScheme(url);
            WKBundleAddOriginAccessWhitelistEntry(m_bundle,
                wkSource, wkProtocol, wkHost, access.bSubDomains);
        }

        WKRelease(wkHost);
        WKRelease(wkProtocol);
        WKRelease(wkSource);
    }
}
#endif

void Bundle::didReceiveMessage(WKStringRef messageName, WKTypeRef messageBody)
{
    _D("message name: %s", toString(messageName).c_str());
    if (WKStringIsEqualToUTF8CString(messageName,
                                     Message::ToInjectedBundle::START))
    {
        if (!messageBody || WKStringGetTypeID() != WKGetTypeID(messageBody)) {
            _E("Wrong message format received, ignoring");
            return;
        }

        std::string msgString =
            toString(static_cast<WKStringRef>(messageBody));
        _D("message body: %s", msgString.c_str());
        // set information from ui process
        std::stringstream ssMsg(msgString);
        std::string argScale;
        std::string argEncodedBundle;
        std::string argTheme;

        std::string id;
        ssMsg >> id;
        m_widgetTizenId = DPL::FromASCIIString(id);

        ssMsg >> argScale;
        ssMsg >> argEncodedBundle;
        ssMsg >> argTheme;

        // ** Language tags setting completed **
        fixWKMessageArgs(argScale, argEncodedBundle, argTheme);
    } else if (WKStringIsEqualToUTF8CString(messageName,
                                            Message::ToInjectedBundle::SHUTDOWN))
    {
        if (m_pagesList.empty()) {
            _D("shutdown plugins");
            PluginModule::shutdown();
        } else {
            _D("PluginModule shutdown ignored, there are still alive pages!");
        }
    }
    else if (WKStringIsEqualToUTF8CString(messageName,
                                          Message::ToInjectedBundle::SET_CUSTOM_PROPERTIES))
    {
        // set information from ui process
        std::string msgString =
            toString(static_cast<WKStringRef>(messageBody));
        _D("message body: %s", msgString.c_str());
        std::string argScale;
        std::string argEncodedBundle;
        std::string argTheme;

        std::stringstream ssMsg(msgString);
        ssMsg >> argScale;
        ssMsg >> argEncodedBundle;
        ssMsg >> argTheme;

        fixWKMessageArgs(argScale, argEncodedBundle, argTheme);

        //apply for each context
        PageGlobalContextContainer::const_iterator it = m_pageGlobalContext.begin();
        for (; it != m_pageGlobalContext.end(); ++it) {
            PluginModule::setCustomProperties(it->second,
                                              m_scale,
                                              m_encodedBundle.c_str(),
                                              m_theme.c_str());
        }
    } else if (WKStringIsEqualToUTF8CString(
                   messageName,
                   Message::ToInjectedBundle::DISPATCH_JS_EVENT))
    {
        _D("dispatch javascript event to created frames");
        using namespace WrtPlugins::W3C;

        // set information from ui process
        std::string text = toString(static_cast<WKStringRef>(messageBody));
        int eventType;
        SoftKeyboardChangeArgs softKeyboardArgs;

        std::stringstream ss(text);
        ss >> eventType;

        if (eventType == SoftKeyboardChangeCustomEvent)
        {
            ss >> softKeyboardArgs.state;
            ss >> softKeyboardArgs.width;
            ss >> softKeyboardArgs.height;
        }

        //apply for each context
        PageGlobalContextContainer::const_iterator it = m_pageGlobalContext.begin();

        for (; it != m_pageGlobalContext.end(); ++it)
        {
            if (eventType == SoftKeyboardChangeCustomEvent)
            {
                DispatchEventSupport::dispatchSoftKeyboardChangeEvent(it->second,
                                                    softKeyboardArgs.state,
                                                    softKeyboardArgs.width,
                                                    softKeyboardArgs.height);
            }
        }
    } else if (WKStringIsEqualToUTF8CString(
                   messageName,
                   Message::ToInjectedBundle::INIT))
    {
        if (!m_initialized) {
            _D("initialize");
            std::string msgString = toString(static_cast<WKStringRef>(messageBody));
            m_widgetTizenId = DPL::FromASCIIString(msgString);
            WrtDB::WidgetDAOReadOnly dao(m_widgetTizenId);

            // process pool - set app_privilige
            //for only one time run
            static bool first = true;
            if (UID_ROOT == getuid() && first) {
                first = false;

                using namespace WrtDB::GlobalConfig;

                std::string appPath;
                std::string tzAppId = DPL::ToUTF8String(dao.getTizenAppId());
                std::string tzPkgId = DPL::ToUTF8String(dao.getTizenPkgId());
                DPL::OptionalString installedPath = dao.getWidgetInstalledPath();
                if (!installedPath) {
                    appPath = std::string(GetUserInstalledWidgetPath()) + "/" +
                        tzPkgId + GetUserWidgetExecPath() + "/" + tzAppId;
                } else {
                    appPath = DPL::ToUTF8String(*installedPath) +
                        GetUserWidgetExecPath() + "/" + tzAppId;
                }

                _D("set_app_smack_label(%s)", appPath.c_str());
                if (set_app_smack_label(appPath.c_str()) != 0) {
                    _E("set_app_smack_label() failed");
                }

                _D("perm_app_set_privilege(%s)", appPath.c_str());
                perm_app_set_privilege(tzPkgId.c_str(), PRIVILEGE_APP_TYPE, appPath.c_str());

                // set process name
                const int PR_NAME_LENGTH = 16;
                std::string processName = tzAppId.substr(0, PR_NAME_LENGTH);
                prctl(PR_SET_NAME, processName.c_str());
            }

            // add web process to cgroup (appId) to trace network
            pid_t wpid = getpid();
            int ret = join_app_performance(DPL::ToUTF8String(dao.getTizenAppId()).c_str(), wpid);
            if (ret != RESOURCED_ERROR_NONE) {
                _D("fail to add web process(%d) to UI cgroup", wpid);
            } else {
                _D("add cgroup (%s), (%d)", DPL::ToUTF8String(dao.getTizenAppId()).c_str(), wpid);
            }

            /* This type of message is received when widget is restarting
             * (proably in other situation too). Widget restart can be
             * called after system language change so language tags have to
             * be recreated here.
             * Do NOT MOVE LanguageTags reset before m_widgetHandle initialization
             */
            // reset language tags (create new tags based on system locales)
            LanguageTagsProviderSingleton::Instance().resetLanguageTags();
            DPL::OptionalString defaultLocale = dao.getDefaultlocale();
            if (!!defaultLocale) {
                LanguageTagsProviderSingleton::Instance().addWidgetDefaultLocales(
                    *defaultLocale);
            }
            LanguageTags tags =
                LanguageTagsProviderSingleton::Instance().getLanguageTags();
            _D("Current widget locales (language tags):");
            FOREACH(it, tags) {
                _D("Locale: %s", DPL::ToUTF8String(*it).c_str());
            }

            _D("Preload PluginLogicSingleton");
            PluginModule::init(WrtDB::WidgetDAOReadOnly::getHandle(m_widgetTizenId));
            _D("Preload PluginLogicSingleton_end");

            m_securityModelVersion = dao.getSecurityModelVersion();
#if ENABLE(CORS_WHITELISTING)
            bypassCORSforWARPAccessList(dao);
#endif
            m_decryptionSupport->initialize(m_widgetTizenId);
            m_viewmodesSupport.reset(
                new InjectedBundle::ViewmodesSupport(m_widgetTizenId));
            m_initialized = true;
        } else {
            _D("already initalized");
        }
    } else if (WKStringIsEqualToUTF8CString(
                   messageName,
                   Message::ToInjectedBundle::SET_XWINDOW_HANDLE))
    {
            std::string msgString =
                toString(static_cast<WKStringRef>(messageBody));
#if 0 // sub mode disable
            _D("set x window handle [%s]", msgString.c_str());
            IPCMessageSupport::setXwindowHandle(atoi(msgString.c_str()));
#else
            _D("sub mode is disabled, set x window handle [NULL]");
#endif

    } else if (WKStringIsEqualToUTF8CString(
                   messageName,
                   Message::ToInjectedBundle::SET_VIEWMODES))
    {
        std::string msgBody =
            toString(static_cast<WKStringRef>(messageBody));
        _D("set viewmode to [%s]", msgBody.c_str());
        if (msgBody == Message::ToInjectedBundle::SET_VIEWMODES_MSGBODY_EXIT) {
            m_viewmodesSupport->exitViewmodesAllPages();
        } else {
            m_viewmodesSupport->enterViewmodesAllPages(msgBody);
        }
    }
    else if (WKStringIsEqualToUTF8CString(messageName, IPCMessageSupport::REPLY_ASYNC))
    {
        using namespace IPCMessageSupport;

        std::string msgBody = toString(static_cast<WKStringRef>(messageBody));

        if (msgBody.find_first_of('_') != std::string::npos) {
            std::string strHandle = msgBody.substr(0, msgBody.find_first_of('_'));
            std::string strBody = msgBody.substr(msgBody.find_first_of('_')+1);

            _D("handle: %s, , Body: %s", strHandle.c_str(), strBody.c_str());

            int handle = atoi(strHandle.c_str());

            AsyncConnectionPtr connection = AsyncConnectionManager::instance().getConnection(handle);

            if (connection) {
                if (connection->replyCallback) {
                    _D("connection->replyCallback()");
                    (connection->replyCallback)(handle, connection->data, strBody.c_str());
                }

                AsyncConnectionManager::instance().removeConnection(handle);
            } else {
                _D("Connection is not available. Ignored.");
            }
        }
    }
}

WKURLRequestRef Bundle::willSendRequestForFrameCallback(
    WKBundlePageRef /*page*/,
    WKBundleFrameRef /*frame*/,
    uint64_t /*resourceIdentifier*/,
    WKURLRequestRef request,
    WKURLResponseRef /*response*/,
    const void *clientInfo)
{
    Bundle* This = static_cast<Bundle*>(const_cast<void*>(clientInfo));
    WKURLRequestRef ret = This->willSendRequestForFrame(request);

    return ret;
}

void Bundle::didStartProvisionalLoadForFrameCallback(
    WKBundlePageRef page,
    WKBundleFrameRef frame,
    WKTypeRef* /*userData*/,
    const void *clientInfo)
{
    _D("called");
    Bundle* This = static_cast<Bundle*>(const_cast<void*>(clientInfo));

    if (This->m_pageGlobalContext.find(page) == This->m_pageGlobalContext.end()) {
        return;
    }
    if (This->m_pageContext.count(page) == 0) {
        return;
    }

    JSGlobalContextRef context = WKBundleFrameGetJavaScriptContext(frame);

    ContextSet::iterator i = This->m_pageContext[page].find(context);

    if (i == This->m_pageContext[page].end()) {
        _D("Initially attached frame");
        return;
    }

    This->m_pageContext[page].erase(i);
    This->m_willRemoveContext = context;
}

void Bundle::didRemoveFrameFromHierarchyCallback(
    WKBundlePageRef page,
    WKBundleFrameRef frame,
    WKTypeRef* /*userData*/,
    const void *clientInfo)
{
    _D("called");
    Bundle* This = static_cast<Bundle*>(const_cast<void*>(clientInfo));

    if (This->m_pageContext.count(page) == 0) {
        _D("his->m_pageContext.count(page) == 0");
        return;
    }

    JSGlobalContextRef context = WKBundleFrameGetJavaScriptContext(frame);

    ContextSet::iterator i = This->m_pageContext[page].find(context);

    if (i == This->m_pageContext[page].end()) {
        _W("Tried to unload frame which has never been loaded");
        return;
    }

    This->m_pageContext[page].erase(i);

    PluginModule::unloadFrame(context);
}

void Bundle::didFinishLoadForResourceCallback(
    WKBundlePageRef /*page*/,
    WKBundleFrameRef /*frame*/,
    uint64_t /*resourceIdentifier*/,
    const void* /*clientInfo*/)
{
    _D("called");
}

void Bundle::didCommitLoadForFrameCallback(
    WKBundlePageRef page,
    WKBundleFrameRef frame,
    WKTypeRef* /*userData*/,
    const void *clientInfo)
{
    _D("called");
    LOG_PROFILE_START("didCommitLoadForFrameCallback");
    Bundle* This = static_cast<Bundle*>(const_cast<void*>(clientInfo));

    WKURLRef url = WKBundleFrameCopyURL(frame);

    if (url == NULL) {
        _W("url is NULL");
        return;
    }

    if (This->m_willRemoveContext) {
        PluginModule::unloadFrame(This->m_willRemoveContext);
        This->m_willRemoveContext = NULL;
    }

    JSGlobalContextRef context = WKBundleFrameGetJavaScriptContext(frame);

    This->m_pageContext[page].insert(context);
    std::string urlStr = toString(url);

    if (WKBundleFrameIsMainFrame(frame)) {
        _D("frame main frame");
        if(This->m_pageGlobalContext.find(page) != This->m_pageGlobalContext.end())
        {
            _D("Previous context: %p", This->m_pageGlobalContext.getContextForPage(page));
            PluginModule::stop(This->m_pageGlobalContext.getContextForPage(page));
        }
        _D("New context: %p", context);
        //note that since we need old context for unloading plugins it must be sotred
        //custom container take care of increamenting and decrementing references
        This->m_pageGlobalContext.insertContextForPage(page, context);
    }

    if (InjectedBundleURIHandling::processURIForPlugin(urlStr.c_str())){
        _D("start plugin");
        LOG_PROFILE_START("PluginModule start");
        PluginModule::start(
            WrtDB::WidgetDAOReadOnly::getHandle(This->m_widgetTizenId),
            context,
            This->m_scale,
            This->m_encodedBundle.c_str(),
            This->m_theme.c_str() );
        LOG_PROFILE_STOP("PluginModule start");

        PluginModule::loadFrame(context);
        LOG_PROFILE_STOP("didCommitLoadForFrameCallback");
    }
}

WKBundlePagePolicyAction Bundle::decidePolicyForNavigationActionCallback(
    WKBundlePageRef page,
    WKBundleFrameRef frame,
    WKBundleNavigationActionRef navigationAction,
    WKURLRequestRef request,
    WKTypeRef* userData,
    const void* clientInfo)
{
    _D("called");
    Bundle* This = static_cast<Bundle*>(const_cast<void*>(clientInfo));
    return This->decidePolicyForAction(false,
                                       page,
                                       frame,
                                       navigationAction,
                                       request,
                                       userData);
}

WKBundlePagePolicyAction Bundle::decidePolicyForNewWindowActionCallback(
    WKBundlePageRef page,
    WKBundleFrameRef frame,
    WKBundleNavigationActionRef navigationAction,
    WKURLRequestRef request,
    WKStringRef /*frameName*/,
    WKTypeRef* userData,
    const void* clientInfo)
{
    _D("called");
    Bundle* This = static_cast<Bundle*>(const_cast<void*>(clientInfo));
    return This->decidePolicyForAction(true,
                                       page,
                                       frame,
                                       navigationAction,
                                       request,
                                       userData);
}

WKBundlePagePolicyAction Bundle::decidePolicyForResponseCallback(
    WKBundlePageRef /* page */,
    WKBundleFrameRef /* frame */,
    WKURLResponseRef response,
    WKURLRequestRef /* request */,
    WKTypeRef*          /* userData */,
    const void*         /* clientInfo */)
{
    _D("called");
    Assert(response);
    WKStringRef contentTypeRef = WKURLResponseEflCopyContentType(response);

    std::string contentType = toString(contentTypeRef);
    _D("contentTypeRef : %s", contentType.c_str());
    WKRelease(contentTypeRef);

    if (contentType == HTML_MIME) {
        _D("Accepting HTML_MIME type");
        return WKBundlePagePolicyActionUse;
    }
    if (contentType == PHP_MIME) {
        _D("Accepting php type");
        return WKBundlePagePolicyActionUse;
    }

    return WKBundlePagePolicyActionPassThrough;
}

WKURLRequestRef Bundle::willSendRequestForFrame(WKURLRequestRef request)
{
    static bool logEnable = (getenv(WRT_WILL_SEND_REQUEST_LOG_ENABLE) != NULL);

    WKURLRef    wkUrl   = WKURLRequestCopyURL(request);
    WKStringRef wkStr   = WKURLCopyString(wkUrl);

    std::string stdUrl  = Bundle::toString(wkStr);
    std::string localizedUrl;

    WKRelease(wkStr);
    WKRelease(wkUrl);

    if (logEnable){ _D("willSendRequestForFrame : %s", stdUrl.c_str()); }

    std::string scheme = stdUrl.substr(0, stdUrl.find_first_of(':'));

#if ENABLE(APP_SCHEME)
    if (scheme == SCHEME_FILE) {
        _E("File schema blocked for: %s", stdUrl.c_str());
        return NULL;
    }
#endif

    // "about:blank"/"about:srcdoc" uri doesn't need uri processing.
    if (stdUrl == BLANK_PAGE_URL || stdUrl == SRC_DOC_PAGE_URL) {
        WKRetain(request);
        return request;
    }

    localizedUrl = InjectedBundleURIHandling::localizeURI(stdUrl, DPL::ToUTF8String(m_widgetTizenId));
    bool ret = InjectedBundleURIHandling::processURI(localizedUrl, m_widgetTizenId, m_securityModelVersion);

    if (!ret) {
        _D("Not permitted resource: %s", localizedUrl.c_str());
        return NULL;
    }

    // log disabled for performance
    //LogDebug("URI processing result: " << result);
    scheme = localizedUrl.substr(0, localizedUrl.find_first_of(':'));

    // Return value must contain details information of input
    // WKURLRequestRef. Current webkit2 doesn't support api that
    // copy WKURLRequestRef or change url only. Before webkit2
    // support api, callback return original WKURLRequestRef in the
    // case of external scheme

    // external scheme also need to send message to UI process for
    // checking roaming and security

    if (scheme == SCHEME_HTTP || scheme == SCHEME_HTTPS) {
        if (logEnable){ _D("external scheme return original WKURLRequestRef"); }
        WKRetain(request);

        return request;
    } else {
        std::string checkUrl = localizedUrl;

        if (m_decryptionSupport->isNeedDecryption(checkUrl)) {
            std::string decryptString =
                m_decryptionSupport->decryptResource(checkUrl);

            if (logEnable){ _D("return value : %s", decryptString.c_str()); }

            WKURLRef destUrl =
                WKURLCreateWithUTF8CString(decryptString.c_str());
            WKURLRequestRef req = WKURLRequestCreateWithWKURL(destUrl);
            WKRelease(destUrl);

            return req;
        }
    }

    WKURLRef newUrl = WKURLCreateWithUTF8CString(localizedUrl.c_str());
    WKURLRequestRef req = WKURLRequestCreateWithWKURL(newUrl);
    WKRelease(newUrl);

    if (logEnable){ _D("return value : %s", localizedUrl.c_str()); }

    return req;
}

WKBundlePagePolicyAction Bundle::decidePolicyForAction(
    bool isNewWindow,
    WKBundlePageRef /* page */,
    WKBundleFrameRef frame,
    WKBundleNavigationActionRef /* navigationAction */,
    WKURLRequestRef request,
    WKTypeRef* /* userData */)
{
    using namespace ViewModule;
    using namespace ViewModule::SchemeActionMap;

    char const * const TIZEN_SCHEME = "tizen";

    std::string request_uri = toString(request);

    _D("request uri : %s", request_uri.c_str());

    // exception uri
    if (request_uri == BLANK_PAGE_URL) {
        return WKBundlePagePolicyActionUse;
    }

    // in case of box scheme, unconditionally PassThrough should be returned
    if (!request_uri.compare(0, 6, SCHEME_BOX_SLASH)) {
        return WKBundlePagePolicyActionPassThrough;
    }

    DPL::String dplUrl = DPL::FromUTF8String(request_uri);
    bool ret =
        InjectedBundleURIHandling::processMainResource(dplUrl,
                                                       m_widgetTizenId,
                                                       m_securityModelVersion);
    if (!ret) {
        std::string blockedUrl = DPL::ToUTF8String(dplUrl);
        _D("URI is blocked: %s", blockedUrl.c_str());

        // Send information about blocked URI to UIProcess
        WKStringRef urlStr = WKStringCreateWithUTF8CString(blockedUrl.c_str());
        WKStringRef blockMessage =
            WKStringCreateWithUTF8CString(Message::ToUIProcess::BLOCKED_URL);
        WKBundlePostMessage(m_bundle, blockMessage, urlStr);
        WKRelease(urlStr);
        WKRelease(blockMessage);
        return WKBundlePagePolicyActionPassThrough;
    }

    // get scheme string
    std::string request_scheme = getScheme(request_uri);

    // is tizen schem?
    if (request_scheme == TIZEN_SCHEME) {
        return WKBundlePagePolicyActionPassThrough;
    }

    // scheme action
    Scheme scheme(request_scheme);
    Scheme::Type type = scheme.GetType();
    if (type < Scheme::FILE || type >= Scheme::COUNT) {
        _D("Unknown scheme : %s", request_scheme.c_str());
        return WKBundlePagePolicyActionPassThrough;
    }

    bool mainFrame = WKBundleFrameIsMainFrame(frame);
    NavigationContext ctx = mainFrame ? TOP_LEVEL : FRAME_LEVEL;
    if (isNewWindow) {
        ctx = NEW_WINDOW;
    }

    UriAction action = g_tizenActionMap[type][ctx];
    _D("Scheme type: %d, Navigation context: %d, Action: %d",
       type,
       ctx,
       action);

    if (action != URI_ACTION_WRT) {
        return WKBundlePagePolicyActionPassThrough;
    }

    return WKBundlePagePolicyActionUse;
}

std::string Bundle::toString(WKStringRef str)
{
    if (WKStringIsEmpty(str)) {
        return "";
    } else {
        size_t size = WKStringGetMaximumUTF8CStringSize(str);

        char buffer[size + 1];
        WKStringGetUTF8CString(str, buffer, sizeof(buffer));

        return buffer;
    }
}

std::string Bundle::toString(WKURLRef url)
{
    WKStringRef urlStr = WKURLCopyString(url);
    std::string str = toString(urlStr);
    WKRelease(urlStr);
    return str;
}

std::string Bundle::toString(WKURLRequestRef req)
{
    WKURLRef reqUrl = WKURLRequestCopyURL(req);
    std::string str = toString(reqUrl);
    WKRelease(reqUrl);
    return str;
}

std::string Bundle::toString(WKErrorRef err)
{
    WKStringRef domErr = WKErrorCopyDomain(err);
    WKStringRef desc = WKErrorCopyLocalizedDescription(err);
    std::string str = toString(domErr) + "\n" + toString(desc);
    WKRelease(domErr);
    WKRelease(desc);
    return str;
}

std::string Bundle::getScheme(std::string uri)
{
    std::size_t found = uri.find(':');
    std::string str;

    if (found != std::string::npos) {
        str = uri.substr(0, found);
    }

    return str;
}

static void vconfChangedHandler(keynode_t* key, void* data)
{
    _D("vconfChangedHandler");

    DPL_UNUSED_PARAM(key);
    DPL_UNUSED_PARAM(data);

    LanguageTagsProviderSingleton::Instance().resetLanguageTags();
}

extern "C"
{
WK_EXPORT
void WKBundleInitialize(WKBundleRef bundle,
                        WKTypeRef)
{
    _D("Bundle initialized");

    DPL::Event::GetMainEventDispatcherInstance().ResetCrossEventCallHandler();
    _D("ResetCrossEventCallHandler()");

    static Bundle s_bundle(bundle);

    WKBundleClient client = {
        kWKBundleClientCurrentVersion,
        &s_bundle,
        &Bundle::didCreatePageCallback,
        &Bundle::willDestroyPageCallback,
        0,     /* didInitializePageGroup */
        &Bundle::didReceiveMessageCallback
    };
    WKBundleSetClient(bundle, &client);

    // process pool - restore process priority
    if (UID_ROOT == getuid()) {
        setpriority(PRIO_PROCESS, 0, DEFAULT_PRIORITY);
    }
    IPCMessageSupport::setWKBundleRef(bundle);

    // TODO: Move to Construtor(Bundle::Bundle)
    //       Add "vconf_ignore_key_changed(VCONFKEY_LANGSET, vconfChangedHandler);" in the destructor(Bundle::~Bundle)
    vconf_notify_key_changed(VCONFKEY_LANGSET, vconfChangedHandler, NULL);
}
}
