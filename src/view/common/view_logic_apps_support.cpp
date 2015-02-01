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
 * @file    view_logic_apps_support.cpp
 * @author  Pawel Sikorski (p.sikorski@samsung.com)
 * @brief   Implementation file of AppsSupport class used by ViewLogic
 */

#include "view_logic_apps_support.h"

#include <fstream>
#include <memory>

#include <sys/stat.h>
#include <sys/types.h>
#include <dpl/assert.h>
#include <dpl/log/log.h>
#include <iri.h>
#include <appsvc.h>
#include <notification.h>
#include <notification_internal.h>

#include <widget_model.h>
#include <application_launcher.h>

namespace ViewModule {
namespace {
const char* const SCHEME_TYPE_HTML5_VIDEO = "html5video";
const char* const HTTP_STREAMING_MPEG_MIMETYPE = "application/x-mpegurl";
const char* const HTTP_STREAMING_APPLE_MIMETYPE =
    "application/vnd.apple.mpegurl";
const char* const SCHEM_FILE = "file";
const char* const DOWNLOAD_PATH = "/opt/usr/media/Downloads/";
}

//Implementation class
class AppsSupportImplementation
{
  private:
    WidgetModel *m_widgetModel;
    bool m_initialized;
    int m_notiId;

    struct HTML5Video {
        const char* path;
        const char* cookie;
    };

    bool httpMultimediaRequest(std::string mimeType, std::string uri)
     {
         LogDebug("httpMultimediaRequest called");

         if ("null" == mimeType || "null" == uri) {
             LogError("uri/mimeType is null");
             return false;
         }

         app_control_h app_control = NULL;
         app_control_create(&app_control);

         // ignore case match of string of mime type
         // if needed, define appsvc response callback
         // and its user data per mimetype
         if (!strcasecmp(mimeType.c_str(), HTTP_STREAMING_APPLE_MIMETYPE) ||
             !strcasecmp(mimeType.c_str(), HTTP_STREAMING_MPEG_MIMETYPE))
         {
             app_control_set_operation(app_control, APP_CONTROL_OPERATION_VIEW);
             app_control_set_mime(app_control, mimeType.c_str());
             app_control_set_uri(app_control, uri.c_str());
         } else {
             LogDebug("Not Supported MIME type in WRT");
             app_control_destroy(app_control);
             return false;
         }

         CONTROLLER_POST_EVENT(
             ApplicationLauncher,
             ApplicationLauncherEvents::LaunchApplicationByAppService(
                 app_control,
                 NULL,
                 NULL));

         return true;
     }

    bool isFileExist(std::string path, bool isDirectory)
    {
        struct stat fileState;
        if (stat(path.c_str(), &fileState) != 0) {
            LogError("Fail to get file stat");
            return false;
        }

        if (isDirectory && S_ISDIR(fileState.st_mode)) {
            return true;
        }
        if (!isDirectory && S_ISREG(fileState.st_mode)) {
            return true;
        }
        return false;
    }

    bool copyFile(std::string file, std::string dest)
    {
        std::ifstream in (file);
        if (in.is_open() == false) {
            LogError("Fail to open input file");
            return false;
        }
        std::ofstream out(dest);
        if (out.is_open() == false) {
            LogError("Fail to open output file");
            return false;
        }
        out << in.rdbuf();
        out.close();
        in.close();

        return true;
    }

    void freeResource(notification_h n, bundle *b)
    {
        if (n) {
            notification_free(n);
        }
        if (b) {
            bundle_free(b);
        }
        return;
    }

    int createCompleteNotification(std::string fileName, std::string filePath, std::string result)
    {
        // create notification
        notification_h noti = NULL;
        noti = notification_create(NOTIFICATION_TYPE_NOTI);
        if (noti == NULL) {
            LogError("Fail to notification_create");
            return -1;
        }

        // set notification layout
        int ret;
        ret = notification_set_layout(noti, NOTIFICATION_LY_ONGOING_EVENT);
        if (ret != NOTIFICATION_ERROR_NONE) {
            LogError("Fail to notification_set_layout");
            freeResource(noti, NULL);
            return -1;
        }

        // set notification title text
        ret = notification_set_text(noti,
                                    NOTIFICATION_TEXT_TYPE_TITLE,
                                    fileName.c_str(),
                                    NULL,
                                    NOTIFICATION_VARIABLE_TYPE_NONE);
        if (ret != NOTIFICATION_ERROR_NONE) {
            LogError("Fail to notification_set_text title");
            freeResource(noti, NULL);
            return -1;
        }

        // set notification content text
        ret = notification_set_text(noti,
                                    NOTIFICATION_TEXT_TYPE_CONTENT,
                                    result.c_str(),
                                    NULL,
                                    NOTIFICATION_VARIABLE_TYPE_NONE);
        if (ret != NOTIFICATION_ERROR_NONE) {
            LogError("Fail to notification_set_text content");
            freeResource(noti, NULL);
            return -1;
        }

        // set bundle data
        bundle *b = bundle_create();
        if (!b) {
            LogError("Fail to bundle_create");
            freeResource(noti, NULL);
            return -1;
        }
        if (appsvc_set_operation(b, APPSVC_OPERATION_VIEW) != APPSVC_RET_OK) {
            LogError("Fail to appsvc_set_operation");
            freeResource(noti, b);
            return -1;
        }
        if (appsvc_set_uri(b, filePath.c_str()) != APPSVC_RET_OK) {
            LogError("Fail to appsvc_set_uri");
            freeResource(noti, b);
            return -1;
        }

        // set notification execute option
        ret = notification_set_execute_option(noti,
                                              NOTIFICATION_EXECUTE_TYPE_SINGLE_LAUNCH,
                                              "View",
                                              NULL,
                                              b);
        if (ret != NOTIFICATION_ERROR_NONE) {
            LogError("Fail to notification_set_execute_option");
            freeResource(noti, b);
            return -1;
        }

        int privId;
        ret = notification_insert(noti, &privId);
        if (ret != NOTIFICATION_ERROR_NONE) {
            LogError("Fail to notification_insert");
            freeResource(noti, b);
            return -1;
        }
        freeResource(noti, b);
        return privId;
    }

    void localUriRequest(std::string filePath)
    {
        LogDebug("file path = [" << filePath << "]");

        // verify request file path
        if (isFileExist(filePath, false) == false) {
            LogError("Request file path isn't existed");
            return;
        }

        // create destPath
        if (isFileExist(DOWNLOAD_PATH, true) == false) {
            LogError("Downloads directory isn't existed");
            return;
        }

        // create destFileFullPath
        std::string fileName = filePath.substr(filePath.rfind('/') + 1);
        unsigned int renameSuffixNb = 0;
        std::string destFileName = fileName;
        while (isFileExist(DOWNLOAD_PATH + destFileName, false)) {
            std::ostringstream suffixOstr;
            suffixOstr.str("");
            suffixOstr << fileName << "_" << renameSuffixNb++;

            destFileName = fileName;
            destFileName.insert(destFileName.find('.'), suffixOstr.str());
        }
        std::string destFileFullPath = DOWNLOAD_PATH + destFileName;

        // copy
        if (copyFile(filePath, destFileFullPath) == false) {
            LogError("Fail to copy file");
            return;
        }
        m_notiId = createCompleteNotification(destFileName,
                                              destFileFullPath,
                                              "Download Complete");
    }

  public:
    AppsSupportImplementation() :
        m_widgetModel(NULL),
        m_initialized(false)
    {}

    ~AppsSupportImplementation()
    {
        AssertMsg(!m_initialized,
               "AppsSupport has to be deinitialized prior destroying!");
    }

    void initialize(WidgetModel *widgetModel, unsigned windowHandle)
    {
        AssertMsg(!m_initialized, "Already initialized!");

        LogDebug("Initializing Apps Support");
        AssertMsg(widgetModel, "Passed widgetModel is NULL!");
        m_widgetModel = widgetModel;

        ApplicationLauncherSingleton::Instance().Touch();
        ApplicationLauncherSingleton::Instance().setWidgetTizenId(
            DPL::ToUTF8String(m_widgetModel->TizenId));
        ApplicationLauncherSingleton::Instance().setWindowHandle(windowHandle);

        LogDebug("Initialized");
        m_initialized = true;
    }

    void deinitialize()
    {
        AssertMsg(m_initialized, "Not initialized!");
        LogDebug("Deinitialized");
        m_widgetModel = NULL;
        m_initialized = false;
    }

    void downloadRequest(const char *url,
                         const char *mimeType,
                         const char *userParam)
    {
        LogDebug("Download info : " << url << "(" <<
                mimeType << ", " << userParam << ")");

        // separate local & host scheme
        std::unique_ptr<iri_t, decltype(&iri_destroy)> iri(iri_parse(url), iri_destroy);
        if (!iri.get()) {
            LogDebug("Fail to get iri");
            return;
        }
        if (!iri->scheme) {
            LogError("Fail to get scheme");
            return;
        }
        if (std::string(iri->scheme) == SCHEM_FILE) {
            LogDebug("copy to Download directory");
            if (!iri->path) {
                LogError("file path is empty");
                return;
            }
            localUriRequest(iri->path);
            return;
        }

        // ignore case match of string of mime type
        bool isAppServiceable = httpMultimediaRequest(
                mimeType ? std::string(mimeType) : "null",
                url ? std::string(url) : "null");

        if (isAppServiceable) {
            LogDebug("Application Service start");
            return;
        }

        CONTROLLER_POST_EVENT(
            ApplicationLauncher,
            ApplicationLauncherEvents::LaunchApplicationByPkgname(
                ApplicationLauncherPkgname::PKG_NAME_DOWNLOAD_PROVIDER,
                url ? std::string(url) : "null",
                mimeType && strlen(mimeType) != 0 ? mimeType : "null",
                userParam && strlen(userParam) != 0 ? userParam : "null"));
    }

    void html5VideoRequest(void* event_info)
    {
        LogDebug("html5VideoRequestCallback called");
        Assert(event_info);
        HTML5Video* video = static_cast<HTML5Video*>(event_info);

        LogDebug("video->path : " << video->path);
        LogDebug("video->cookie : " << video->cookie);
        if (NULL == video->path) {
            LogError("path is null");
            return;
        }
        CONTROLLER_POST_EVENT(
            ApplicationLauncher,
            ApplicationLauncherEvents::LaunchApplicationByPkgname(
                ApplicationLauncherPkgname::PKG_NAME_VIDEO_PLAYER,
                SCHEME_TYPE_HTML5_VIDEO,
                video->path ? video->path : "null",
                video->cookie ? video->cookie : "null"));
    }
};

AppsSupport::AppsSupport() : m_impl(new AppsSupportImplementation)
{}

AppsSupport::~AppsSupport()
{}

void AppsSupport::initialize(WidgetModel *widgetModel, unsigned windowHandle)
{
    m_impl->initialize(widgetModel, windowHandle);
}

void AppsSupport::deinitialize()
{
    m_impl->deinitialize();
}

void AppsSupport::html5VideoRequest(void* event_info)
{
    m_impl->html5VideoRequest(event_info);
}

void AppsSupport::downloadRequest(const char *url,
                                  const char *mimeType,
                                  const char *userParam)
{
    m_impl->downloadRequest(url, mimeType, userParam);
}
} //namespace
