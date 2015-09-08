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
 * @file    view_logic_uri_support.cpp
 * @author  Pawel Sikorski (p.sikorski@samsung.com)
 * @brief   Implementation file of UriSupport API used by ViewLogic
 */

#include "view_logic_uri_support.h"

#include <list>
#include <memory>
#include <string>
#include <vector>
#include <stdio.h>

#include <appsvc.h>
#include <aul.h>
#include <bundle.h>
#include <dpl/localization/w3c_file_localization.h>
#include <dpl/log/log.h>
#include <dpl/platform.h>
#include <dpl/string.h>
#include <iri.h>
#include <pcrecpp.h>
#include <wrt-commons/custom-handler-dao-ro/CustomHandlerDatabase.h>
#include <wrt-commons/custom-handler-dao-ro/custom_handler_dao_read_only.h>

#include <widget_model.h>
#include <application_data.h>

namespace ViewModule {
namespace UriSupport {
namespace {
struct AppControlCompareData {
std::string operation;
std::string uri;
std::string scheme;
std::string mime;
};
enum ServiceDataType
{
    APP_CONTROL_DATA_TYPE_OPERATION,
    APP_CONTROL_DATA_TYPE_URI,
    APP_CONTROL_DATA_TYPE_URI_SCHEME,
    APP_CONTROL_DATA_TYPE_MIME
};

#if ENABLE(APP_SCHEME)
char const * const SCHEME_TYPE_APP = "app";
#endif
char const * const SCHEME_TYPE_FILE = "file";
char const * const SCHEME_TYPE_HTTP = "http";
char const * const SCHEME_TYPE_HTTPS = "https";
char const * const SCHEME_TYPE_WIDGET = "widget";
const char * const SCHEME_TYPE_FILE_SLASH = "file://";

bool wildcardCompare(const std::string& wildcardString, const std::string& target)
{
    std::string re = wildcardString;

    // replace special character to meaning character
    pcrecpp::RE("\\\\").GlobalReplace("\\\\\\\\", &re);
    pcrecpp::RE("\\.").GlobalReplace("\\\\.", &re);
    pcrecpp::RE("\\+").GlobalReplace("\\\\+", &re);
    pcrecpp::RE("\\?").GlobalReplace("\\\\?", &re);
    pcrecpp::RE("\\^").GlobalReplace("\\\\^", &re);
    pcrecpp::RE("\\$").GlobalReplace("\\\\$", &re);
    pcrecpp::RE("\\[").GlobalReplace("\\\\[", &re);
    pcrecpp::RE("\\]").GlobalReplace("\\\\]", &re);
    pcrecpp::RE("\\{").GlobalReplace("\\\\{", &re);
    pcrecpp::RE("\\}").GlobalReplace("\\\\}", &re);
    pcrecpp::RE("\\(").GlobalReplace("\\\\(", &re);
    pcrecpp::RE("\\)").GlobalReplace("\\\\)", &re);
    pcrecpp::RE("\\|").GlobalReplace("\\\\|", &re);

    // replace wildcard character to regex type
    pcrecpp::RE("\\*").GlobalReplace(".*", &re);

    return pcrecpp::RE(re).FullMatch(target);
}

bool compareServiceData(ServiceDataType type,
                        std::string origin,
                        std::string other)
{
    if (APP_CONTROL_DATA_TYPE_OPERATION == type)
    {
        return origin == other;
    }
    else if (APP_CONTROL_DATA_TYPE_URI == type)
    {
        return wildcardCompare(origin, other);
    }
    else if (APP_CONTROL_DATA_TYPE_URI_SCHEME == type)
    {
        return origin == other;
    }
    else if (APP_CONTROL_DATA_TYPE_MIME == type)
    {
        return wildcardCompare(origin, other);
    }
    else
    {
        LogError("Wrong data type");
        return false;
    }
}

std::string getAppControlSrc(
    WrtDB::WidgetAppControlList appControlLost,
    AppControlCompareData data)
{
    LogDebug(" - operation : " << data.operation);
    LogDebug(" - uri       : " << data.uri);
    LogDebug(" - scheme    : " << data.scheme);
    LogDebug(" - mimetype  : " << data.mime);

    FOREACH(appControlIt, appControlLost)
    {
        if (compareServiceData(APP_CONTROL_DATA_TYPE_OPERATION, DPL::ToUTF8String(appControlIt->operation), data.operation))
        {
            if (compareServiceData(APP_CONTROL_DATA_TYPE_URI_SCHEME, DPL::ToUTF8String(appControlIt->uri), data.scheme) ||
                compareServiceData(APP_CONTROL_DATA_TYPE_URI, DPL::ToUTF8String(appControlIt->uri), data.uri) )
            {
                if (compareServiceData(APP_CONTROL_DATA_TYPE_MIME, DPL::ToUTF8String(appControlIt->mime), data.mime))
                {
                    LogDebug("Matched with : " << appControlIt->src);
                    LogDebug(" - operation : " << appControlIt->operation);
                    LogDebug(" - uri       : " << appControlIt->uri);
                    LogDebug(" - mimetype  : " << appControlIt->mime);
                    return DPL::ToUTF8String(appControlIt->src);
                }
            }
        }
    }
    return std::string();
}

}

std::string prepareUrl(const std::string &url, const std::string &insert)
{
    std::string urlFixed = url;
    if (urlFixed.find("file://") == 0) {
        urlFixed.erase(0, 6);
    }
    //replace %s in url with given from appservice
    int size = snprintf(NULL, 0, urlFixed.c_str(), insert.c_str()) + 1;
    char buffer[size];
    snprintf(buffer, size, urlFixed.c_str(), insert.c_str());
    return std::string(buffer);
}

std::string getCustomHandlerProtocolUri(
    WidgetModel *widgetModel,
    const std::string &schemeType,
    const std::string &schemeValue)
{
    CustomHandlerDB::Interface::attachDatabaseRO();
    CustomHandlerDB::CustomHandlerDAOReadOnly handlersDao(widgetModel->TizenId);
    CustomHandlerDB::CustomHandlerPtr handler =
        handlersDao.getActivProtocolHandler(
            DPL::FromASCIIString(schemeType));
    CustomHandlerDB::Interface::detachDatabase();
    if (handler) {
        LogDebug("Found handler, url: " << handler->url);
        return prepareUrl(DPL::ToUTF8String(handler->base_url) +
                          DPL::ToUTF8String(handler->url), schemeValue);
    }
    return "";
}

std::string getCustomHandlerContentUri(
    WidgetModel *widgetModel,
    const std::string &mime,
    const std::string &mimeValue)
{
    CustomHandlerDB::Interface::attachDatabaseRO();
    CustomHandlerDB::CustomHandlerDAOReadOnly handlersDao(widgetModel->TizenId);
    CustomHandlerDB::CustomHandlerPtr handler =
        handlersDao.getActivContentHandler(
            DPL::FromASCIIString(mime));
    CustomHandlerDB::Interface::detachDatabase();
    if (handler) {
        LogDebug("Found handler, url: " << handler->url);
        return prepareUrl(DPL::ToUTF8String(handler->base_url) +
                          DPL::ToUTF8String(handler->url), mimeValue);
    }
    return "";
}

std::string getAppControlUri(bundle *bundle, WidgetModel *widgetModel)
{
    if (!bundle)
    {
        LogError("Bundle is empty");
        return std::string("");
    }

    AppControlCompareData data;
    // get operation. Operation is mandatory.
    const char* value = NULL;
    value = appsvc_get_operation(bundle);
    data.operation = value ? value : "";
    // ignore operation is NULL case
    if (data.operation.empty()) {
        LogDebug("Operation is NULL");
        return std::string("");
    }

    // get mime
    value = appsvc_get_mime(bundle);
    data.mime = value ? value : "";

    // get uri
    value = appsvc_get_uri(bundle);
    data.uri = value ? value : "";

    LogDebug("Passed AppControl data");
    LogDebug(" - operation : " << data.operation);
    LogDebug(" - uri       : " << data.uri);
    LogDebug(" - mimetype  : " << data.mime);


    // get scheme and mime
    std::string originScheme = "";
    std::string originPath = "";
    if (!data.uri.empty()) {
        std::unique_ptr<iri_t, decltype(&iri_destroy)> iri(iri_parse(data.uri.c_str()), iri_destroy);
        if (!iri.get()) {
            LogDebug("Fail to get iri");
            originScheme = "";
            originPath = "";
        }
        if (iri->scheme) {
            originScheme = iri->scheme;
        }
        if (iri->path) {
            originPath = iri->path;
        }

        // checking condition that mime is empthy and uri is available
        if (data.mime.empty() && !data.uri.empty()) {
            // checking passed uri is local file
            // case 1. uri = file:///xxxx
            // case 2. uri = /xxxx
            if (!data.scheme.empty() && data.scheme != SCHEME_TYPE_FILE) {
                LogDebug("Passed uri isn't local file");
            } else {
                const char* FILE_URI_CASE_1 = "/";
                const char* FILE_URI_CASE_2 = "file:/";
                const char* FILE_URI_CASE_3 = "file:///";

                char mimetype[128] = {0,};
                int ret = AUL_R_EINVAL;
                const char* uri_c_str = data.uri.c_str();

                if(strncmp(uri_c_str, FILE_URI_CASE_1, 1) == 0){
                    ret = aul_get_mime_from_file(uri_c_str,
                                                 mimetype,
                                                 sizeof(mimetype));
                } else if(strncmp(uri_c_str, FILE_URI_CASE_2, 6) == 0){
                    ret = aul_get_mime_from_file(&uri_c_str[5],
                                                 mimetype,
                                                 sizeof(mimetype));
                } else if(strncmp(uri_c_str, FILE_URI_CASE_3, 8) == 0){
                    ret = aul_get_mime_from_file(&uri_c_str[7],
                                                 mimetype,
                                                 sizeof(mimetype));
                }

                if (AUL_R_OK == ret) {
                    data.mime = mimetype;
                }
            }
        }
    }

    WrtDB::WidgetAppControlList appControlList =
        widgetModel->AppControlList.Get();
    if (!appControlList.empty()) {
        // case 0, operation only
        // scheme =
        // uri    =
        if (data.uri.empty()) {
            LogDebug("AppControl case 0 (NULL, NULL)");
            std::string src = getAppControlSrc(appControlList, data);
            if (!src.empty()) {
                return src;
            }
            return std::string();
        }

        // case 1
        // scheme = nfc
        // uri    = nfc:///xxx/xxx
        if (!originScheme.empty()) {
            data.scheme = originScheme;
            LogDebug("AppControl case 1 (file, file:///xxx/xxx)");
            std::string src = getAppControlSrc(appControlList, data);
            if (!src.empty()) {
                return src;
            }
        }

        // case 2
        // scheme =
        // uri    = file:///xxx/xxx
        data.scheme = "";
        LogDebug("AppControl case 2 (NULL, file:///xxx/xxx)");
        std::string src = getAppControlSrc(appControlList, data);
        if (!src.empty()) {
            return src;
        }

        // case 3
        // scheme = file
        // uri    = /xxx/xxx
        if (!originScheme.empty() && !originPath.empty()) {
            data.scheme = originScheme;
            data.uri = originPath;
            LogDebug("AppControl case 3 (file, /xxx/xxx)");
            std::string src = getAppControlSrc(appControlList, data);
            if (!src.empty()) {
                return src;
            }
        }

        // case 4
        // scheme =
        // uri    = /xxx/xxx
        if (!originPath.empty()) {
            data.scheme = "";
            data.uri = originPath;
            LogDebug("AppControl case 4 (NULL, /xxx/xxx)");
            std::string src = getAppControlSrc(appControlList, data);
            if (!src.empty()) {
                return src;
            }
        }
    }

    if (!originScheme.empty()) {
        LogDebug("Scheme parts: " << originScheme << ", " << originPath);
        return getCustomHandlerProtocolUri(
                   widgetModel, originScheme, originPath);
    }
    if (data.mime != "") {
        value = appsvc_get_data(bundle, APPSVC_DATA_SELECTED);
        if (value != NULL) {
            LogDebug("Use mime type for: " << value);
            return getCustomHandlerContentUri(
                       widgetModel, data.mime, std::string(value));
        } else {
            LogError("Selected file for mime is null, nothind to do");
        }
    }
    LogDebug("no matching result");
    return std::string("");
}

std::string getUri(WidgetModel *widgetModel, const std::string &defaultUri, bool* isSelfTarget)
{
    DPL::String uri;
    std::string startUri;
    LogDebug("default uri: " << defaultUri);
    bundle *originBundle = ApplicationDataSingleton::Instance().getBundle();
    // search app-control data
    startUri = getAppControlUri(originBundle, widgetModel).c_str();
    LogDebug("app-control start uri is " << startUri);
    if (startUri == "") {
        LogDebug("app-control data doesn't have matched data");
        startUri = defaultUri;
    } else if (startUri == SELF_TARGET) {
        startUri = defaultUri;
        if (isSelfTarget) {
            *isSelfTarget = true;
        }
    }

    // insert prefix path
    std::string preFix = DPL::ToUTF8String(widgetModel->PrefixURL.Get());
    if (strstr(startUri.c_str(), SCHEME_TYPE_HTTP) == startUri.c_str() ||
        strstr(startUri.c_str(), SCHEME_TYPE_HTTPS) == startUri.c_str() ||
        strstr(startUri.c_str(), preFix.c_str()) == startUri.c_str())
    {
        return startUri;
    } else {
        return preFix + startUri;
    }
}

DPL::OptionalString localizeURI(const DPL::String& inputURI,
                                const WidgetModel* model)
{
    auto uri = DPL::ToUTF8String(inputURI);
    LogDebug("localizing url: " << uri);

    auto urlcstr = uri.c_str();

    const char *end = strstr(urlcstr, ":");
    if (!end) {
        LogDebug("no schema in link, return null");
        // lack of schema
        return DPL::OptionalString();
    }

    std::string scheme(urlcstr, end);
#if ENABLE(APP_SCHEME)
    if (scheme != SCHEME_TYPE_WIDGET && scheme != SCHEME_TYPE_FILE && scheme != SCHEME_TYPE_APP) {
#else
    if (scheme != SCHEME_TYPE_WIDGET && scheme != SCHEME_TYPE_FILE) {
#endif
        LogDebug("scheme doesn't need to localize");
        return DPL::OptionalString(inputURI);
    }

    DPL::OptionalString found =
        W3CFileLocalization::getFilePathInWidgetPackageFromUrl(
            model->TizenId,
            DPL::FromUTF8String(uri));

    if (!found) {
        // In this case, path doesn't need to localize. return input uri
        LogDebug("Path not found within current locale in current widget");
        return DPL::OptionalString(inputURI);
    } else {
        DPL::String uri(L"file://" + *found);
        LogDebug("Will load resource: " << uri);
        LogDebug("finished");
        return DPL::OptionalString(uri);
    }
}
} // namespace UriSupportImplementation
} // namespace ViewModule
