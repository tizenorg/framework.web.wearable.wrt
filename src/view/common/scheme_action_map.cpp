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
 * @file       scheme_action_map.cpp
 * @author     Krzysztof Jackiewicz (k.jackiewicz@samsung.com)
 * @version    1.0
 */

#include "scheme_action_map.h"
#include <scheme.h>
#include <dpl/log/log.h>
#include <dpl/event/controller.h>
#include <application_launcher.h>
#include <appsvc.h>
#include <pcrecpp.h>
#include <memory>
#include <curl/curl.h>
#include "scheme_action_map_data.h"

namespace ViewModule {
namespace {
/*
 * Lazy construction pattern.
 * TODO Make it more general. Use variadic template/tuples/lambdas or sth. Move
 * to DPL.
 */
template <typename T, typename Arg1>
class Lazy
{
  public:
    explicit Lazy(const Arg1& arg1) :
        m_arg1(arg1),
        m_object(new std::unique_ptr<T>())
    {}
    Lazy(const Lazy<T, Arg1>& other) :
        m_arg1(other.m_arg1),
        m_object(other.m_object)
    {}

    T& operator*()
    {
        return GetObject();
    }
    const T& operator*() const
    {
        return GetObject();
    }
    const T* operator->() const
    {
        return &GetObject();
    }
    T* operator->()
    {
        return &GetObject();
    }

    Lazy<T, Arg1>& operator=(const Lazy<T, Arg1>& other)
    {
        m_arg1 = other.m_arg1;
        m_object = other.m_object;
        return *this;
    }

  private:
    T& GetObject() const
    {
        if (!(*m_object)) {
            (*m_object).reset(new T(m_arg1));
        }
        return **m_object;
    }

    Arg1 m_arg1;
    // single unique_ptr shared among many Lazy copies
    mutable std::shared_ptr<std::unique_ptr<T> > m_object;
};

/*
 * Struct defining conversion of scheme for given appsvc key for example:
 * sms:5551212?body=expected%20text => APPSVC_DATA_TEXT + expected%20text
 */
struct AppSvcConversion {
    AppSvcConversion(char const * const keyValue,
                     const std::string& regexStr) :
        key(keyValue),
        regex(regexStr) {}
    char const * const key;
    Lazy<pcrecpp::RE, std::string> regex;
};

/*
 * Struct defining an appsvc operation and a list of scheme conversions used to
 * fill in additional appsvc data.
 */
struct ServiceOperation {
    const char* operation;
    const char* fakeUri;
    const char* mime;
    std::list<AppSvcConversion> conversions;
};

typedef std::map<Scheme::Type, ServiceOperation> ServiceOperationMap;

// Regular expressions used to extract appsvc data from scheme
// TODO what about multiple recipients?
char const * const REGEX_BODY =         ".*[?&]body=([^&]+).*";
char const * const REGEX_SMS =           "sms:([^&]+).*";
char const * const REGEX_SMSTO =        "smsto:([^&]+).*";
char const * const REGEX_MMSTO =        "mmsto:([^&]+).*";
char const * const REGEX_MAILTO =       "mailto:([^&]+).*";
char const * const REGEX_TO =           ".*[?&]to=([^&]+).*";
char const * const REGEX_CC =           ".*[?&]cc=([^&]+).*";
char const * const REGEX_BCC =          ".*[?&]bcc=([^&]+).*";
char const * const REGEX_SUBJECT =      ".*[?&]subject=([^&]+).*";
char const * const REGEX_DATA_CONTEXT = ".*;phone-context=([^:]+).*";

ServiceOperationMap initializeAppSvcOperations()
{
    ServiceOperationMap ret;

    // FILE, HTTP & HTTPS
    ServiceOperation viewOp;
    viewOp.operation = APP_CONTROL_OPERATION_VIEW;
    viewOp.fakeUri = NULL;
    viewOp.mime = NULL;
    // no additional data
    ret.insert(std::make_pair(Scheme::FILE, viewOp));
    ret.insert(std::make_pair(Scheme::HTTP, viewOp));
    ret.insert(std::make_pair(Scheme::HTTPS, viewOp));

    // SMS
    ServiceOperation smsOp;
    smsOp.operation = APP_CONTROL_OPERATION_COMPOSE;
    smsOp.fakeUri = NULL;
    smsOp.mime = "*/*";
    smsOp.conversions.push_back(AppSvcConversion(APP_CONTROL_DATA_TO, REGEX_SMS));
    smsOp.conversions.push_back(AppSvcConversion(APP_CONTROL_DATA_TEXT, REGEX_BODY));
    ret.insert(std::make_pair(Scheme::SMS, smsOp));

    // SMSTO
    ServiceOperation smstoOp;
    smstoOp.operation = APP_CONTROL_OPERATION_COMPOSE;
    smstoOp.fakeUri = "sms";
    smstoOp.mime = "*/*";
    smstoOp.conversions.push_back(AppSvcConversion(APP_CONTROL_DATA_TO, REGEX_SMSTO));
    smstoOp.conversions.push_back(AppSvcConversion(APP_CONTROL_DATA_TEXT, REGEX_BODY));
    ret.insert(std::make_pair(Scheme::SMSTO, smstoOp));


    // MMSTO & MAILTO
    ServiceOperation sendOp;
    sendOp.operation = APP_CONTROL_OPERATION_COMPOSE;
    sendOp.fakeUri = NULL;
    sendOp.mime = NULL;
    sendOp.conversions.push_back(AppSvcConversion(APP_CONTROL_DATA_TO, REGEX_MMSTO));
    sendOp.conversions.push_back(AppSvcConversion(APP_CONTROL_DATA_TO, REGEX_MAILTO));
    sendOp.conversions.push_back(AppSvcConversion(APP_CONTROL_DATA_CC, REGEX_CC));
    sendOp.conversions.push_back(
        AppSvcConversion(APP_CONTROL_DATA_BCC, REGEX_BCC));
    sendOp.conversions.push_back(
        AppSvcConversion(APP_CONTROL_DATA_SUBJECT, REGEX_SUBJECT));
    sendOp.conversions.push_back(
        AppSvcConversion(APP_CONTROL_DATA_TEXT, REGEX_BODY));
    ret.insert(std::make_pair(Scheme::MAILTO, sendOp));
    sendOp.mime = "*/*";
    ret.insert(std::make_pair(Scheme::MMSTO, sendOp));

    // TODO what about DATA?

    // TEL
    ServiceOperation telOp;
    telOp.operation = APP_CONTROL_OPERATION_CALL;
    telOp.fakeUri = NULL;
    telOp.mime = NULL;
    ret.insert(std::make_pair(Scheme::TEL, telOp));

    return ret;
}

ServiceOperationMap g_serviceOperationMap = initializeAppSvcOperations();

void handleTizenServiceScheme(const char* uri)
{
    // <a href="tizen-service:AppID=com.samsung.myfile; key=key1, value=value1; end">Tizen Service</a>
    std::string parameter = std::string(uri);
    std::string appId;
    size_t start, end = 0;

    if (parameter.find("AppID=") != std::string::npos) {
        start = parameter.find("AppID=") + strlen("AppID=");
        end = parameter.find(";", start);
        appId = parameter.substr(start, end-start);
    } else {
        LogError("parameter doesn't contain appID");
        return;
    }

    app_control_h handle = NULL;
    if (app_control_create(&handle) != APP_CONTROL_ERROR_NONE) {
        LogError("Fail to create service handle");
        return;
    }

    if (app_control_set_app_id(handle, appId.c_str()) < 0) {
        LogError("Fail to service_set_app_id");
        app_control_destroy(handle);
        return;
    }

    const char* KEY_KEY = "key=";
    const char* KEY_VALUE = "value=";

    char* buf = strdup(parameter.c_str());
    const char* ptr = strtok(buf,";");
    while (ptr != NULL) {
        std::string string = ptr;
        ptr = strtok (NULL, ";");

        size_t devide = string.find(',');
        if (devide == std::string::npos) {
            continue;
        }
        size_t keyPos = string.find(KEY_KEY);
        if (keyPos == std::string::npos) {
            continue;
        }
        size_t valuePos = string.find(KEY_VALUE);
        if (valuePos == std::string::npos) {
            continue;
        }

        std::string key =
            string.substr(keyPos + std::string(KEY_KEY).size(),
                          devide - (keyPos + std::string(KEY_KEY).size()));
        std::string value =
            string.substr(valuePos + std::string(KEY_VALUE).size());

        if (app_control_add_extra_data(handle, key.c_str(), value.c_str())) {
            LogError("service_add_extra_data is failed.");
            app_control_destroy(handle);
            free(buf);
            return;
        }
    }
    free(buf);

    CONTROLLER_POST_EVENT(
        ApplicationLauncher,
        ApplicationLauncherEvents::LaunchApplicationByAppService(
            handle,
            NULL,
            NULL));
}

void handleUnknownScheme(const char* scheme, const char* uri)
{
    LogError("Invalid scheme: " << scheme);
    // case of unknown scheme, send to app-control
    // This is temporary soultion. "invalid" scheme should be handled by
    // scheme map data

    if (!strcmp(scheme, "tizen-service")) {
        handleTizenServiceScheme(uri);
        return;
    } else {
        // create app_control
        app_control_h app_control = NULL;
        if (APP_CONTROL_ERROR_NONE == app_control_create(&app_control)) {
            app_control_set_operation(app_control, APP_CONTROL_OPERATION_VIEW);
            app_control_set_uri(app_control, uri);
            CONTROLLER_POST_EVENT(
                ApplicationLauncher,
                ApplicationLauncherEvents::LaunchApplicationByAppService(
                    app_control,
                    NULL,
                    NULL));
        }
    }
}


} // namespace

namespace SchemeActionMap {
bool HandleUri(const char* uri, NavigationContext context)
{
    if (!uri) {
        LogError("wrong arguments passed");
        return false;
    }
    LogDebug("Uri being checked: " << uri);

    const char *end = strstr(uri, ":");
    if (!end) {
        LogError("Lack of scheme - ignoring");
        return false;
    }
    std::string name(uri, end);
    Scheme scheme(name);
    LogDebug("Scheme: " << name);

    Scheme::Type type = scheme.GetType();
    if (type < Scheme::FILE || type >= Scheme::COUNT) {
        LogError("Invalid scheme: " << name);
        handleUnknownScheme(name.c_str(), uri);
        return false;
    }

    LogDebug("Scheme type: " << type);
    LogDebug("Navigation context: " << context);

    UriAction action = g_tizenActionMap[type][context];

    LogDebug("Uri action: " << action);

    // execute action if necessary
    switch (action) {
    case URI_ACTION_APPSVC:
    {
        // find AppSvcOperation for given scheme type
        auto it = g_serviceOperationMap.find(type);
        if (it == g_serviceOperationMap.end()) {
            LogError("No entry for scheme: " << name);
            return false;
        }

        // prepare appsvc bundle
        app_control_h app_control = NULL;
        app_control_create(&app_control);
        LogDebug("appsvc operation " << it->second.operation);
        app_control_set_operation(app_control, it->second.operation);
        if (it->second.fakeUri) {
            size_t size = strlen(it->second.fakeUri) + strlen(uri) + 1;
            char *newUri = new char[size];
            strcpy(newUri, it->second.fakeUri);
            const char* uriArgs = strstr(uri, ":");
            strcpy(newUri + strlen(it->second.fakeUri), uriArgs);
            app_control_set_uri(app_control, newUri);
            delete [] newUri;
        }
        else {
            app_control_set_uri(app_control, uri);
        }
        if (it->second.mime) {
            app_control_set_mime(app_control, it->second.mime);
        }

        // this is safe as there are no other threads
        CURL* curl = curl_easy_init();
        // unescape the url
        int outLength = 0;
        char* unescaped = curl_easy_unescape(curl, uri, 0, &outLength);
        std::string uUri(unescaped, outLength);
        curl_free(unescaped);
        curl_easy_cleanup(curl);
        LogDebug("unescaped " << uUri);

        // setup additional appsvc data
        FOREACH(cit, it->second.conversions) {
            LogDebug("extracting data for key " << cit->key);

            std::string match;
            pcrecpp::StringPiece input(uUri);

            // convert scheme text to appsvc format
            while (cit->regex->Consume(&input, &match)) {
                LogDebug("Adding apssvc data: " << cit->key << " " << match);
                app_control_add_extra_data(app_control, cit->key, match.c_str());
            }
        }

        // TODO do we need a callback?
        CONTROLLER_POST_EVENT(
            ApplicationLauncher,
            ApplicationLauncherEvents::LaunchApplicationByAppService(
                app_control,
                NULL,
                NULL));
        break;
    }

    case URI_ACTION_VIDEO:
        CONTROLLER_POST_EVENT(
            ApplicationLauncher,
            ApplicationLauncherEvents::LaunchApplicationByPkgname(
                ApplicationLauncherPkgname::PKG_NAME_VIDEO_PLAYER,
                name,
                uri,
                "null"));
        break;
    default:
        break;
    }
    return (action == URI_ACTION_WRT);
}
} // namespace SchemeActionMap
} /* namespace ViewModule */
