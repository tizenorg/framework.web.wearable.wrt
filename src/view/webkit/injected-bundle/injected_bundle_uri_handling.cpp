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
 * @file    injected_bundle_uri_handling.cpp
 * @author  Marcin Kaminski (marcin.ka@samsung.com)
 * @version 1.0
 */

#include "injected_bundle_uri_handling.h"

#include <memory>
#include <string.h>
#include <sys/stat.h>

#include <dpl/log/secure_log.h>
#include <dpl/utils/wrt_global_settings.h>
#include <dpl/platform.h>
// For dao creation (widget info fetching)
#include <dpl/wrt-dao-ro/widget_dao_read_only.h>
// security checks for URI
#include <ace-common/ace_api_common.h>
#include <ace-client/ace_api_client.h>
#include <dpl/utils/warp_iri.h>
// URI localization
#include <dpl/localization/w3c_file_localization.h>
// WARP check
#include <widget_data_types.h>
#include <dpl/wrt-dao-ro/common_dao_types.h>
// allow-navigation check
#include <pcrecpp.h>
#include <iri.h>

namespace {
char const * const SCHEME_TYPE_FILE = "file";
char const * const SCHEME_TYPE_WIDGET = "widget";
char const * const SCHEME_TYPE_APP = "app";
char const * const SCHEME_TYPE_HTTP = "http";
char const * const PARAM_URL = "param:url";
char const * const ACE_IGNORED_SCHEMA[] = {
    "file://",
    "widget://",
#if ENABLE(APP_SCHEME)
    "app://",
#endif
    "data:",
    "tel:",
    "sms:",
    "mmsto:",
    "mailto:",
    0 };

bool wildcardCompare(std::string wildcardString, std::string target)
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

bool checkWARP(const char *url, const DPL::String& tizenId)
{
    // ignore WARP in test mode
    if (GlobalSettings::WarpTestModeEnabled()) {
        return true;
    }

    if (WarpIRI::isIRISchemaIgnored(url)) {
        // scheme is not supported by WARP
        return true;
    }

    WrtDB::WidgetDAOReadOnly dao = WrtDB::WidgetDAOReadOnly(tizenId);
    WrtDB::WidgetAccessInfoList widgetAccessInfoList;
    dao.getWidgetAccessInfo(widgetAccessInfoList);

    // temporary solution for libiri parsing error
    // This code will be removed
    std::string urlstr = url;
    size_t pos = urlstr.find_first_of("#?");
    if (pos != std::string::npos) {
        urlstr = urlstr.substr(0, pos);
    }

    return (static_cast<WidgetAccessList>(widgetAccessInfoList)).isRequiredIRI(
               DPL::FromUTF8String(urlstr));
}

bool checkWhitelist(const char *url)
{
    if (url == NULL) {
        return true;
    }

    std::unique_ptr<iri_t, decltype(&iri_destroy)> iri(iri_parse(url), iri_destroy);
    if (!iri->scheme || !iri->host || strlen(iri->host) == 0) {
        return true;
    }
    std::string scheme = iri->scheme;
    std::string host = iri->host;

    if (scheme.find(SCHEME_TYPE_HTTP) == std::string::npos) {
        return true;
    }

    return false;
}

bool checkAllowNavigation(const char *url, const DPL::String& tizenId)
{
    if (url == NULL) {
        return true;
    }

    std::unique_ptr<iri_t, decltype(&iri_destroy)> iri(iri_parse(url), iri_destroy);
    if (!iri->scheme || !iri->host || strlen(iri->host) == 0) {
        return true;
    }
    std::string scheme = iri->scheme;
    std::string host = iri->host;

    if (scheme.find(SCHEME_TYPE_HTTP) == std::string::npos) {
        return true;
    }

    WrtDB::WidgetDAOReadOnly dao = WrtDB::WidgetDAOReadOnly(tizenId);
    WrtDB::WidgetAllowNavigationInfoList list;
    dao.getWidgetAllowNavigationInfo(list);

    FOREACH(it, list) {
        if (wildcardCompare(DPL::ToUTF8String(it->scheme), scheme) &&
            wildcardCompare(DPL::ToUTF8String(it->host), host))
        {
            return true;
        }
    }
    _E("deny");
    return false;
}

bool preventSymlink(const std::string & url)
{
    if(0 != strncmp(url.c_str(), SCHEME_TYPE_FILE, strlen(SCHEME_TYPE_FILE)))
    {
        return true;
    }

    if(url.size() >= strlen(SCHEME_TYPE_FILE) + 3)
    {
        std::string file = url.substr(strlen(SCHEME_TYPE_FILE) + 3);
        struct stat st;
        if(0 != stat(file.c_str(), &st)) return true;
        return !S_ISLNK(st.st_mode);
    }
    else
    {
        return true;
    }
}

bool checkACE(const char* url, bool xhr, const DPL::String& tizenId)
{
    if (url) {
        for (size_t i = 0; ACE_IGNORED_SCHEMA[i]; ++i) {
            if (0 == strncmp(url,
                             ACE_IGNORED_SCHEMA[i],
                             strlen(ACE_IGNORED_SCHEMA[i])))
            {
                return true;
            }
        }
    }

    const char *devCapNamesMarkup = "externalNetworkAccess";
    const char *devCapNamesXHR = "XMLHttpRequest";

    ace_request_t aceRequest;

    aceRequest.widget_handle = WrtDB::WidgetDAOReadOnly::getHandle(tizenId);

    // TODO! We should get session id from somewhere (outside Widget Process)
    const std::string session = "";
    aceRequest.session_id = const_cast<ace_session_id_t>(session.c_str());
    aceRequest.feature_list.count = 0;
    aceRequest.dev_cap_list.count = 1;
    aceRequest.dev_cap_list.items = new ace_dev_cap_t[1];

    if (xhr) {
        aceRequest.dev_cap_list.items[0].name =
            const_cast<ace_string_t>(devCapNamesXHR);
    } else {
        aceRequest.dev_cap_list.items[0].name =
            const_cast<ace_string_t>(devCapNamesMarkup);
    }

    aceRequest.dev_cap_list.items[0].param_list.count = 1;
    aceRequest.dev_cap_list.items[0].param_list.items = new ace_param_t[1];
    aceRequest.dev_cap_list.items[0].param_list.items[0].name =
        const_cast<ace_string_t>(PARAM_URL);
    aceRequest.dev_cap_list.items[0].param_list.items[0].value =
        const_cast<ace_string_t>(url);

    ace_check_result_t result = ACE_PRIVILEGE_DENIED;
    ace_return_t ret = ace_check_access_ex(&aceRequest, &result);

    _D("Result is: %d", static_cast<int>(result));

    delete[] aceRequest.dev_cap_list.items[0].param_list.items;
    delete[] aceRequest.dev_cap_list.items;

    return ACE_OK == ret && ACE_ACCESS_GRANTED == result;
}
} // namespace (anonymous)

namespace InjectedBundleURIHandling {
bool processURI(const std::string& inputURI,
                const DPL::String& tizenId,
                WrtDB::WidgetSecurityModelVersion version)
{
    if (version == WrtDB::WidgetSecurityModelVersion::WIDGET_SECURITY_MODEL_V1)
    {
        if (!checkWARP(inputURI.c_str(), tizenId)) {
            _E("Request was blocked by WARP: %s", inputURI.c_str());
            return false;
        }
    }

    // disable for performance
    // if (!preventSymlink(inputURI)) {
    //     LogWarning("Request for symlink is invalid: " << inputURI);
    //     return false;
    //}

    return true;
}

bool processURI(const DPL::String& inputURI,
                const DPL::String& tizenId,
                WrtDB::WidgetSecurityModelVersion version)
{
    DPL::OptionalString optionalUri(inputURI);
    if (!optionalUri) {
        _D("uri is empty");
        return true;
    }

    std::string uri = DPL::ToUTF8String(inputURI);
    return processURI(uri, tizenId, version);
}

bool processMainResource(const DPL::String& inputURI,
                         const DPL::String& tizenId,
                         WrtDB::WidgetSecurityModelVersion version)
{
    DPL::OptionalString optionalUri(inputURI);
    if (!optionalUri) {
        _D("uri is empty");
        return true;
    }

    std::string uri = DPL::ToUTF8String(inputURI);
    if (version ==
        WrtDB::WidgetSecurityModelVersion::WIDGET_SECURITY_MODEL_V1)
    {
        if (!checkWARP(uri.c_str(), tizenId)) {
            _E("Request was blocked by WARP: %s", uri.c_str());
            return false;
        }
    } else if (version ==
        WrtDB::WidgetSecurityModelVersion::WIDGET_SECURITY_MODEL_V2)
    {
#if ENABLE(ALLOW_NAVIGATION)
        if (!checkAllowNavigation(uri.c_str(), tizenId)) {
            _E("Request was blocked by allow-navigation: %s", uri.c_str());
            return false;
        }
#else
        return false;
#endif
    }

    // disable for performance
    // if (!preventSymlink(uri)) {
    //     LogWarning("Request for symlink is invalid: " << uri);
    //     return false;
    // }
    return true;
}

bool processURIForPlugin(const char* url)
{
    return checkWhitelist(url);
}

std::string localizeURI(const std::string& inputURI, const std::string& tizenId)
{
    if (inputURI.compare(0, strlen(SCHEME_TYPE_WIDGET), SCHEME_TYPE_WIDGET) &&
        inputURI.compare(0, strlen(SCHEME_TYPE_FILE), SCHEME_TYPE_FILE) &&
        inputURI.compare(0, strlen(SCHEME_TYPE_APP), SCHEME_TYPE_APP))
    {
        _D("scheme doesn't need to localize");
        return inputURI;
    }

    std::string localizedURI = W3CFileLocalization::getFilePathInWidgetPackageFromUrl(tizenId, inputURI);

    if (localizedURI.empty()) {
        return inputURI;
    } else {
        return std::string("file://") + localizedURI;
    }
}

DPL::OptionalString localizeURI(const DPL::String& inputURI,
                                const DPL::String& tizenId)
{
    std::string uri = DPL::ToUTF8String(inputURI);
    const char* urlcstr = uri.c_str();
    const char *end = strstr(urlcstr, ":");
    if (!end) {
        _W("no schema in link, return null");
        return DPL::OptionalString();
    }
    std::string scheme(urlcstr, end);

#if ENABLE(APP_SCHEME)
    if (scheme != SCHEME_TYPE_WIDGET && scheme != SCHEME_TYPE_FILE && scheme != SCHEME_TYPE_APP) {
#else
    if (scheme != SCHEME_TYPE_WIDGET && scheme != SCHEME_TYPE_FILE) {
#endif
        _D("scheme doesn't need to localize");
        return DPL::OptionalString(inputURI);
    }

    DPL::OptionalString found =
        W3CFileLocalization::getFilePathInWidgetPackageFromUrl(
            tizenId,
            DPL::FromUTF8String(uri));

    if (!found) {
        // In this case, path doesn't need to localize. return input uri
        _W("Path not found within current locale in current widget");
        return DPL::OptionalString(inputURI);
    } else {
        DPL::String uri(L"file://" + *found);
        _D("Will load resource: %s", DPL::ToUTF8String(uri).c_str());
        return DPL::OptionalString(uri);
    }
}
} // namespace InjectedBundleURIHandling
